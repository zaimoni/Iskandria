// lossy.hpp

#ifndef LOSSY_HPP
#define LOSSY_HPP 1

#include "interval_shim.hpp"

#include "Zaimoni.STL/augment.STL/cmath"
#include "Zaimoni.STL/Logging.h"

// currently redundant, but we might want to build against a 3rd-party interval type again
#ifdef ZAIMONI_USING_STACKTRACE
#include "Zaimoni.STL/Pure.CPP/stacktrace.hpp"
#endif

namespace zaimoni {
namespace math {

// identify interval-arithmetic type suitable for degrading to
// default to pass-through
template<class T> struct interval_type
{
	typedef typename std::remove_cv<T>::type type;
};

#define ZAIMONI_OVERRIDE_TYPE_STRUCT(TYPE,SRC,DEST)	\
template<>	\
struct TYPE<SRC>	\
{	\
	typedef DEST type;	\
};	\
	\
template<>	\
struct TYPE<const SRC>	\
{	\
	typedef DEST type;	\
};	\
	\
template<>	\
struct TYPE<volatile SRC>	\
{	\
	typedef DEST type;	\
};	\
	\
template<>	\
struct TYPE<const volatile SRC>	\
{	\
	typedef DEST type;	\
}

ZAIMONI_OVERRIDE_TYPE_STRUCT(interval_type,float, ISK_INTERVAL<float>);
ZAIMONI_OVERRIDE_TYPE_STRUCT(interval_type,double, ISK_INTERVAL<double>);
ZAIMONI_OVERRIDE_TYPE_STRUCT(interval_type,long double, ISK_INTERVAL<long double>);

// don't undefine after migrating to Zaimoni.STL
#undef ZAIMONI_OVERRIDE_TYPE_STRUCT

// XXX anything using this class could be micro-optimized
template<class T>
struct fp_compare
{
	static bool good_sum_lt(const T& lhs, const T& rhs)
	{
#ifdef ZAIMONI_USING_STACKTRACE
		zaimoni::ref_stack<zaimoni::stacktrace, const char*> log(zaimoni::stacktrace::get(), __PRETTY_FUNCTION__);
#endif
		int exponent[2];
		// only has to work reasonably after preprocessing by rearrange sum
		frexp(lhs, exponent+0);
		frexp(rhs, exponent+1);
		return exponent[0]<exponent[1];
	}
};

template<class T>
struct fp_compare<ISK_INTERVAL<T> >
{
	static bool good_sum_lt(const ISK_INTERVAL<T>& lhs, const ISK_INTERVAL<T>& rhs)
	{
#ifdef ZAIMONI_USING_STACKTRACE
		zaimoni::ref_stack<zaimoni::stacktrace, const char*> log(zaimoni::stacktrace::get(), __PRETTY_FUNCTION__);
#endif
		int exponent[6];
		// only has to work reasonably after preprocessing by rearrange sum
		frexp(lhs.lower(),exponent+0);
		frexp(lhs.upper(),exponent+1);
		frexp(rhs.lower(),exponent+2);
		frexp(rhs.upper(),exponent+3);
		exponent[4] = (exponent[0]<exponent[1] ? exponent[1] : exponent[0]);
		exponent[5] = (exponent[2]<exponent[3] ? exponent[3] : exponent[2]);
		return exponent[4]<exponent[5];
	}
};


// interval arithmetic wrappers
// we need proper function overloading here so use static member functions of a template class
template<class T>
struct lossy
{
	using interval = typename interval_type<T>::type;

	static interval sum(typename const_param<T>::type lhs, typename const_param<T>::type rhs)
	{
		const bool incoming_finite = isFinite(lhs) && isFinite(rhs);
		interval ret(lhs);
		ret += rhs;
		if (incoming_finite && !isFinite(ret)) throw std::overflow_error("addition");
		return ret;
	}
	static interval sum(interval lhs, typename const_param<T>::type rhs)
	{
		const bool incoming_finite = isFinite(lhs) && isFinite(rhs);
		lhs += rhs;
		if (incoming_finite && !isFinite(lhs)) throw std::overflow_error("addition");
		return lhs;
	}
	static interval sum(typename const_param<T>::type lhs, interval rhs)
	{
		const bool incoming_finite = isFinite(lhs) && isFinite(rhs);
		rhs += lhs;
		if (incoming_finite && !isFinite(rhs)) throw std::overflow_error("addition");
		return rhs;
	}
	static interval sum(interval lhs, typename const_param<interval>::type rhs)
	{
		const bool incoming_finite = isFinite(lhs) && isFinite(rhs);
		lhs += rhs;
		if (incoming_finite && !isFinite(lhs)) throw std::overflow_error("addition");
		return lhs;
	}

	static interval product(typename const_param<T>::type lhs, typename const_param<T>::type rhs)
	{
		const bool incoming_finite = isFinite(lhs) && isFinite(rhs);
		interval ret(lhs);
		ret *= rhs;
		if (incoming_finite && !isFinite(ret)) throw std::overflow_error("product");
		return ret;
	}
	static interval product(interval lhs, typename const_param<T>::type rhs)
	{
		const bool incoming_finite = isFinite(lhs) && isFinite(rhs);
		lhs *= rhs;
		if (incoming_finite && !isFinite(lhs)) throw std::overflow_error("product");
		return lhs;
	}
	static interval product(typename const_param<T>::type lhs, interval rhs)
	{
		const bool incoming_finite = isFinite(lhs) && isFinite(rhs);
		rhs *= lhs;
		if (incoming_finite && !isFinite(rhs)) throw std::overflow_error("product");
		return rhs;
	}
	static interval product(interval lhs, typename const_param<interval>::type rhs)
	{
		const bool incoming_finite = isFinite(lhs) && isFinite(rhs);
		lhs *= rhs;
		if (incoming_finite && !isFinite(lhs)) throw std::overflow_error("product");
		return lhs;
	}

	static interval quotient(interval lhs, typename const_param<T>::type rhs)
	{
		const bool incoming_finite = isFinite(lhs) && isFinite(rhs);
		if (causes_division_by_zero(rhs)) throw std::runtime_error("division by zero NaN " __FILE__ ":" DEEP_STRINGIZE(__LINE__));
		lhs /= rhs;
		if (incoming_finite && !isFinite(lhs)) throw std::overflow_error("quotient");
		return lhs;
	}

	static interval quotient(interval lhs, typename const_param<interval>::type rhs)
	{
		const bool incoming_finite = isFinite(lhs) && isFinite(rhs);
		if (causes_division_by_zero(rhs)) throw std::runtime_error("division by zero NaN " __FILE__ ":" DEEP_STRINGIZE(__LINE__));
		const bool infinite_out_ok = (0.0==rhs.lower() || 0.0==rhs.upper());
		lhs /= rhs;
		if (incoming_finite && !infinite_out_ok && !isFinite(lhs)) throw std::overflow_error("quotient");
		return lhs;
	}

	static interval self_product(interval& lhs, typename const_param<T>::type rhs)
	{
		const bool incoming_finite = isFinite(lhs) && isFinite(rhs);
		lhs *= rhs;
		if (incoming_finite && !isFinite(lhs)) throw std::overflow_error("product");
		return lhs;
	}

	static interval self_product(interval& lhs, typename const_param<interval>::type rhs)
	{
		const bool incoming_finite = isFinite(lhs) && isFinite(rhs);
		lhs *= rhs;
		if (incoming_finite && !isFinite(lhs)) throw std::overflow_error("product");
		return lhs;
	}

	static interval square(typename const_param<T>::type x)
	{
		const bool incoming_finite = isFinite(x);
		interval ret(zaimoni::math::square(x));
		if (incoming_finite && !isFinite(ret)) throw std::overflow_error("square");
		return ret;
	}

	static interval square(interval x)
	{
		const bool incoming_finite = isFinite(x);
		x = zaimoni::math::square(x);;
		if (incoming_finite && !isFinite(x)) throw std::overflow_error("square");
		return x;
	}
};

template<class T>
ISK_INTERVAL<T> quotient(const ISK_INTERVAL<T>& lhs, const ISK_INTERVAL<T>& rhs)
{
	return lossy<typename std::remove_cv<T>::type>::quotient(lhs,rhs);
}

}	// namespace math
}	// namespace zaimoni

#endif
