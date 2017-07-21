// overprecise.hpp

#ifndef OVERPRECISE_HPP
#define OVERPRECISE_HPP 1

#include "Zaimoni.STL/Logging.h"
#include <cmath>
#include <limits>
#include "Zaimoni.STL/Augment.STL/type_traits"
#include <stdexcept>
#include <utility>
#include <boost/numeric/interval.hpp>

// interval division of floating point can legitimately create intervals with an infinite endpoint.
// Nothing legitimately creates NaN; just assume it's pre-screened.

namespace zaimoni {

using std::swap;

using std::fpclassify;
using std::isfinite;
using std::isinf;
using std::isnan;
using std::signbit;

template<class T>
constexpr bool isnan(const boost::numeric::interval<T>& x)
{
	return empty(x)	// null set isn't that useful
        || isnan(x.lower()) || isnan(x.upper()) 	// intuitive
		|| (isinf(x.lower()) && isinf(x.upper()) && x.lower()<x.upper());	// also disallow (-infinity,infinity) (total loss of information)
}

template<class T>
constexpr bool isfinite(const boost::numeric::interval<T>& x)
{
	return std::isfinite(x.lower()) && std::isfinite(x.upper());
}

// several choices of how to define.
template<class T>
constexpr bool signbit(const boost::numeric::interval<T>& x)
{
	return std::signbit(x.upper());
}

namespace math {

template<class T>
typename std::enable_if< std::is_floating_point<T>::value, bool >::type
set_signbit(T& x, bool is_negative)
{
	x = copysign(x,is_negative ? -1.0 : 1.0);
	return true;
}

template<class T>
typename std::enable_if< std::is_integral<T>::value && std::is_signed<T>::value, bool >::type
set_signbit(T& x, bool is_negative)
{
	if (0==x) return true;
	if ((0>x) == is_negative) return true;
	if (-std::numeric_limits<T>::max()>x) return false;	// no-op for one's complement and signed-bit integer representations
	x = -x;
	return true;
}

template<class T>
constexpr typename std::enable_if< std::is_integral<T>::value && std::is_unsigned<T>::value, bool >::type
set_signbit(const T x, bool is_negative)
{
	if (0==x) return true;
	if (!is_negative) return true;
	return false;
}

template<class T>
typename std::enable_if< std::is_floating_point<T>::value, bool >::type
self_negate(T& x)
{
	set_signbit(x,!signbit(x));
	return true;
}

template<class T>
typename std::enable_if< std::is_integral<T>::value && std::is_signed<T>::value, bool >::type
self_negate(T& x)
{
	if (-std::numeric_limits<T>::max()>x) return false;	// no-op for one's complement and signed-bit integer representations
	x = -x;
	return true;
}

template<class T>
constexpr typename std::enable_if< std::is_integral<T>::value && std::is_unsigned<T>::value, bool >::type
self_negate(const T x)
{
	return 0==x;
}

template<class T>
typename std::enable_if<std::is_floating_point<T>::value , bool>::type
self_negate(boost::numeric::interval<T>& x)
{
	x = -x;
	return true;
}

#if 2
template<class T> struct fp_stats;

template<>
class fp_stats<double>
{
private:
	int _exponent;
	double _mantissa;
public:
	fp_stats() = delete;
	fp_stats(double src) {assert(0.0!=src); assert(isfinite(src)); _mantissa = frexp(src,&_exponent);}
	fp_stats(const fp_stats& src) = delete;
	fp_stats(fp_stats&& src) = delete;
	~fp_stats() = default;
	void operator=(const fp_stats& src) = delete;
	void operator=(fp_stats&& src) = delete;
	void operator=(double src) {assert(0.0!=src); assert(isfinite(src)); _mantissa = frexp(src,&_exponent);} 

	// frexp convention: mantissa is [0.5,1.0) and exponent of 1.0 is 1
	int exponent() const {return _exponent;};
	double mantissa() const {return _mantissa;};

	double delta(int n) const { return copysign(scalbn(0.5,n),_mantissa); };	// usually prepared for subtractive cancellation

	std::pair<int,int> safe_subtract_exponents()
	{
		std::pair<int,int> ret(_exponent-std::numeric_limits<double>::digits,_exponent);
		if (0.5==_mantissa || -0.5==_mantissa) ret.first--;
		if (std::numeric_limits<double>::min_exponent > ret.second) ret.second = std::numeric_limits<double>::min_exponent;
		if (std::numeric_limits<double>::min_exponent > ret.first) ret.first = std::numeric_limits<double>::min_exponent;
		return ret;
	}

	std::pair<int,int> safe_add_exponents()	// not for denormals
	{
		std::pair<int,int> ret(_exponent-std::numeric_limits<double>::digits,_exponent);
		const double abs_mantissa = (signbit(_mantissa) ? -_mantissa : _mantissa);
		double mantissa_delta = 0.5;
		while(1.0-mantissa_delta < abs_mantissa)
			{
			assert(ret.first<ret.second);
			ret.second--;
			mantissa_delta = scalbn(mantissa_delta,-1);
			}
		return ret;
	}
};
#endif

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

ZAIMONI_OVERRIDE_TYPE_STRUCT(interval_type,float,boost::numeric::interval<float>);
ZAIMONI_OVERRIDE_TYPE_STRUCT(interval_type,double,boost::numeric::interval<double>);
ZAIMONI_OVERRIDE_TYPE_STRUCT(interval_type,long double,boost::numeric::interval<long double>);

// don't undefine after migrating to Zaimoni.STL
#undef ZAIMONI_OVERRIDE_TYPE_STRUCT

template<class T>
struct static_cache
{
	template<intmax_t n> 
	static typename std::enable_if<
			std::is_convertible<intmax_t, T>::value && !std::is_scalar<T>::value,
		typename return_copy<T>::type>::type as()
	{
		static const T ret(n);
		return ret;
	}
};

template<intmax_t n, class T>
typename std::enable_if<
			std::is_convertible<intmax_t, T>::value && !std::is_scalar<T>::value,
		typename return_copy<typename std::remove_reference<T>::type>::type>::type int_as()
{
	return static_cache<typename std::remove_cv<typename std::remove_reference<T>::type>::type>::template as<n>();
}

template<intmax_t n, class T>
constexpr typename std::enable_if<std::is_arithmetic<typename std::remove_reference<T>::type>::value , typename return_copy<typename std::remove_reference<T>::type>::type >::type int_as()
{
	return n;
}
#define ZAIMONI_INT_AS_DEFINED(T) (std::is_arithmetic<typename std::remove_reference<T>::type>::value || (std::is_convertible<intmax_t, T>::value && !std::is_scalar<T>::value))

// data representation conventions
// general series sum/product: std::vector
// power: std::pair x^n
// integer range: boost::numeric::interval or std::pair

template<class T>
struct power_term : public std::pair<T,intmax_t>
{
	typedef std::pair<T,intmax_t> super;

	power_term() = default;
	// note that using std::conditional to try to optimize alignment of the type, is very contorted for this constructor
	power_term(typename const_param<T>::type x, intmax_t n) : super(x,n) { _standard_form();};
	power_term(const power_term& src) = default;
	power_term(power_term&& src) = default;
	~power_term() = default;
	power_term& operator=(const power_term& src) = default;
	power_term& operator=(power_term&& src) = default;

	T& base() {return this->first;};
	typename return_copy<T>::type base() const {return this->first;};
	intmax_t& power() {return this->second;};
	intmax_t power() const {return this->second;};
private:
	void _standard_form() {
		if (0 == this->second) {
			// interval would use "contains" here
			if (this->first == 0) return;	// 0^0 is degenerate ... interpretation depends on context
			// n^0 is a zero-ary product: 1.
			this->first = T(1);
			this->second = 1;
			return;
		}
		if (1 == this->second) return;	// normal-form
		if (T(1) == this->first) {
			// 1^n is 1.
			this->second = 1;
			return;
		}
		if (T(-1) == this->first) {
			// XXX \todo any element that is period 2 would work here.  Issue is a problem with matrices.
			if (0 == this->second%2) this->first = T(1);
			this->second = 1;
			return;
		}
		// XXX reduce small periods n?
	}
};

// algebra on fundamental types
// has not been fully hardened against non-binary floating point

template<class T>
typename std::enable_if<std::is_floating_point<T>::value , bool>::type delta_cancel(T& lhs, T& rhs, T delta)
{
	lhs += delta;
	rhs -= delta;
	return 0.0==rhs;
}

// trivial_sum family returns -1 for lhs annihilated, 1 for rhs annihilated
template<class T, class U>
typename std::enable_if<ZAIMONI_INT_AS_DEFINED(T) && ZAIMONI_INT_AS_DEFINED(U), int>::type trivial_sum(T& lhs, U& rhs)
{
	assert(!isnan(lhs));
	assert(!isnan(rhs));
	if (int_as<0,U>() == rhs) return 1;
	if (int_as<0,T>() == lhs) return -1;
	if (isinf(lhs))
		{
		if (!isinf(rhs)) return 1;
		if (signbit(lhs)!=signbit(rhs)) throw std::runtime_error("infinity-infinity NaN");
		return 1;
		}
	if (isinf(rhs)) return -1;
	return 0;
}

// the rearrange_sum family returns true iff rhs has been annihilated with exact arithmetic
template<class T>
typename std::enable_if<std::is_floating_point<T>::value , bool>::type rearrange_sum(T& lhs, T& rhs)
{
	assert(!trivial_sum(lhs,rhs));

	// 0: lhs
	// 1: rhs
hard_restart:
	const int fp_type[2] = { fpclassify(lhs) , fpclassify(rhs)};
	const bool is_negative[2] = {signbit(lhs) , signbit(rhs)};

	// epsilon exponent is simply -std::numeric_limits<T>::digits+1 (+1 from bias)
	// remember: 1.0 maps to exponent 1, mantissa 0.5
restart:
	fp_stats<T> lhs_stats(lhs);
	fp_stats<T> rhs_stats(rhs);
	const int exponent_delta = rhs_stats.exponent()-lhs_stats.exponent();

	if (is_negative[0]==is_negative[1]) {
		// same sign
		if (lhs==rhs && std::numeric_limits<T>::max_exponent>lhs_stats.exponent()) {
			lhs = scalbn(lhs,1);
			rhs = 0.0;
			return true;
		}
		// a denormal acts like it has exponent std::numeric_limits<T>::min_exponent - 1
		if (FP_SUBNORMAL == fp_type[0] && FP_SUBNORMAL == fp_type[1]) {
			T tmp = copysign(std::numeric_limits<T>::min(),lhs);
			// lhs+rhs = (lhs+tmp)+(rhs-tmp)
			rhs -= tmp;	// now opp-sign denormal
			rhs += lhs;
			lhs = tmp;
			if (0.0 == rhs) return true;
			goto hard_restart;	// could be more clever here if breaking const
		}
		if (0==exponent_delta && std::numeric_limits<T>::max_exponent>lhs_stats.exponent()) {	// same idea as above
			T tmp = copysign(scalbn(1.0,lhs_stats.exponent()+1),(is_negative[0] ? -1.0 : 1.0));
			rhs -= tmp;
			rhs += lhs;
			lhs = tmp;
			goto restart;
		}
		if (0<exponent_delta) {	// rhs larger
			const std::pair<int,int> lhs_safe(lhs_stats.safe_subtract_exponents());
			const std::pair<int,int> rhs_safe(rhs_stats.safe_add_exponents());
			if (rhs_safe.first>lhs_safe.second) return false;

			if (delta_cancel(rhs,lhs,lhs_stats.delta(lhs_safe.second)))
				{
				swap(lhs,rhs);
				return true;
				}
			if (std::numeric_limits<T>::min_exponent+std::numeric_limits<T>::digits >= lhs_stats.exponent()) goto hard_restart;	// may have just denormalized
			goto restart;
		} else {	// lhs larger
			const std::pair<int,int> lhs_safe(lhs_stats.safe_add_exponents());
			const std::pair<int,int> rhs_safe(rhs_stats.safe_subtract_exponents());
			if (lhs_safe.first>rhs_safe.second) return false;

			if (delta_cancel(lhs,rhs,rhs_stats.delta(rhs_safe.second))) return true;
			if (std::numeric_limits<T>::min_exponent+std::numeric_limits<T>::digits >= rhs_stats.exponent()) goto hard_restart;	// may have just denormalized
			goto restart;
		}
	} else {
		// opposite sign: cancellation
		if (0==exponent_delta) {
			lhs += rhs;
			rhs = 0.0;
			return true;
		}
		if (    (FP_SUBNORMAL == fp_type[0] || lhs_stats.exponent()==std::numeric_limits<T>::min_exponent)
		     && (FP_SUBNORMAL == fp_type[1] || lhs_stats.exponent()==std::numeric_limits<T>::min_exponent)) {
			lhs += rhs;
			rhs = 0.0;
			return true;
		}
		const std::pair<int,int> lhs_safe(lhs_stats.safe_subtract_exponents());
		const std::pair<int,int> rhs_safe(rhs_stats.safe_subtract_exponents());
		if (0<exponent_delta) {
			// rhs larger
			if (rhs_safe.first>lhs_safe.second) return false;
			if (delta_cancel(rhs,lhs,lhs_stats.delta(lhs_safe.second)))
				{
				swap(lhs,rhs);
				return true;
				}
			if (std::numeric_limits<T>::min_exponent+std::numeric_limits<T>::digits >= lhs_stats.exponent()) goto hard_restart;	// may have just denormalized
			goto restart;
		} else {
			// lhs larger
			if (lhs_safe.first>rhs_safe.second) return false;
			if (delta_cancel(lhs,rhs,rhs_stats.delta(rhs_safe.second))) return true;
			if (std::numeric_limits<T>::min_exponent+std::numeric_limits<T>::digits >= rhs_stats.exponent()) goto hard_restart;	// may have just denormalized
			goto restart;
		}	
	}	
}

// not guaranteed effective for long double
// it is possible to optimize this with reinterpret_cast goo
// probably should fix the constants to cope with non-binary floating point
template<class T>
typename std::enable_if<std::is_floating_point<T>::value , unsigned long long>::type _mantissa_as_int(T mantissa)
{
	unsigned long long ret = 0;
	while(0.0<mantissa)
		{
		ret <<= 1;
		if (0.5<=mantissa) ret += 1;
		mantissa = scalbn(mantissa,1);
		mantissa -= 1.0;
		}
	return ret;
}

// for integer types, this just discards factors of two.  Definitions are to play nice with floating-point arithmetic
template<class T>
typename std::enable_if<std::is_integral<T>::value &&  std::is_unsigned<T>::value, unsigned long long>::type _mantissa_as_int(T mantissa)
{
	unsigned long long ret = mantissa;
	if (0==ret) return;
	while(0 == (ret & 1)) ret >>= 1; 
	return ret;
}

template<class T>
typename std::enable_if<std::is_integral<T>::value &&  std::is_signed<T>::value, unsigned long long>::type _mantissa_as_int(T mantissa)
{
	unsigned long long ret = (0<=mantissa ? mantissa : (-std::numeric_limits<T>::max()<=mantissa ? -mantissa : (unsigned long long)(std::numeric_limits<T>::max())+1ULL));
	if (0==ret) return;
	while(0 == (ret & 1)) ret >>= 1; 
	return ret;
}

// exponent values are from frexp
// we assume the lhs has fewer significant digits, so would be more useful to have in the [0.5,1.0) range
template<class T>
typename std::enable_if<std::is_floating_point<T>::value , void>::type _rebalance_exponents(T& lhs, T& rhs, int lhs_exponent, int rhs_exponent)
{
	if (1==lhs_exponent) return;
	const int delta_lhs_exp = lhs_exponent-1;
	const int abs_delta_lhs_exp = (0<delta_lhs_exp ? delta_lhs_exp : -delta_lhs_exp);
	const int clearance = (0<delta_lhs_exp ? std::numeric_limits<T>::max_exponent-rhs_exponent : rhs_exponent-std::numeric_limits<T>::min_exponent);
	if (0==clearance) return;
	const int abs_delta = (abs_delta_lhs_exp < clearance ? abs_delta_lhs_exp : clearance);
	const int delta_exp = (0<delta_lhs_exp ? abs_delta : -abs_delta);
	lhs = scalbn(lhs,delta_exp);
	rhs = scalbn(rhs,delta_exp);
}

// 1 success, 0 no-op, -2 failed to evaluate
template<class T, class U>
typename std::enable_if<ZAIMONI_INT_AS_DEFINED(U) , int>::type identity_product(T& lhs, const U& identity)
{
	if (int_as<1,U>() == identity) return 1;
	if (int_as<-1,U>() != identity) return 0;
	return self_negate(lhs) ? 1 : -2;
}

// trivial_product family returns -1 for lhs annihilated, 1 for rhs annihilated; -2 on error
template<class T, class U>
typename std::enable_if<std::is_arithmetic<T>::value && std::is_arithmetic<U>::value, int>::type trivial_product(T& lhs, U& rhs)
{
	assert(!isnan(lhs));
	assert(!isnan(rhs));
	if (int ret = identity_product(lhs,rhs)) return (-2==ret ? -2 : 1==ret);
	if (int ret = identity_product(rhs,lhs)) return (-2==ret ? -2 : -(1==ret));

	const int inf_code = (bool)(isinf(rhs))-(bool)(isinf(lhs));
	const int zero_code = 2*(int_as<0,U>()==rhs)+(int_as<0,T>()==lhs);
	switch(4*inf_code+zero_code)
	{
	case 4+1:
	case -4+2:	throw std::runtime_error("0*infinity NaN");;
	case -4+0:
	case 0+1:
	case 0+3:
		return set_signbit(lhs,signbit(lhs)!=signbit(rhs));
	case 4+0:	
	case 0+2:
		return -set_signbit(rhs,signbit(lhs)!=signbit(rhs));
//	case 0+0:	break;
#ifndef NDEBUG
	case 4+2:
	case 4+3:
	case -4+1:
	case -4+3:	throw std::runtime_error("compiler/library/hardware bug: numeral simultaneously infinite and zero");
#endif	
	}

	return 0;
}

template<class T>
typename std::enable_if<std::is_floating_point<T>::value , int>::type trivial_product(boost::numeric::interval<T>& lhs, T& rhs)
{
	assert(!isnan(lhs));
	assert(!isnan(rhs));
	if (lhs.lower()==lhs.upper())
		{	// allow for negative zero fp weirdness: upper() rather than lower()
		T tmp_lhs = lhs.upper();
		const int ret = trivial_product(tmp_lhs,rhs);
		if (ret) lhs = tmp_lhs;
		return ret;
		}

	if (int ret = identity_product(lhs,rhs)) return 1==ret;

	// intervals are only bounded by infinity
	if (isinf(rhs)) {
		if (0.0==lhs.lower() || 0.0==lhs.upper()) throw std::runtime_error("0*infinity NaN");
		if (signbit(lhs.lower())!=signbit(lhs.upper())) throw std::runtime_error("interval (-infinity,infinity) NaN");
		return -set_signbit(rhs,signbit(lhs.lower())!=signbit(rhs));
	}

	if (0.0 == rhs) return -set_signbit(rhs,signbit(lhs.upper())==signbit(rhs));

	return 0;
}

template<class T>
typename std::enable_if<std::is_floating_point<T>::value , int>::type trivial_product(boost::numeric::interval<T>& lhs, boost::numeric::interval<T>& rhs)
{
	assert(!isnan(lhs));
	assert(!isnan(rhs));
	if (rhs.lower()==rhs.upper())
		{
		T tmp_rhs = rhs.upper();
		const int ret = trivial_product(lhs,tmp_rhs);
		if (ret) rhs = tmp_rhs;
		return ret;
		}
	if (lhs.lower()==lhs.upper())
		{
		T tmp_lhs = lhs.upper();
		const int ret = trivial_product(rhs,tmp_lhs);
		if (ret) lhs = tmp_lhs;
		return -ret;
		}
#if 0
// the rejection of (-infinity,infinity) allows this source code reduction
#if 0
			if (0.0<=rhs.lower()) return 1;
			if (0.0>=rhs.upper())
				{
				lhs.assign(-lhs.upper(),copysign(0.0,-1.0));
				return 1;
				}
#else
			if (0.0>=rhs.upper()) lhs.assign(copysign(std::numeric_limits<T>::infinity(),-1.0),copysign(0.0,-1.0));
			return 1;
#endif
#endif
#define ZAIMONI_POSITIVE_INFINITY(lhs,rhs,ret)	\
	if (isinf(lhs.upper()))	\
		{	/* lhs positive infinity upper bound */	\
		if (0.0>rhs.lower() && 0.0<rhs.upper()) throw std::runtime_error("interval (-infinity,infinity) NaN");	\
		if (0.0 == lhs.lower())	\
			{	/*	lhs [0,infinity) */	\
			if (0.0>=rhs.upper()) lhs.assign(-lhs.upper(),copysign(0.0,-1.0));	\
			return ret;	\
			}	\
		}

ZAIMONI_POSITIVE_INFINITY(lhs,rhs,1)
ZAIMONI_POSITIVE_INFINITY(rhs,lhs,-1)

#undef ZAIMONI_POSITIVE_INFINITY

#define ZAIMONI_NEGATIVE_INFINITY(lhs,rhs,ret)	\
	if (isinf(lhs.lower()))	\
		{	/* lhs negative infinity lower bound */	\
		if (0.0>rhs.lower() && 0.0<rhs.upper()) throw std::runtime_error("interval (-infinity,infinity) NaN");	\
		if (0.0 == lhs.upper())	\
			{	/*	lhs (-infinity,0] */	\
			if (0.0>=rhs.upper()) lhs.assign(0.0,std::numeric_limits<T>::infinity());	\
			return ret;	\
			}	\
		}

ZAIMONI_NEGATIVE_INFINITY(lhs,rhs,1)
ZAIMONI_NEGATIVE_INFINITY(rhs,lhs,-1)

#undef ZAIMONI_NEGATIVE_INFINITY

	return 0;	// no-op to allow compiling
}

// the rearrange_product family returns true if the rhs has been annihilated (usually value 1)
template<class T>
typename std::enable_if<std::is_floating_point<T>::value , bool>::type rearrange_product(T& lhs, T& rhs)
{
	assert(!trivial_product(lhs,rhs));

	// 0: lhs
	// 1: rhs
	const int fp_type[2] = { fpclassify(lhs) , fpclassify(rhs)};
	const bool is_negative[2] = {signbit(lhs) , signbit(rhs)};

	int exponent[2];
	const T mantissa[2] = {frexp(lhs,exponent) , frexp(rhs,exponent+1)};

	// 1.0*1.0 is 1.0
	int predicted_exponent = exponent[0]+exponent[1]-1;
	if (0.5==mantissa[0] || -0.5==mantissa[0])
		{	// should be exact
		if (   std::numeric_limits<T>::max_exponent < predicted_exponent
			|| std::numeric_limits<T>::min_exponent > predicted_exponent)
			{
			_rebalance_exponents(lhs,rhs,exponent[0],exponent[1]);
			return false;
			}
		lhs *= rhs;
		rhs = 1.0;
		return true;
		}
	if (0.5==mantissa[1] || -0.5==mantissa[1])
		{	// should be exact.
		if (   std::numeric_limits<T>::max_exponent < predicted_exponent	// overflows
			|| std::numeric_limits<T>::min_exponent > predicted_exponent)	// underflows
			{
			_rebalance_exponents(rhs,lhs,exponent[1],exponent[0]);
			return false;
			}
		lhs *= rhs;
		rhs = 1.0;
		return true;
		}

	boost::numeric::interval<double> predicted_mantissa(mantissa[0]);
	predicted_mantissa *= mantissa[1];
	if (0.5<=predicted_mantissa.lower() || -0.5>=predicted_mantissa.upper()) predicted_exponent++;
	if (   std::numeric_limits<T>::max_exponent > predicted_exponent
		&& std::numeric_limits<T>::min_exponent < predicted_exponent
		&& predicted_mantissa.lower()==predicted_mantissa.upper())
		{	// exact.
		lhs *= rhs;
		rhs = 1.0;
		return true;
		}
	// XXX want to see the mantissas as integers to decide which one to optimize
	const unsigned long long mantissa_as_int[2] = {_mantissa_as_int(copysign(mantissa[0],1.0)), _mantissa_as_int(copysign(mantissa[1],1.0))};
	if (mantissa_as_int[0]<mantissa_as_int[1]) {
		_rebalance_exponents(lhs,rhs,exponent[0],exponent[1]);
	} else {
		_rebalance_exponents(rhs,lhs,exponent[1],exponent[0]);
	}
	return false;
}

template<class T>
typename std::enable_if<std::is_floating_point<T>::value , bool>::type rearrange_product(boost::numeric::interval<T>& lhs, boost::numeric::interval<T>& rhs)
{
	return false;	// no-op to allow compiling
}

// interval arithmetic wrappers
// we need proper function overloading here so use static member functions of a template class
template<class T>
struct lossy
{
	static typename interval_type<T>::type sum(typename const_param<T>::type lhs, typename const_param<T>::type rhs) 
	{
		const bool incoming_finite = isfinite(lhs) && isfinite(rhs);
		typename interval_type<T>::type ret(lhs);
		ret += rhs;
		if (incoming_finite && !isfinite(ret)) throw std::overflow_error("addition");
		return ret;
	}
	static typename interval_type<T>::type sum(typename interval_type<T>::type lhs, typename const_param<T>::type rhs) 
	{
		const bool incoming_finite = isfinite(lhs) && isfinite(rhs);
		lhs += rhs;
		if (incoming_finite && !isfinite(lhs)) throw std::overflow_error("addition");
		return lhs;
	}
	static typename interval_type<T>::type sum(typename const_param<T>::type lhs, typename interval_type<T>::type rhs) 
	{
		const bool incoming_finite = isfinite(lhs) && isfinite(rhs);
		rhs += lhs;
		if (incoming_finite && !isfinite(rhs)) throw std::overflow_error("addition");
		return rhs;
	}
	static typename interval_type<T>::type sum(typename interval_type<T>::type lhs, typename const_param<typename interval_type<T>::type>::type rhs) 
	{
		const bool incoming_finite = isfinite(lhs) && isfinite(rhs);
		lhs += rhs;
		if (incoming_finite && !isfinite(lhs)) throw std::overflow_error("addition");
		return lhs;
	}

	static typename interval_type<T>::type product(typename const_param<T>::type lhs, typename const_param<T>::type rhs) 
	{
		const bool incoming_finite = isfinite(lhs) && isfinite(rhs);
		typename interval_type<T>::type ret(lhs);
		ret *= rhs;
		if (incoming_finite && !isfinite(ret)) throw std::overflow_error("product");
		return ret;
	}
	static typename interval_type<T>::type product(typename interval_type<T>::type lhs, typename const_param<T>::type rhs) 
	{
		const bool incoming_finite = isfinite(lhs) && isfinite(rhs);
		lhs *= rhs;
		if (incoming_finite && !isfinite(lhs)) throw std::overflow_error("product");
		return lhs;
	}
	static typename interval_type<T>::type product(typename const_param<T>::type lhs, typename interval_type<T>::type rhs) 
	{
		const bool incoming_finite = isfinite(lhs) && isfinite(rhs);
		rhs *= lhs;
		if (incoming_finite && !isfinite(rhs)) throw std::overflow_error("product");
		return rhs;
	}
	static typename interval_type<T>::type product(typename interval_type<T>::type lhs, typename const_param<typename interval_type<T>::type>::type rhs) 
	{
		const bool incoming_finite = isfinite(lhs) && isfinite(rhs);
		lhs *= rhs;
		if (incoming_finite && !isfinite(lhs)) throw std::overflow_error("product");
		return lhs;
	}
};

}	// namespace math
}	// namespace zaimoni

#endif
