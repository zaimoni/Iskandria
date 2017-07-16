// overprecise.hpp

#ifndef OVERPRECISE_HPP
#define OVERPRECISE_HPP 1

#include <math.h>
#include <limits>
#include <type_traits>
#include <stdexcept>
#include <utility>
#include <boost/numeric/interval.hpp>

namespace zaimoni {

using std::swap;
using std::fpclassify;
using std::signbit;

// replicate efficient readonly call parameter options from boost
template<class T> struct const_param
{
	typedef typename std::add_const<
		typename std::conditional<sizeof(unsigned long long)>=sizeof(T) , T , typename std::add_lvalue_reference<T>::type >::type
	>::type type;
};

namespace math {

// identify interval-arithmetic type suitable for degrading to
// default to pass-through
template<class T> struct interval_type
{
	typedef typename std::remove_cv<T>::type type;
};

template<>
struct interval_type<float>
{
	typedef boost::numeric::interval<float> type;
};

template<>
struct interval_type<double>
{
	typedef boost::numeric::interval<double> type;
};

template<>
struct interval_type<long double>
{
	typedef boost::numeric::interval<long double> type;
};

template<>
struct interval_type<boost::numeric::interval<float> >
{
	typedef boost::numeric::interval<float> type;
};

template<>
struct interval_type<boost::numeric::interval<double> >
{
	typedef boost::numeric::interval<double> type;
};

template<>
struct interval_type<boost::numeric::interval<long double> >
{
	typedef boost::numeric::interval<long double> type;
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

template<class T>
typename std::enable_if<std::is_floating_point<T>::value , bool>::type find_safe_delta(T& delta, T& mantissa_delta, T abs_mantissa, int scale, int exponent)
{
	while(1.0-mantissa_delta < abs_mantissa)
		{
		if (std::numeric_limits<T>::digits <= -scale) return false;
		--scale;
		delta = scalbn(delta,-1);
		mantissa_delta = scalbn(mantissa_delta,-1);
		}
	if (1.0-mantissa_delta == abs_mantissa && std::numeric_limits<T>::max_exponent == exponent) return false;
	return true;
}


// the rearrange_sum family returns true iff rhs has been annihilated with exact arithmetic
template<class T>
typename std::enable_if<std::is_floating_point<T>::value , bool>::type rearrange_sum(T& lhs, T& rhs)
{
	if (0.0 == rhs) return true;
	if (0.0 == lhs) {
		swap(lhs,rhs);
		return true;
	}

	// 0: lhs
	// 1: rhs
hard_restart:
	const int fp_type[2] = { fpclassify(lhs) , fpclassify(rhs)};

	if (FP_NAN == fp_type[0]) return true;
	if (FP_NAN == fp_type[1]) {
		swap(lhs,rhs);
		return true;
	}

	const bool is_negative[2] = {signbit(lhs) , signbit(rhs)};

	if (FP_INFINITE == fp_type[0])
		{
		if (FP_INFINITE != fp_type[1]) return true;
		// inf - inf is NaN.  We want to abort the calculation entirely.
		if (is_negative[0]!=is_negative[1]) throw std::runtime_error("infinity-infinity NaN");
		return true;
		}
	if (FP_INFINITE == fp_type[1])
		{
		swap(lhs,rhs);
		return true;
		}

	// epsilon exponent is simply -std::numeric_limits<T>::digits+1 (+1 from bias)

	// remember: 1.0 maps to exponent 1, mantissa 0.5
restart:
	int exponent[2];
	const T mantissa[2] = {frexp(lhs,exponent) , frexp(rhs,exponent+1)};
	const int exponent_delta = exponent[1]-exponent[0];

	if (is_negative[0]==is_negative[1]) {
		// same sign
		if (lhs==rhs && std::numeric_limits<T>::max_exponent>exponent[0]) {
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
		if (0==exponent_delta && std::numeric_limits<T>::max_exponent>exponent[0]) {	// same idea as above
			T tmp = copysign(scalbn(1.0,exponent[0]+1),(is_negative[0] ? -1.0 : 1.0));
			rhs -= tmp;
			rhs += lhs;
			lhs = tmp;
			goto restart;
		}
		if (0<exponent_delta) {	// rhs larger
			if (std::numeric_limits<T>::digits < exponent_delta) return false;
			T delta = copysign(scalbn(0.5,exponent[0]),rhs);
			T mantissa_delta = scalbn(0.5,-exponent_delta);
			const T abs_mantissa = (is_negative[1] ? -mantissa[1] : mantissa[1]);
			if (!find_safe_delta(delta, mantissa_delta, abs_mantissa, exponent_delta, exponent[1])) return false;
			if (delta_cancel(rhs,lhs,delta))
				{
				swap(lhs,rhs);
				return true;
				}
			if (std::numeric_limits<T>::min_exponent+std::numeric_limits<T>::digits >= exponent[1]) goto hard_restart;	// may have just denormalized
			goto restart;
		} else {	// lhs larger
			if (std::numeric_limits<T>::digits < -exponent_delta) return false;
			T delta = copysign(scalbn(0.5,exponent[1]),rhs);
			T mantissa_delta = scalbn(0.5,exponent_delta);
			const T abs_mantissa = (is_negative[0] ? -mantissa[0] : mantissa[0]);
			if (!find_safe_delta(delta, mantissa_delta, abs_mantissa, exponent_delta, exponent[0])) return false;
			if (delta_cancel(lhs,rhs,delta)) return true;
			if (std::numeric_limits<T>::min_exponent+std::numeric_limits<T>::digits >= exponent[0]) goto hard_restart;	// may have just denormalized
			goto restart;
		}
	} else {
		// opposite sign: cancellation
		if (exponent[0]==exponent[1]) {
			lhs += rhs;
			rhs = 0.0;
			return true;
		}
		if (    (FP_SUBNORMAL == fp_type[0] || exponent[0]==std::numeric_limits<T>::min_exponent)
		     && (FP_SUBNORMAL == fp_type[1] || exponent[1]==std::numeric_limits<T>::min_exponent)) {
			lhs += rhs;
			rhs = 0.0;
			return true;
		}
		if (0<exponent_delta) {
			// rhs larger
			if (std::numeric_limits<T>::digits+1<exponent_delta) return false;
			if (std::numeric_limits<T>::digits+1==exponent_delta)
				{
				if (0.5!=mantissa[1] && -0.5!=mantissa[1]) return false;
				}
			if (delta_cancel(rhs,lhs,copysign(scalbn(0.5,exponent[0]),lhs)))
				{
				swap(lhs,rhs);
				return true;
				}
			if (std::numeric_limits<T>::min_exponent+std::numeric_limits<T>::digits >= exponent[0]) goto hard_restart;	// may have just denormalized
			goto restart;
		} else {
			// lhs larger
			if (std::numeric_limits<T>::digits+1< -exponent_delta) return false;
			if (std::numeric_limits<T>::digits+1== -exponent_delta)
				{
				if (0.5!=mantissa[0] && -0.5!=mantissa[0]) return false;
				}
			if (delta_cancel(lhs,rhs,copysign(scalbn(0.5,exponent[1]),rhs))) return true;
			if (std::numeric_limits<T>::min_exponent+std::numeric_limits<T>::digits >= exponent[1]) goto hard_restart;	// may have just denormalized
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

// the rearrange_product family returns true if the rhs has been annihilated (usually value 1)
template<class T>
typename std::enable_if<std::is_floating_point<T>::value , bool>::type rearrange_product(T& lhs, T& rhs)
{
	if (1.0 == rhs) return true;
	if (-1.0 == rhs) {
		lhs = -lhs;
		rhs = 1.0;
		return true;
	}
	if (1.0 == lhs) {
		swap(lhs,rhs);
		return true;
	}
	if (-1.0 == lhs) {
		lhs = -rhs;
		rhs = 1.0;
		return true;
	}

	// 0: lhs
	// 1: rhs
hard_restart:
	const int fp_type[2] = { fpclassify(lhs) , fpclassify(rhs)};
	if (FP_NAN == fp_type[0]) return true;
	if (FP_NAN == fp_type[1]) {
		swap(lhs,rhs);
		return true;
	}

	// reject: 0*infinity (goes to NaN)
	if (0.0 == lhs && FP_INFINITE == fp_type[1]) throw std::runtime_error("0*infinity NaN");;
	if (0.0 == rhs && FP_INFINITE == fp_type[0]) throw std::runtime_error("0*infinity NaN");;

	const bool is_negative[2] = {signbit(lhs) , signbit(rhs)};

	// ok: infinity*infinity
	// ok: infinity*finite
	if (FP_INFINITE == fp_type[0]) {
		lhs = copysign(lhs,(is_negative[0]==is_negative[1] ? 1.0 : -1.0));
		rhs = 1.0;
		return true;
	}
	if (FP_INFINITE == fp_type[1]) {
		lhs = copysign(rhs,(is_negative[0]==is_negative[1] ? 1.0 : -1.0));
		rhs = 1.0;
		return true;
	}

	// ok: 0*finite
	if (0.0 == lhs) {
		lhs = copysign(lhs,(is_negative[0]==is_negative[1] ? 1.0 : -1.0));
		rhs = 1.0;
		return true;
	}
	if (0.0 == rhs) {
		lhs = copysign(rhs,(is_negative[0]==is_negative[1] ? 1.0 : -1.0));
		rhs = 1.0;
		return true;
	}

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
	if (   std::numeric_limits<T>::max_exponent >= predicted_exponent
		&& std::numeric_limits<T>::min_exponent <= predicted_exponent
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

// interval arithmetic wrappers
// we need proper function overloading here so use static member functions of a template class
template<class T>
struct lossy
{
	static typename interval_type<T>::type sum(typename const_param<T>::type lhs, typename const_param<T>::type rhs) 
	{
		typename interval_type<T>::type ret(lhs);
		ret += rhs;
		return ret;
	}
	static typename interval_type<T>::type sum(typename interval_type<T>::type lhs, typename const_param<T>::type rhs) 
	{
		lhs += rhs;
		return lhs;
	}
	static typename interval_type<T>::type sum(typename const_param<T>::type lhs, typename interval_type<T>::type rhs) 
	{
		rhs += lhs;
		return rhs;
	}
	static typename interval_type<T>::type sum(typename interval_type<T>::type lhs, typename const_param<typename interval_type<T>::type>::type rhs) 
	{
		lhs += rhs;
		return lhs;
	}

	static typename interval_type<T>::type product(typename const_param<T>::type lhs, typename const_param<T>::type rhs) 
	{
		typename interval_type<T>::type ret(lhs);
		ret *= rhs;
		return ret;
	}
	static typename interval_type<T>::type product(typename interval_type<T>::type lhs, typename const_param<T>::type rhs) 
	{
		lhs *= rhs;
		return lhs;
	}
	static typename interval_type<T>::type product(typename const_param<T>::type lhs, typename interval_type<T>::type rhs) 
	{
		rhs *= lhs;
		return rhs;
	}
	static typename interval_type<T>::type product(typename interval_type<T>::type lhs, typename const_param<typename interval_type<T>::type>::type rhs) 
	{
		lhs *= rhs;
		return lhs;
	}
};

}	// namespace math
}	// namespace zaimoni

#endif
