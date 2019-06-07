// lossy.hpp

#ifndef LOSSY_HPP
#define LOSSY_HPP 1

#include "interval_shim.hpp"

#include "Zaimoni.STL/augment.STL/cmath"
#include "Zaimoni.STL/Logging.h"

namespace zaimoni {
namespace math {

// testing indicates Boost's numeric interval Does The Wrong Thing here (crashes sin by div-by-zero)
template<>
template<class T>
struct static_cache<ISK_INTERVAL<T> >
{
	template<intmax_t n> 
	static const ISK_INTERVAL<T>& as()
	{
		static const ISK_INTERVAL<T> ret((T)n);
		return ret;
	}

	template<uintmax_t n> 
	static typename std::enable_if<
			std::numeric_limits<intmax_t>::max()<n,
		const ISK_INTERVAL<T>& >::type as2()
	{
		static const ISK_INTERVAL<T> ret((T)n);
		return ret;
	}

	static ISK_INTERVAL<T> as3(intmax_t n) { return ISK_INTERVAL<T>(T(n)); }
	static ISK_INTERVAL<T> as4(uintmax_t n) { return ISK_INTERVAL<T>(T(n)); }
};


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

// extend numerical error support to boost::numeric::interval
template<>
template<class T>
struct numerical<ISK_INTERVAL<T> >
{
	enum {
		error_tracking = 1
	};
	typedef typename std::remove_cv<T>::type exact_type;
	typedef typename std::remove_cv<T>::type exact_arithmetic_type;

	static long double error(const ISK_INTERVAL<T>& src) {
		ISK_INTERVAL<long double> err(src.upper());
		err -= src.lower();
		return err.upper();
	}
	static bool causes_division_by_zero(const ISK_INTERVAL<T>& src)
	{
		if (int_as<0,T>() > src.lower() && int_as<0,T>() < src.upper()) return true;
		if (int_as<0,T>() == src.lower() && int_as<0,T>() == src.upper()) return true;
		return false;	
	}
	static constexpr bool equals(const ISK_INTERVAL<T>& lhs, exact_type rhs) {return lhs.lower()==rhs && lhs.upper()==rhs;};	// \todo obsolete, == operator generally ok here
};

// XXX anything using this class could be micro-optimized
template<class T>
struct fp_compare
{
	static bool good_sum_lt(const T& lhs, const T& rhs)
	{
		int exponent[2];
		// only has to work reasonably after preprocessing by rearrange sum
		frexp(lhs, exponent+0);
		frexp(rhs, exponent+1);
		return exponent[0]<exponent[1];
	}
};

template<>
template<class T>
struct fp_compare<ISK_INTERVAL<T> >
{
	static bool good_sum_lt(const ISK_INTERVAL<T>& lhs, const ISK_INTERVAL<T>& rhs)
	{
		int exponent[5];
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
	static typename interval_type<T>::type sum(typename const_param<T>::type lhs, typename const_param<T>::type rhs) 
	{
		const bool incoming_finite = isFinite(lhs) && isFinite(rhs);
		typename interval_type<T>::type ret(lhs);
		ret += rhs;
		if (incoming_finite && !isFinite(ret)) throw std::overflow_error("addition");
		return ret;
	}
	static typename interval_type<T>::type sum(typename interval_type<T>::type lhs, typename const_param<T>::type rhs) 
	{
		const bool incoming_finite = isFinite(lhs) && isFinite(rhs);
		lhs += rhs;
		if (incoming_finite && !isFinite(lhs)) throw std::overflow_error("addition");
		return lhs;
	}
	static typename interval_type<T>::type sum(typename const_param<T>::type lhs, typename interval_type<T>::type rhs) 
	{
		const bool incoming_finite = isFinite(lhs) && isFinite(rhs);
		rhs += lhs;
		if (incoming_finite && !isFinite(rhs)) throw std::overflow_error("addition");
		return rhs;
	}
	static typename interval_type<T>::type sum(typename interval_type<T>::type lhs, typename const_param<typename interval_type<T>::type>::type rhs) 
	{
		const bool incoming_finite = isFinite(lhs) && isFinite(rhs);
		lhs += rhs;
		if (incoming_finite && !isFinite(lhs)) throw std::overflow_error("addition");
		return lhs;
	}

	static typename interval_type<T>::type product(typename const_param<T>::type lhs, typename const_param<T>::type rhs) 
	{
		const bool incoming_finite = isFinite(lhs) && isFinite(rhs);
		typename interval_type<T>::type ret(lhs);
		ret *= rhs;
		if (incoming_finite && !isFinite(ret)) throw std::overflow_error("product");
		return ret;
	}
	static typename interval_type<T>::type product(typename interval_type<T>::type lhs, typename const_param<T>::type rhs) 
	{
		const bool incoming_finite = isFinite(lhs) && isFinite(rhs);
		lhs *= rhs;
		if (incoming_finite && !isFinite(lhs)) throw std::overflow_error("product");
		return lhs;
	}
	static typename interval_type<T>::type product(typename const_param<T>::type lhs, typename interval_type<T>::type rhs) 
	{
		const bool incoming_finite = isFinite(lhs) && isFinite(rhs);
		rhs *= lhs;
		if (incoming_finite && !isFinite(rhs)) throw std::overflow_error("product");
		return rhs;
	}
	static typename interval_type<T>::type product(typename interval_type<T>::type lhs, typename const_param<typename interval_type<T>::type>::type rhs) 
	{
		const bool incoming_finite = isFinite(lhs) && isFinite(rhs);
		lhs *= rhs;
		if (incoming_finite && !isFinite(lhs)) throw std::overflow_error("product");
		return lhs;
	}

	static typename interval_type<T>::type quotient(typename interval_type<T>::type lhs, typename const_param<T>::type rhs) 
	{
		const bool incoming_finite = isFinite(lhs) && isFinite(rhs);
		if (causes_division_by_zero(rhs)) throw std::runtime_error("division by zero NaN " __FILE__ ":" DEEP_STRINGIZE(__LINE__));
		lhs /= rhs;
		if (incoming_finite && !isFinite(lhs)) throw std::overflow_error("quotient");
		return lhs;
	}

	static typename interval_type<T>::type quotient(typename interval_type<T>::type lhs, typename const_param<typename interval_type<T>::type>::type rhs) 
	{
		const bool incoming_finite = isFinite(lhs) && isFinite(rhs);
		if (causes_division_by_zero(rhs)) throw std::runtime_error("division by zero NaN " __FILE__ ":" DEEP_STRINGIZE(__LINE__));
		const bool infinite_out_ok = (0.0==rhs.lower() || 0.0==rhs.upper());
		lhs /= rhs;
		if (incoming_finite && !infinite_out_ok && !isFinite(lhs)) throw std::overflow_error("quotient");
		return lhs;
	}

	static typename interval_type<T>::type self_product(typename interval_type<T>::type& lhs, typename const_param<T>::type rhs) 
	{
		const bool incoming_finite = isFinite(lhs) && isFinite(rhs);
		lhs *= rhs;
		if (incoming_finite && !isFinite(lhs)) throw std::overflow_error("product");
		return lhs;
	}

	static typename interval_type<T>::type self_product(typename interval_type<T>::type& lhs, typename const_param<typename interval_type<T>::type>::type rhs) 
	{
		const bool incoming_finite = isFinite(lhs) && isFinite(rhs);
		lhs *= rhs;
		if (incoming_finite && !isFinite(lhs)) throw std::overflow_error("product");
		return lhs;
	}

	static typename interval_type<T>::type square(typename const_param<T>::type x) 
	{
		const bool incoming_finite = isFinite(x);
		typename interval_type<T>::type ret(square(x));
		if (incoming_finite && !isFinite(ret)) throw std::overflow_error("square");
		return ret;
	}

	static typename interval_type<T>::type square(typename interval_type<T>::type x) 
	{
		const bool incoming_finite = isFinite(x);
		x = square(x);;
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
