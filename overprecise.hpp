// overprecise.hpp

#ifndef OVERPRECISE_HPP
#define OVERPRECISE_HPP 1

#include <stdexcept>
#include <utility>

#include "dicounter.hpp"
#include "lossy.hpp"
#include "int_range.hpp"
#include "Zaimoni.STL/augment.STL/cmath"

// interval division of floating point can legitimately create intervals with an infinite endpoint.
// Nothing legitimately creates NaN; just assume it's pre-screened.

namespace zaimoni {

using std::swap;

namespace math {

using std::swap;

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
	set_signbit(x,!std::signbit(x));
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
self_negate(ISK_INTERVAL<T>& x)
{
	x = -x;
	return true;
}

// will need greatest common divisor for divisibility tests
inline constexpr uintmax_t gcd(uintmax_t lhs, uintmax_t rhs)
{
    return  0==rhs ? lhs
         : (0==lhs ? rhs 
         : (1==rhs ? 1 
         : (1==lhs ? 1 
         : (lhs==rhs ? lhs
         : (lhs<rhs ? gcd(lhs,rhs%lhs) 
				    : gcd(rhs,lhs%rhs))))));
}

template<>
class fp_stats<uintmax_t>
{
private:
	int _exponent;
	int _x;
public:
	fp_stats() = delete;
	explicit fp_stats(uintmax_t src) {assert(0!=src); _exponent = INT_LOG2(src)+1; _x = src;}
	fp_stats(const fp_stats& src) = delete;
	fp_stats(fp_stats&& src) = default;
	~fp_stats() = default;
	void operator=(const fp_stats& src) = delete;
	void operator=(fp_stats&& src) = delete;
	void operator=(uintmax_t src) {assert(0!=src); _exponent = INT_LOG2(src)+1; _x = src;} 

	// while we don't want to copy, we do want to swap
	void swap(fp_stats& rhs) { std::swap(_exponent,rhs._exponent); std::swap(_x,rhs._x); }

	// frexp convention: mantissa is [0.5,1.0) and exponent of 1.0 is 1
	int exponent() const {return _exponent;};
	uintmax_t int_mantissa() const {return _mantissa_as_int(_x);}
	uintmax_t divisibility_test() const {return _mantissa_as_int(_x);}

	constexpr uintmax_t delta(int n) const { return 1ULL<<(n-1); };	// usually prepared for subtractive cancellation

	// these are in terms of absolute value
	std::pair<int,int> safe_subtract_exponents() { return std::pair<int,int>(1,_exponent); }

	std::pair<int,int> safe_add_exponents()	// not at 
	{
		assert((uintmax_t)(-1)>_x);
		uintmax_t air = (uintmax_t)(-1)-_x;
		return std::pair<int,int>(1,INT_LOG2(air)+1);
	}
};

template<class T>
class fp_stats<ISK_INTERVAL<T> >
{
	typedef ISK_INTERVAL<T> value_type;
private:
	fp_stats<double> _lb;
	fp_stats<double> _ub;
public:
	fp_stats() = delete;
	explicit fp_stats(const value_type& src) : _lb(src.lower()),_ub(src.upper()) {}
	fp_stats(const fp_stats& src) = delete;
	fp_stats(fp_stats&& src) = default;
	~fp_stats() = default;
	void operator=(const fp_stats& src) = delete;
	void operator=(fp_stats&& src) = delete;
	void operator=(const value_type& src) {_lb = src.lower(); _ub = src.upper();} 

	// while we don't want to copy, we do want to swap
	void swap(fp_stats& rhs) { _lb.swap(rhs._lb); _ub.swap(rhs._ub); }

	// frexp convention: mantissa is [0.5,1.0) and exponent of 1.0 is 1
	uintmax_t divisibility_test() {return gcd(_lb.divisibilty_test(),_ub.divisibilty_test());}
	int exponent() const {return _lb.exponent()<=_ub.exponent() ? _ub.exponent() : _lb.exponent();};
	int safe_2_n_multiply() const {return _lb.safe_2_n_multiply()<=_ub.safe_2_n_multiply() ? _lb.safe_2_n_multiply() : _ub.safe_2_n_multiply();};
	int safe_2_n_divide() const {return _lb.safe_2_n_divide()<=_ub.safe_2_n_divide() ? _lb.safe_2_n_divide() : _ub.safe_2_n_divide();};

	void update(value_type& x, dicounter& power_of_2)
	{
		*this = x;
		if (1==exponent()) return;
		if (1< exponent())
			{
			const int delta = exponent()-1;
			const uintmax_t test = power_of_2.add_capacity();
			if (test>=delta)
				{
				x = scalBn(x,-delta);
				power_of_2.safe_add(delta);
				*this = x;
				return;
				}
			else if (0<test)
				{
				x = scalBn(x,-((int)test));
				power_of_2.safe_add(test);
				*this = x;
				return;
				}
			}
		else{
			const int delta = 1-exponent();
			const uintmax_t test = power_of_2.sub_capacity();
			if (test>=delta)
				{
				x = scalBn(x,delta);
				power_of_2.safe_sub(delta);
				*this = x;
				return;
				}
			else if (0<test)
				{
				x = scalBn(x,(int)test);
				power_of_2.safe_sub(test);
				*this = x;
				return;
				}
			}
	}

	void inv_update(value_type& x, dicounter& power_of_2)
	{
		*this = x;
		if (1==exponent()) return;
		if (1< exponent())
			{
			const int delta = exponent()-1;
			const uintmax_t test = power_of_2.add_capacity();
			if (test>=delta)
				{
				x = scalBn(x,-delta);
				power_of_2.safe_sub(delta);
				*this = x;
				return;
				}
			else if (0<test)
				{
				x = scalBn(x,-((int)test));
				power_of_2.safe_sub(test);
				*this = x;
				return;
				}
			}
		else{
			const int delta = 1-exponent();
			const uintmax_t test = power_of_2.sub_capacity();
			if (test>=delta)
				{
				x = scalBn(x,delta);
				power_of_2.safe_add(delta);
				*this = x;
				return;
				}
			else if (0<test)
				{
				x = scalBn(x,(int)test);
				power_of_2.safe_add(test);
				*this = x;
				return;
				}
			}
	}

	int missed_good_exponent_by(const int useful_exponent, value_type& x, dicounter& power_of_2)
	{
		if (useful_exponent<=exponent()) return 0;
		uintmax_t delta = useful_exponent-exponent();
		uintmax_t test = power_of_2.sub_capacity();
		if (test>=delta)
			{
			x = scalBn(x,(int)delta);
			power_of_2.sub(delta);
			delta = 0;
			*this = x;
			return 0;
			}
		else if (0<test)
			{
			x = scalBn(x,(int)test);
			power_of_2.sub(test);
			delta -= test;
			*this = x;
			}
		return (int)delta;
	}

	void prepare_return_value(value_type& x, dicounter& power_of_2)
	{
		if (power_of_2.negative())
			{
			const int tmp = safe_2_n_divide();
			if (tmp>power_of_2.negative())
				{
				x = scalBn(x,-((int)power_of_2.negative()));
				power_of_2.safe_add(power_of_2.negative());
				*this = x;
				}
			else if (0<tmp)
				{
				x = scalBn(x,-tmp);
				power_of_2.safe_add(tmp);
				*this = x;
				}
			}
		else if (power_of_2.positive())
			{
			const int tmp = safe_2_n_multiply();
			if (tmp>power_of_2.positive())
				{
				x = scalBn(x,power_of_2.positive());
				power_of_2.safe_sub(power_of_2.positive());
				*this = x;
				}
			else if (0<tmp)
				{
				x = scalBn(x,tmp);
				power_of_2.safe_sub(tmp);
				*this = x;
				}
			}
	}

#if 0
	double mantissa() const {return _mantissa;};
	uintmax_t int_mantissa() const {return _mantissa_as_int(_mantissa);}

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
#endif
};

template<class T>
fp_stats<typename std::remove_cv<T>::type> get_fp_stats(T& x)
{
	return fp_stats<typename std::remove_cv<T>::type>(x);
}


template<class T>
struct is_series_product 
{
	enum {value = 0};
};

// data representation conventions
// general series sum/product: std::vector
// power: std::pair x^n
// integer range: boost::numeric::interval or std::pair

template<class T,class U>
struct power_term : public std::pair<T,U>
{
	static_assert(std::is_same_v<U, intmax_t> || std::is_same_v<U, uintmax_t>);
	using super = std::pair<T, U>;
	using canonical_type = power_term<typename interval_type<T>::type, uintmax_t>;

	power_term() = default;
	// note that using std::conditional to try to optimize alignment of the type, is very contorted for this constructor
	power_term(const_param_t<T> x, U n) : super(x,n) { _standard_form();};
	ZAIMONI_DEFAULT_COPY_DESTROY_ASSIGN(power_term);

	T& base() {return this->first;};
	typename return_copy<T>::type base() const {return this->first;};
	U& power() {return this->second;};
	U power() const {return this->second;};

	// algebra support
	typename interval_type<T>::type apply_factor(typename interval_type<T>::type& x)
	{
		lossy<typename numerical<T>::exact_type>::self_product(x,base());
		power()--;
		return x;		
	}

	// eval support
	T bootstrap_eval() {
		assert(!isNaN(*this));
		if (1==power()%2)
			{
			power()--;
			return base();
			}
		return int_as<1,T>();
	}
	// not so safe
	bool iter_eval(T& ret)
	{
		assert(!isNaN(*this));
		if (0>=power()) return false;
		if (1==power()%2) {
			apply_factor(ret);
			return true;
		}
		base() = lossy<typename numerical<T>::exact_type>::square(base());
		power() /= 2;
		return true;
	}

	canonical_type standardize() const {
		assert(!isNaN(*this));
		if (0<=power()) return canonical_type(base(),power());

		typename interval_type<T>::type new_base(int_as<1,typename interval_type<T>::type>());
		new_base /= base();
		return canonical_type(new_base,norm(-power()));
	}
private:
	void _standard_form() {
		if (0 == this->second) {
			// interval could use "contains" here, but we have decent limit behavior with integer exponents
			if (definitely_equal(int_as<0,T>(),this->first)) return;	// 0^0 is degenerate ... interpretation depends on context
			// n^0 is a zero-ary product: 1.
			this->first = int_as<1,T>();
			this->second = 1;
			return;
		}
		if (1 == this->second) return;	// normal-form
		if (definitely_equal(int_as<0,T>(),this->first)) {
			// 0^n is 0.
			this->second = 1;
			return;
		}
		if (definitely_equal(int_as<1,T>(),this->first)) {
			// 1^n is 1.
			this->second = 1;
			return;
		}
		if (definitely_equal(int_as<-1,T>(),this->first)) {
			// XXX \todo any element that is period 2 would work here.  Issue is a problem with matrices.
			if (0 == this->second%2) this->first = int_as<1,T>();
			this->second = 1;
			return;
		}
		// XXX reduce small periods n?
	}
};

template<class T,class U>
struct is_series_product<power_term<T,U> >
{
	enum {value = 1};
};

template<class T,class U>
constexpr bool isNaN(const power_term<T,U>& x)
{
	return isNaN(x.base())
        || (0==x.power() && definitely_equal(int_as<0,T>(),x.base()));
}

template<class T>
typename std::enable_if<std::is_same<power_term<T,uintmax_t>, typename power_term<T,uintmax_t>::canonical_type>::value,typename interval_type<T>::type>::type self_eval(power_term<T,uintmax_t>& x)
{
	assert(!isNaN(x));
	if (1==x.power()) return x.base();
	auto ret(x.bootstrap_eval());
	while(x.iter_eval(ret));
	return ret;
}

template<class T,class U>
typename std::enable_if<std::is_same<power_term<T,U>, typename power_term<T,U>::canonical_type>::value,typename interval_type<T>::type>::type eval(power_term<T,U> x)
{
	return self_eval(x);
}

template<class T,class U>
typename std::enable_if<!std::is_same<power_term<T,U>, typename power_term<T,U>::canonical_type>::value,typename interval_type<T>::type>::type eval(power_term<T,U> x)
{
	return eval(x.standardize());
}

// algebra on fundamental types
// has not been fully hardened against non-binary floating point

// trivial_sum family returns -1 for lhs annihilated, 1 for rhs annihilated
template<class T, class U>
typename std::enable_if<std::is_arithmetic<T>::value && std::is_arithmetic<U>::value, int>::type trivial_sum(const T& lhs, const U& rhs)
{
	assert(!isNaN(lhs));
	assert(!isNaN(rhs));
	if (int_as<0,U>() == rhs) return 1;
	if (int_as<0,T>() == lhs) return -1;
	if (isINF(lhs))
		{
		if (isINF(rhs) && signBit(lhs)!=signBit(rhs)) throw std::runtime_error("infinity-infinity NaN");
		return 1;
		}
	if (isINF(rhs)) return -1;
	return 0;
}

template<class T>
int trivial_sum(ISK_INTERVAL<T>& lhs, ISK_INTERVAL<T>& rhs)
{
	assert(!isNaN(lhs));
	assert(!isNaN(rhs));
	const int inf_code = 8*isINF(rhs.upper())+4*isINF(rhs.lower())+2* isINF(lhs.upper())+ isINF(rhs.lower());
	switch(inf_code)
	{
	case 15:		// lhs infinite, rhs infinite
		if (signBit(lhs.lower())!=signBit(rhs.lower())) throw std::runtime_error("infinity-infinity NaN");
		return 1;

	case 14:		// rhs infinite, lhs.upper() positive infinity: open ray annihilated
	case 13:		// rhs infinite, lhs.lower() negative infinity: open ray annihilated
	case 12:	return -1;	// rhs infinity, lhs finite

	case 11:		// lhs infinite, rhs.upper() negative infinity: open ray annihilated
	case 7:		// lhs infinite, rhs.lower() negative infinity: open ray annihilated
	case 3:		return 1;	// lhs infinite, rhs finite

	case 9:	// rhs.upper() positive infinity, lhs.lower negative infinity
	case 6:	// lhs.upper() positive infinity, rhs.lower negative infinity
		throw std::runtime_error("(-infinity,infinity) NaN");

	case 8:	// rhs.upper() positive infinity, lhs finite
		rhs.assign(rhs.lower(),lhs.upper());	// reduce to case 10	
		return trivial_sum(lhs.lower(),rhs.lower());	// inline rather than goto
	case 2:	// lhs.upper() positive infinity, rhs finite
		rhs.assign(rhs.lower(),lhs.upper());	// reduce to case 10	
		// intentional fall through
	case 10: return trivial_sum(lhs.lower(),rhs.lower());	// rhs.upper() and lhs.upper() both negative infinity

	case 4:	// rhs.lower() negative infinity, lhs finite
		lhs.assign(rhs.lower(),lhs.upper());	// reduce to case 5
		return trivial_sum(lhs.upper(),rhs.upper());	// inline rather than goto
	case 1:	// lhs.lower() negative infinity, rhs finite
		rhs.assign(lhs.lower(),rhs.upper());	// reduce to case 5
		// intentional fall through
	case 5: return trivial_sum(lhs.upper(),rhs.upper());	// rhs.lower() and lhs.lower() both negative infinity
//	case 0:
//	default:
	}
	// no infinite endpoints survive past here

	const int rhs_zero_code = 2*(rhs.upper()==int_as<0,T>())+(rhs.lower()==int_as<0,T>());
	if (3==rhs_zero_code) return 1;

	const int lhs_zero_code = 2*(lhs.upper()==int_as<0,T>())+(lhs.lower()==int_as<0,T>());
	if (3==lhs_zero_code) return 1;

	switch(3*rhs_zero_code+lhs_zero_code)
	{
	// zero-splices
	case 7:	// rhs.upper() zero, lhs.lower() zero
		lhs.assign(rhs.lower(),lhs.upper());
		rhs = int_as<0,T>();
		return 1;
	case 5:	// rhs.lower() zero, lhs.upper.zero()
		lhs.assign(lhs.lower(),rhs.upper());
		rhs = int_as<0,T>();
		return 1;
#if 0
	case 8:	// rhs.upper(), lhs.upper() zero
		break;
	case 6:	// rhs.upper() zero,
		break;
	case 4:	// rhs.lower() zero, lhs.lower() zero
		break;
	case 3:	// rhs.lower() zero
		break;
	case 2:	// lhs.upper() zero
		break;
	case 1:	// lhs.lower() zero
		break;
//	case 0:
//	default:
#endif
	}

	return 0;
}

// the rearrange_sum family returns true/1 iff rhs has been annihilated with exact arithmetic, 2 if any change
// negative values are reserved for C-style error codes, should they be eventually needed.
template<std::floating_point T> int rearrange_sum(ISK_INTERVAL<T>& lhs, ISK_INTERVAL<T>& rhs)
{
	assert(!trivial_sum(lhs,rhs));
	// all four coordinates are finite
	assert(!isINF(lhs.lower()));
	assert(!isINF(lhs.upper()));
	assert(!isINF(rhs.lower()));
	assert(!isINF(rhs.upper()));

	// we can have zero coordinates leaking through
	const int rhs_zero_code = 2*(rhs.upper()==int_as<0,T>())+(rhs.lower()==int_as<0,T>());
	assert(2>=rhs_zero_code);

	const int lhs_zero_code = 2*(lhs.upper()==int_as<0,T>())+(lhs.lower()==int_as<0,T>());
	assert(2>=lhs_zero_code);

	const int zero_code = 3*rhs_zero_code+lhs_zero_code;
	assert(7!=zero_code);
	assert(5!=zero_code);

	T tmp_bounds[4] = {lhs.lower(), rhs.lower(), lhs.upper(), rhs.upper()};
	bool any_change = false;

#ifdef ZAIMONI_USING_STACKTRACE
	zaimoni::ref_stack<zaimoni::stacktrace, const char*> log(zaimoni::stacktrace::get(), __PRETTY_FUNCTION__);
#endif
	int lower_rearrange_code = 0;
	switch(zero_code)
	{
	case 8:	// rhs.upper(), lhs.upper() zero
	case 6:	// rhs.upper() zero,
	case 2:	// lhs.upper() zero
	case 0:	// no coordinates zero
		lower_rearrange_code = bits::rearrange_sum(tmp_bounds[0],tmp_bounds[1]);
		if (0 > lower_rearrange_code) lower_rearrange_code = -lower_rearrange_code;	// historically non-negative
		break;
	case 1:	// lhs.lower() zero
		swap(tmp_bounds[0],tmp_bounds[1]);	// intentional fall-through
	case 4:	// rhs.lower() zero, lhs.lower() zero
	case 3:	// rhs.lower() zero
		lower_rearrange_code = 1;	// simulate; 0 is rhs.lower()
	}
	int upper_rearrange_code = 0;
	switch(zero_code)
	{
	case 4:	// rhs.lower() zero, lhs.lower() zero
	case 3:	// rhs.lower() zero
	case 1:	// lhs.lower() zero
	case 0:	// no coordinates zero
		upper_rearrange_code = bits::rearrange_sum(tmp_bounds[2],tmp_bounds[3]);
		if (0 > upper_rearrange_code) upper_rearrange_code = -upper_rearrange_code;	// historically non-negative
		break;
	case 2:	// lhs.upper() zero
		swap(tmp_bounds[2],tmp_bounds[3]);	// intentional fall-through		
	case 8:	// rhs.upper(), lhs.upper() zero
	case 6:	// rhs.upper() zero,
		upper_rearrange_code = 1;	// simulate: 0 is rhs.upper()
	}

	// can break down when one side starts exact and the other doesn't
	const int rearrange_code = 3*upper_rearrange_code+lower_rearrange_code;
#if 0
	switch(rearrange_code)
	{
	// upper changed
	case 8:	// lower changed
		break;
	case 7:	// lower rhs zero
		break;
	case 6:	// lower no change
		break;
	// upper went rhs zero
	case 5:	// lower changed
		break;
	case 4:	// lower rhs zero
		break;
	case 3:	// lower no change
		break;
	// no change upper
	case 2:	// lower changed
		break;
	case 1:	// lower rhs zero
		break;
	case 0:	return 0;	// no change
	}
#endif
	// early exit;
	switch(rearrange_code)
	{
	case 0:	return 0;	// no change
	case 4:	// upper, lower rhs zero
		lhs.assign(tmp_bounds[0],tmp_bounds[2]);
		rhs = int_as<0,T>();
		return 1;
	}
	bool direct_legal = (tmp_bounds[0]<=tmp_bounds[2] && tmp_bounds[1]<=tmp_bounds[3]);
	bool chiasm_legal = (tmp_bounds[0]<=tmp_bounds[3] && tmp_bounds[1]<=tmp_bounds[2]);
	// rearrangements that break the ordering invariants are easy to hit in test cases.
	if (direct_legal && chiasm_legal)
		{	// standardize
		if (tmp_bounds[0]>tmp_bounds[1]) swap(tmp_bounds[0],tmp_bounds[1]);
		if (tmp_bounds[2]>tmp_bounds[3]) swap(tmp_bounds[2],tmp_bounds[3]);
		direct_legal = false;	// chiasm mode guaranteed to concentrate the error
		}
	// but assume rearrange failure is not a logic paradox here
	if (chiasm_legal)
		{
		lhs.assign(tmp_bounds[0],tmp_bounds[3]);
		rhs.assign(tmp_bounds[1],tmp_bounds[2]);
		return 2;
		}
	else if (direct_legal)
		{
		lhs.assign(tmp_bounds[0],tmp_bounds[2]);
		rhs.assign(tmp_bounds[1],tmp_bounds[3]);
		return 2;
		}	
	return 0;
}

// exponent values are from frexp
// we assume the lhs has fewer significant digits, so would be more useful to have in the [0.5,1.0) range
// return true iff a change was made
template<class T>
typename std::enable_if<std::is_floating_point<T>::value , bool>::type _rebalance_exponents(T& lhs, T& rhs, int lhs_exponent, int rhs_exponent)
{
	if (1==lhs_exponent) return false;
	const int delta_lhs_exp = lhs_exponent-1;
	const int abs_delta_lhs_exp = (0<delta_lhs_exp ? delta_lhs_exp : -delta_lhs_exp);
	const int clearance = (0<delta_lhs_exp ? std::numeric_limits<T>::max_exponent-rhs_exponent : rhs_exponent-std::numeric_limits<T>::min_exponent);
	if (0==clearance) return false;
	const int abs_delta = (abs_delta_lhs_exp < clearance ? abs_delta_lhs_exp : clearance);
	const int delta_exp = (0<delta_lhs_exp ? abs_delta : -abs_delta);
	lhs = scalbn(lhs,delta_exp);
	rhs = scalbn(rhs,delta_exp);
	return true;
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
	assert(!isNaN(lhs));
	assert(!isNaN(rhs));
	if (int ret = identity_product(lhs,rhs)) return (-2==ret ? -2 : 1==ret);
	if (int ret = identity_product(rhs,lhs)) return (-2==ret ? -2 : -(1==ret));

	const int inf_code = (bool)(isINF(rhs))-(bool)(isINF(lhs));
	const int zero_code = 2*(int_as<0,U>()==rhs)+(int_as<0,T>()==lhs);
	switch(4*inf_code+zero_code)
	{
	case 4+1:
	case -4+2:	throw std::runtime_error("0*infinity NaN");;
	case -4+0:
	case 0+1:
	case 0+3:
		return set_signbit(lhs,signBit(lhs)!=signBit(rhs));
	case 4+0:	
	case 0+2:
		return -set_signbit(rhs,signBit(lhs)!=signBit(rhs));
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
typename std::enable_if<std::is_floating_point<T>::value , int>::type trivial_product(ISK_INTERVAL<T>& lhs, T& rhs)
{
	assert(!isNaN(lhs));
	assert(!std::isnan(rhs));
	if (lhs.lower()==lhs.upper())
		{	// allow for negative zero fp weirdness: upper() rather than lower()
		T tmp_lhs = lhs.upper();
		const int ret = trivial_product(tmp_lhs,rhs);
		if (ret) lhs = tmp_lhs;
		return ret;
		}

	if (int ret = identity_product(lhs,rhs)) return 1==ret;

	// intervals are only bounded by infinity
	if (std::isinf(rhs)) {
		if (0.0==lhs.lower() || 0.0==lhs.upper()) throw std::runtime_error("0*infinity NaN");
		if (std::signbit(lhs.lower())!=std::signbit(lhs.upper())) throw std::runtime_error("interval (-infinity,infinity) NaN");
		return -set_signbit(rhs,std::signbit(lhs.lower())!=std::signbit(rhs));
	}

	if (0.0 == rhs) return -set_signbit(rhs,std::signbit(lhs.upper())==std::signbit(rhs));

	return 0;
}

template<class T>
typename std::enable_if<std::is_floating_point<T>::value , int>::type trivial_product(ISK_INTERVAL<T>& lhs, ISK_INTERVAL<T>& rhs)
{
	assert(!isNaN(lhs));
	assert(!isNaN(rhs));
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
	if (std::isinf(lhs.upper()))	\
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
	if (std::isinf(lhs.lower()))	\
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
template<std::floating_point F> bool rearrange_product(F& lhs, F& rhs)
{
	assert(!trivial_product(lhs,rhs));
#ifdef ZAIMONI_USING_STACKTRACE
	zaimoni::ref_stack<zaimoni::stacktrace, const char*> log(zaimoni::stacktrace::get(), __PRETTY_FUNCTION__);
#endif

	auto l_stat = edit_fp(lhs);
	auto r_stat = edit_fp(rhs);
	int predicted_exponent = l_stat.predict_product_exponent(r_stat);

	// 1.0*1.0 is 1.0
	if (l_stat.product_is_exact()) {
		if (l_stat.exponent_is_normal(predicted_exponent))
			{
exact_product:
			lhs *= rhs;
			rhs = 1.0;
			return true;
			}
		l_stat.rebalance(r_stat);
		return false;
	}
	if (r_stat.product_is_exact()) {
		if (l_stat.exponent_is_normal(predicted_exponent)) goto exact_product;
		r_stat.rebalance(l_stat);
		return false;
	}

	ISK_INTERVAL<double> predicted_mantissa(l_stat.mantissa()); // XXX \todo fix where long double better than double
	predicted_mantissa *= r_stat.mantissa();
	if (predicted_mantissa.lower()==predicted_mantissa.upper())
		{
		if (0.5<=predicted_mantissa.lower() || -0.5>=predicted_mantissa.upper()) predicted_exponent++;
		if (l_stat.exponent_is_normal(predicted_exponent)) goto exact_product;
		// \todo adjusting the mantissas is still justifiable
		}

	// XXX want to see the mantissas as integers to decide which one to optimize
	if (l_stat.mantissa_as_int() < r_stat.mantissa_as_int()) {
		l_stat.rebalance(r_stat);
	} else {
		r_stat.rebalance(l_stat);
	}
	return false;
}

template<std::floating_point F> bool rearrange_product(ISK_INTERVAL<F>& lhs, ISK_INTERVAL<F>& rhs)
{
	return false;	// no-op to allow compiling
}

// 1 success, 0 no-op, -2 failed to evaluate
template<class T, class U>
typename std::enable_if<ZAIMONI_INT_AS_DEFINED(U) , int>::type identity_quotient(T& lhs, const U& identity)
{
	if (int_as<1,U>() == identity) return 1;
	if (int_as<-1,U>() != identity) return 0;
	return self_negate(lhs) ? 1 : -2;
}


// trivial_quotient family returns -1 for lhs annihilated, 1 for rhs annihilated; -2 on error
template<class T, class U>
typename std::enable_if<std::is_arithmetic<T>::value && std::is_arithmetic<U>::value, int>::type trivial_quotient(T& lhs, U& rhs)
{
	assert(!isnan(lhs));
	assert(!isnan(rhs));

	if (int_as<0,U>()==rhs)
		{
		if (int_as<0,T>()==lhs)  throw std::runtime_error("0/0 NaN");
		if (!isinf(lhs)) throw std::overflow_error("division by zero");
		// infinity/0 is ...not great, but at least not degenerating
		set_signbit(lhs,signbit(lhs)!=signbit(rhs));
		return 1;
		}
	if (isinf(lhs))
		{
		if (isinf(rhs)) throw std::runtime_error("infinity/infinity NaN");
		set_signbit(lhs,signbit(lhs)!=signbit(rhs));
		return 1;		
		}
	if (isinf(rhs))
		{
		bool is_negative = (signbit(lhs)!=signbit(rhs));
		lhs = 0.0;
		set_signbit(lhs,is_negative);
		return 1;
		}
	return identity_quotient(lhs,rhs);
}

template<class T, class U>
typename std::enable_if<std::is_arithmetic<T>::value && std::is_floating_point<U>::value, int>::type trivial_quotient(T& lhs, ISK_INTERVAL<U>& rhs)
{
	assert(!isnan(lhs));
	assert(!isnan(rhs));

	if (rhs.lower()==rhs.upper())
		{
		T tmp(rhs.upper());
		int ret = trivial_product(lhs,tmp);
		if (ret) rhs = tmp;
		return ret;
		}
	if (0.0 >= rhs.lower() && 0.0 <= rhs.upper()) throw std::overflow_error("division by interval containing zero");
	if (isinf(lhs))
		{
		set_signbit(lhs,signbit(lhs)!=signbit(rhs));
		return 1;
		}
	return 0;
}

template<class T, class U>
typename std::enable_if<std::is_floating_point<T>::value && std::is_arithmetic<U>::value, int>::type trivial_quotient(ISK_INTERVAL<T>& lhs, U& rhs)
{
	assert(!isnan(lhs));
	assert(!isnan(rhs));

	if (lhs.lower()==lhs.upper())
		{
		T tmp(lhs.upper());
		int ret = trivial_product(tmp,rhs);
		if (ret) lhs = tmp;
		return ret;
		}
	if (int_as<0,U>()==rhs) throw std::overflow_error("division by zero");
	if (isinf(rhs))
		{
		T tmp = 0.0;
		if (!signbit(lhs.lower()))
			{
			set_signbit(tmp,signbit(rhs));
			lhs = tmp;
			return 1;
			}
		if (signbit(lhs.upper()))
			{
			set_signbit(tmp,!signbit(rhs));
			lhs = tmp;
			return 1;
			}
		set_signbit(tmp,true);
		lhs.assign(tmp,0.0);
		return 1;
		}
	return identity_quotient(lhs,rhs);
}

template<class T>
typename std::enable_if<std::is_floating_point<T>::value, int>::type trivial_quotient(ISK_INTERVAL<T>& lhs, ISK_INTERVAL<T>& rhs)
{
	assert(!isnan(lhs));
	assert(!isnan(rhs));

	if (rhs.lower()==rhs.upper())
		{
		T tmp(rhs.upper());
		int ret = trivial_product(lhs,tmp);
		if (ret) rhs = tmp;
		return ret;
		}
	if (lhs.lower()==lhs.upper())
		{
		T tmp(lhs.upper());
		int ret = trivial_product(tmp,rhs);
		if (ret) lhs = tmp;
		return ret;
		}
	if (causes_division_by_zero(rhs)) throw std::overflow_error("division by interval containing zero");
	return 0;
}

inline void eval_series_product(int_range<uintmax_t>& src, uintmax_t& accumulator,uintmax_t& power_of_2)	// XXX would be ok for non-header implementation, but may need to convert to template instead
{
	assert(1<=src.lower());	// special cases to handle this...probably would benefit from a true class

	while(!src.empty()) {
		uintmax_t tmp = src.lower();
		unsigned tmp_power_of_2 = 0;
		while(0==tmp%2) {
			tmp /= 2;
			tmp_power_of_2++;
		};
		if (UINTMAX_MAX/accumulator < tmp || UINTMAX_MAX-power_of_2<tmp_power_of_2) return;
		accumulator *= tmp;
		power_of_2 += tmp_power_of_2;
		src.pop_front();
	}
}

// XXX lhs corresponds to both numerator and accumulator in quotient_of_series_products
template<class base>
typename std::enable_if<
	    numerical<base>::error_tracking
    && !is_series_product<base>::value,	// these should go to a quotient_of_series_products
base>::type quotient_by_series_product(base lhs, int_range<uintmax_t> divisor)
{
	assert(!isNaN(lhs));
	if (divisor.empty()) return lhs;
	if (1==divisor.lower() && 1==divisor.upper()) return lhs;
	if (0>=divisor.lower()) throw std::runtime_error("division by zero NaN " __FILE__ ":" DEEP_STRINGIZE(__LINE__));

	if (1==divisor.lower()) divisor.pop_front();
	else if (1==divisor.upper()) divisor.pop_back();
	if (divisor.lower()==divisor.upper()) return quotient(lhs,uint_as<base>(divisor.lower()));

	// intermediate data structures
	uintmax_t quotient_accumulator = 1;
	// could replace these by two arbitrary-precision integers
	uintmax_t quotient_power_of_2 = 0;
	dicounter numerator_power_of_2;	// zero-initializes

	auto lhs_stats(get_fp_stats(lhs));	// XXX corresponds to both base_stats and accumulator_stats in quotient_of_series_of_products

	if (1>lhs_stats.exponent() && numerator_power_of_2.sub_capacity()>=1)
		{	// underflow defense
		int delta_exponent = 1-lhs_stats.exponent();
		lhs = scalBn(lhs,delta_exponent);
		numerator_power_of_2.safe_sub(delta_exponent);
		lhs_stats = lhs;
		}

	while((!divisor.empty() || 1<quotient_accumulator))
		{
		if (!divisor.empty() && 1==quotient_accumulator) eval_series_product(divisor,quotient_accumulator,quotient_power_of_2);

		if (0<quotient_power_of_2) numerator_power_of_2.sub(quotient_power_of_2); 
		if (0<numerator_power_of_2.negative() && 1<lhs_stats.exponent())
			{
			int delta = lhs_stats.exponent()-1;
			if (numerator_power_of_2.sub_capacity() < delta) delta = numerator_power_of_2.sub_capacity();
			if (0<delta)
				{
				lhs = scalBn(lhs,-delta);
				numerator_power_of_2.safe_add(delta);
				lhs_stats = lhs;
				continue;
				}
			}

		// try to concel out factors
		if (1<quotient_accumulator)
			{
			uintmax_t test = gcd(quotient_accumulator,lhs_stats.divisibility_test());
			if (1<test)
				{	// don't bother trying to recover against numerator.base();
				lhs_stats.missed_good_exponent_by(INT_LOG2(test)+2, lhs, numerator_power_of_2);
				lhs /= uint_as<base>(test);
				lhs_stats.update(lhs,numerator_power_of_2);
				quotient_accumulator /= test;
				continue;
				}
			}

		if (1<quotient_accumulator)
			{
			int delta = lhs_stats.missed_good_exponent_by(INT_LOG2(quotient_accumulator)+2, lhs, numerator_power_of_2);
			lhs /= uint_as<base>(quotient_accumulator);
			lhs_stats.update(lhs,numerator_power_of_2);
			quotient_accumulator = 1;
			continue;
			}
		};	// main loop: k..m not reduced

	assert(divisor.empty());

	lhs_stats.prepare_return_value(lhs,numerator_power_of_2);

	if (!isFinite(lhs)) throw std::overflow_error("overflow: quotient by series product");
	if (0==lhs_stats.safe_2_n_multiply() && numerator_power_of_2.positive()) throw std::overflow_error("quotient by series product");
	if (0==lhs_stats.safe_2_n_divide() && numerator_power_of_2.negative())
		{	// underflow
		if (std::numeric_limits<long double>::digits<numerator_power_of_2.negative()) return scalBn(lhs,-std::numeric_limits<long double>::digits-1);
		return scalBn(lhs,-((int)(numerator_power_of_2.negative())));
		}

	return lhs;
}

// cf. return type of power_term::standardize()
template<class base>
typename std::enable_if<
	   numerical<base>::error_tracking 
	&& std::is_same<base, typename interval_type<base>::type>::value,
base>::type quotient(power_term<base,uintmax_t> numerator, base rhs)
{
	assert(!isNaN(numerator));
	if (1==numerator.power()) return quotient(numerator.base(),rhs);
	if (rhs == 1.0) return eval(numerator);
	if (causes_division_by_zero(rhs)) throw std::runtime_error("division by zero NaN " __FILE__ ":" DEEP_STRINGIZE(__LINE__));

	// intermediate data structures
	base accumulator(int_as<1,base>());
	// could replace these by two arbitrary-precision integers
	dicounter numerator_power_of_2;	// zero-initializes

	auto base_stats(get_fp_stats(numerator.base()));
	auto accumulator_stats(get_fp_stats(accumulator));
	auto rhs_stats(get_fp_stats(rhs));

	if (1>base_stats.exponent() && numerator_power_of_2.sub_capacity()>=numerator.power())
		{	// underflow defense
		int delta_exponent = 1-base_stats.exponent();
		if (UINTMAX_MAX/numerator.power()<delta_exponent) delta_exponent = UINTMAX_MAX/numerator.power();
		numerator.base() = scalBn(numerator.base(),delta_exponent);
		numerator_power_of_2.safe_sub(delta_exponent*numerator.power());
		base_stats = numerator.base();
		}

	rhs_stats.inv_update(rhs,numerator_power_of_2);

	while(0<numerator.power())
		{
		if (0<numerator_power_of_2.negative() && 1<base_stats.exponent())
			{
			int delta = base_stats.exponent()-1;
			if (numerator_power_of_2.sub_capacity()/numerator.power() < delta) delta = numerator_power_of_2.sub_capacity()/numerator.power();
			if (0<delta)
				{
				numerator.base() = scalBn(numerator.base(),-delta);
				numerator_power_of_2.safe_add(delta*numerator.power());
				base_stats = numerator.base();
				continue;
				}
			}

		// try to cancel out factors
		{
		uintmax_t test = gcd(rhs_stats.divisibility_test(),accumulator_stats.divisibility_test());
		if (1<test)
			{	// don't bother trying to recover against numerator.base();
			const base tmp((typename numerical<base>::exact_type)test);
			accumulator /= tmp;
			rhs /= tmp;
			accumulator_stats.update(accumulator,numerator_power_of_2);
			rhs_stats.inv_update(rhs,numerator_power_of_2);
			continue;
			}
		}

		if (1==numerator.power()%2)
			{	// XXX should be operation of power_term
			numerator.apply_factor(accumulator);
			accumulator_stats.update(accumulator,numerator_power_of_2);
			}
		else{	// XXX should be operation of power_term
			numerator.base() = square(numerator.base());
			numerator.power() /= 2;
			base_stats = numerator.base();	// won't underflow
			}
		};	// main loop: both x^n and k..m not reduced

	quotient(accumulator,rhs);
	accumulator_stats.prepare_return_value(accumulator,numerator_power_of_2);

	if (!isFinite(accumulator)) throw std::overflow_error("overflow: quotient of series products");
	if (0==accumulator_stats.safe_2_n_multiply() && numerator_power_of_2.positive()) throw std::overflow_error("quotient of series products");
	if (0==accumulator_stats.safe_2_n_divide() && numerator_power_of_2.negative())
		{	// underflow
		if (std::numeric_limits<long double>::digits<numerator_power_of_2.negative()) return scalBn(accumulator,-std::numeric_limits<long double>::digits-1);
		return scalBn(accumulator,-((int)(numerator_power_of_2.negative())));
		}

	return accumulator;
}

// cf. return type of power_term::standardize()
template<class base>
typename std::enable_if<
	   numerical<base>::error_tracking 
	&& std::is_same<base, typename interval_type<base>::type>::value,
base>::type quotient_of_series_products(power_term<base,uintmax_t> numerator, int_range<uintmax_t> divisor)
{
	assert(!isNaN(numerator));
	if (divisor.empty()) return eval(numerator);
	if (1==divisor.lower() && 1==divisor.upper()) return eval(numerator);
	if (0>=divisor.lower()) throw std::runtime_error("division by zero NaN " __FILE__ ":" DEEP_STRINGIZE(__LINE__));

	if (1==divisor.lower()) divisor.pop_front();
	else if (1==divisor.upper()) divisor.pop_back();
	if (1==numerator.power()) return quotient_by_series_product(numerator.base(),divisor);
	if (divisor.lower()==divisor.upper()) return quotient(numerator,uint_as<base>(divisor.lower()));

	// intermediate data structures
	base accumulator(int_as<1,base>());
	uintmax_t quotient_accumulator = 1;
	// could replace these by two arbitrary-precision integers
	uintmax_t quotient_power_of_2 = 0;
	dicounter numerator_power_of_2;	// zero-initializes

	auto base_stats(get_fp_stats(numerator.base()));
	auto accumulator_stats(get_fp_stats(accumulator));

	if (1>base_stats.exponent() && numerator_power_of_2.sub_capacity()>=numerator.power())
		{	// underflow defense
		int delta_exponent = 1-base_stats.exponent();
		if (UINTMAX_MAX/numerator.power()<delta_exponent) delta_exponent = UINTMAX_MAX/numerator.power();
		numerator.base() = scalBn(numerator.base(),delta_exponent);
		numerator_power_of_2.safe_sub(delta_exponent*numerator.power());
		base_stats = numerator.base();
		}

	while((!divisor.empty() || 1<quotient_accumulator) && (0<numerator.power() || 0<quotient_power_of_2 || 0<numerator_power_of_2.positive() || 0<numerator_power_of_2.negative()))
		{
		if (!divisor.empty() && 1==quotient_accumulator) eval_series_product(divisor,quotient_accumulator,quotient_power_of_2);

		if (0<quotient_power_of_2) numerator_power_of_2.sub(quotient_power_of_2); 
		if (0<numerator_power_of_2.negative() && 1<base_stats.exponent())
			{
			int delta = base_stats.exponent()-1;
			if (numerator_power_of_2.sub_capacity()/numerator.power() < delta) delta = numerator_power_of_2.sub_capacity()/numerator.power();
			if (0<delta)
				{
				numerator.base() = scalBn(numerator.base(),-delta);
				numerator_power_of_2.safe_add(delta*numerator.power());
				base_stats = numerator.base();
				continue;
				}
			}

		// try to concel out factors
		if (1<quotient_accumulator)
			{
			uintmax_t test = gcd(quotient_accumulator,accumulator_stats.divisibility_test());
			if (1<test)
				{	// don't bother trying to recover against numerator.base();
				accumulator_stats.missed_good_exponent_by(INT_LOG2(test)+2, accumulator, numerator_power_of_2);
				accumulator /= uint_as<base>(test);
				accumulator_stats.update(accumulator,numerator_power_of_2);
				quotient_accumulator /= test;
				continue;
				}
			test = gcd(quotient_accumulator,base_stats.divisibility_test());
			if (1<test)
				{
				base tmp(numerator.base());
				tmp /= uint_as<base>(test);
				accumulator *= tmp;	// XXX
				numerator.power()--;
				accumulator_stats.update(accumulator,numerator_power_of_2);
				quotient_accumulator /= test;
				continue;
				}
			}

		if (1<quotient_accumulator)
			{
			int delta = accumulator_stats.missed_good_exponent_by(INT_LOG2(quotient_accumulator)+2, accumulator, numerator_power_of_2);
			if (0<delta)
				{
				if (1<base_stats.exponent()) continue;
				if (1==base_stats.exponent())
					{	// exponent should be 1 we werent' able to restart
						// so squaring will not overflow
					if (1==numerator.power()%2)
						{	// XXX should be operation of power_term
						accumulator_stats = numerator.apply_factor(accumulator);
						continue;
						}
					else{	// XXX should be operation of power_term
						numerator.base() = square(numerator.base());
						numerator.power() /= 2;
						base_stats = numerator.base();	// won't underflow
						continue;
						}
					}
				}
			accumulator /= uint_as<base>(quotient_accumulator);
			accumulator_stats.update(accumulator,numerator_power_of_2);
			quotient_accumulator = 1;
			continue;
			}
		};	// main loop: both x^n and k..m not reduced

	while(0<numerator.power())
		{	// 
		if (1==numerator.power()%2)
			{	// XXX should be operation of power_term
			numerator.apply_factor(accumulator);
			accumulator_stats.update(accumulator,numerator_power_of_2);
			}
		else{	// XXX should be operation of power_term
			numerator.base() = square(numerator.base());
			numerator.power() /= 2;
			base_stats = numerator.base();	// won't underflow
			}
		}
	assert(divisor.empty());

	accumulator_stats.prepare_return_value(accumulator,numerator_power_of_2);

	if (!isFinite(accumulator)) throw std::overflow_error("overflow: quotient of series products");
	if (0==accumulator_stats.safe_2_n_multiply() && numerator_power_of_2.positive()) throw std::overflow_error("quotient of series products");
	if (0==accumulator_stats.safe_2_n_divide() && numerator_power_of_2.negative())
		{	// underflow
		if (std::numeric_limits<long double>::digits<numerator_power_of_2.negative()) return scalBn(accumulator,-std::numeric_limits<long double>::digits-1);
		return scalBn(accumulator,-((int)(numerator_power_of_2.negative())));
		}
	return accumulator;
}

template<class base>
typename std::enable_if<
	   numerical<base>::error_tracking 
	&& std::is_same<base, typename interval_type<base>::type>::value,
base>::type quotient_of_series_products(const power_term<base,uintmax_t>& numerator, int_range<intmax_t> divisor)
{
	assert(!isNaN(numerator));
	if (divisor.empty()) return eval(numerator);
	if (1==divisor.lower() && 1==divisor.upper()) return eval(numerator);
	if (0>=divisor.lower() && 0<=divisor.upper()) throw std::runtime_error("division by zero NaN " __FILE__ ":" DEEP_STRINGIZE(__LINE__));

	if (0<divisor.lower()) return quotient_of_series_products(numerator,int_range<uintmax_t>(divisor.lower(),divisor.upper()));
	if (-std::numeric_limits<intmax_t>::max()<=divisor.lower())
		{
		base tmp = quotient_of_series_products(numerator,int_range<uintmax_t>(-divisor.upper(),-divisor.lower()));
		return 1==(divisor.upper()-divisor.lower())%2 ? tmp : -tmp;
		}
	return int_as<base>(divisor.lower())*quotient_of_series_products(numerator,int_range<intmax_t>(divisor.lower()+1,divisor.upper()));
}

template<class base, class power, class divisor_type>
typename std::enable_if<
	    numerical<base>::error_tracking 
	&& !std::is_same<power_term<base,power>, typename power_term<base,power>::canonical_type>::value,
typename interval_type<base>::type>::type quotient_of_series_products(const power_term<base,power>& numerator, const int_range<divisor_type>& divisor)
{
	return quotient_of_series_products(numerator.standardize(), divisor);
}

}	// namespace math
}	// namespace zaimoni

#endif
