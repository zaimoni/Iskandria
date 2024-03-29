#ifndef INTERVAL_HPP
#define INTERVAL_HPP 1

// interval.hpp
// interval arithmetic.  Generally follows Boost API design decisions, but specified to not have the critical bugs that should have blocked release (but didn't)

#include <fenv.h>
#include "augment.STL/cmath"
#include "numeric_error.hpp"


#ifdef ZAIMONI_USING_STACKTRACE
#include "Pure.CPP/stacktrace.hpp"
#endif

#pragma STD FENV_ACCESS ON

namespace zaimoni {

namespace math {

template<class T> class trivial;

namespace bits {

template<std::floating_point F> int round_get() { return fegetround(); }
template<std::integral T> int round_get() {return FE_TONEAREST;}
template<std::floating_point F> void round_set(int mode) { fesetround(mode); }
template<std::integral T> void round_set(int mode) {}

template<std::unsigned_integral T> int rearrange_sum(T& lhs, T& rhs)
{
	if (std::numeric_limits<T>::max() <= lhs) return 0;
	const T ub = std::numeric_limits<T>::max() - lhs;
	if (ub >= rhs) {
		lhs += rhs;
		rhs = 0;
		return -1;
	}
	lhs = std::numeric_limits<T>::max();
	rhs -= ub;
	return -2;
}

template<std::signed_integral T> int rearrange_sum(T& lhs, T& rhs)
{
	if (((0 <= lhs) ? (0 >= rhs) : (0 <= rhs)) || 0==lhs) {
		lhs += rhs;
		rhs = 0;
		return -1;
	}
	const bool lhs_positive = (0 < lhs);
	if (lhs_positive ? (std::numeric_limits<T>::max() <= lhs) : (std::numeric_limits<T>::min() >= lhs)) return 0;
	const T extreme = lhs_positive ? std::numeric_limits<T>::max() - lhs : std::numeric_limits<T>::min() - lhs;
	if (lhs_positive ? (extreme >= rhs) : (extreme <= rhs)) {
		lhs += rhs;
		rhs = 0;
		return -1;
	}
	lhs = lhs_positive ? std::numeric_limits<T>::max() : std::numeric_limits<T>::min();
	rhs -= extreme;
	return -2;
}

template<class T>
typename std::enable_if<std::is_integral<T>::value && std::is_unsigned<T>::value, int>::type rearrange_product(T& lhs, T& rhs)
{
	if (std::numeric_limits<T>::max() <= lhs) return 0;
	const T ub = std::numeric_limits<T>::max()/lhs;
	if (ub >= rhs) {
		lhs *= rhs;
		rhs = 1;
		return -1;
	}
	return 0;
}

}	// namespace bits

template<class T>
class rearrange
{
public:
	// 0: no action
	// -1: keep LHS; 1: keep RHS (never happens with default implementation)
	// -2: incomplete rearrangement
	static int sum(T& lhs, T& rhs) {
#ifdef ZAIMONI_USING_STACKTRACE
		zaimoni::ref_stack<zaimoni::stacktrace, const char*> log(zaimoni::stacktrace::get(), __PRETTY_FUNCTION__);
#endif
		switch(const int code = trivial<T>::sum_c(lhs, rhs))
		{
		case 1:
			lhs = std::move(rhs);
			rhs = 0;
		case -1:
			return -1;
		}
		return bits::rearrange_sum(lhs, rhs);	// type-specific handling
	}

	static int product(T& lhs, T& rhs) {
		int code = trivial<T>::product_c(lhs, rhs);
		if (0 < code) {
			lhs = std::move(rhs);
			rhs = 1;
			code = -code;
		}
		if (0 > code) {
			if (-2 == code) lhs = -lhs;	// \todo want self-negate forwarder
			return -1;
		}
		return bits::rearrange_product(lhs, rhs);	// type-specific handling
	}
};

// two uses of trivial: read-only and destructive
template<class T>
class trivial
{
public:
	// 0: no action
	// -1: keep LHS; 1: keep RHS
	static int sum_c(const_param_t<T> lhs, const_param_t<T> rhs)
	{
		if (isINF(lhs)) {
			if (isINF(rhs) && signBit(lhs) != signBit(rhs)) throw numeric_error("+: NaN");
			return -1;
		} else if (isINF(rhs) || is_zero(lhs)) {
			return 1;
		} else if (is_zero(rhs)) return -1;
		// further handling is type-specific -- do not attempt here
		return 0;
	}

	static bool sum(T& lhs, T rhs)
	{
		T revert(lhs);
		switch(rearrange<T>::sum(lhs, rhs))
		{
		case 1: lhs = std::move(rhs);	// intentional fall-through
		case -1: return true;
		case -2:	// not using partial rearrangement here
			lhs = revert;
		case 0:
			return false;
		default: _fatal_code("trivial::sum: rearrange<T>::sum code out of range", 3);
		}
	}

	// 0: no action
	// -1: keep LHS; 1: keep RHS
	// -2: keep LHS,negated; 2: keep RHS, negated
	static int product_c(const_param_t<T> lhs, const_param_t<T> rhs) {
		const bool is_negative = (signBit(lhs) != signBit(rhs));
		if (isINF(lhs)) {
			if (contains_zero(rhs)) throw numeric_error("*: NaN");
			return (signBit(lhs) == is_negative) ? -1 : -2;
		} else if (isINF(rhs)) {
			if (contains_zero(lhs)) throw numeric_error("*: NaN");
			return (signBit(rhs) == is_negative) ? 1 : 2;
		} else if (is_zero(lhs)) {
			return (!op_works<T>::has_negative_zero || signBit(lhs) == is_negative) ? -1 : -2;
		} else if (is_zero(rhs)) {
			return (!op_works<T>::has_negative_zero || signBit(rhs) == is_negative) ? 1 : 2;
		} else if (is_one(lhs)) return 1;
		else if (is_one(rhs)) return -1;
		else if (is_negative_one(lhs)) return 2;
		else if (is_negative_one(rhs)) return -2;
		// else if (are_mult_inverses(lhs, rhs)) return 3;	// requires <cmath> for floating point; example of further type-sensitive processing
		return 0;
	}
};

// no effort is made by this class to conserve rounding mode.
template<class T>
class interval
{
public:
	static_assert(std::numeric_limits<T>::is_specialized, "this interval type wants accurate numerical stats from std::numeric_limits");
	typedef T base_type;
private:
	T _lb;
	T _ub;

	static const interval _empty;	// self-constexpr doesn't work in either CLang or MingW64
	static const interval _whole;
public:
	constexpr interval() = default;
	constexpr interval(const interval& src) = default;
	constexpr interval(const T& src) noexcept(std::is_nothrow_copy_constructible_v<T>) : _lb(src), _ub(src) {}	// this constructor could get expensive.
	constexpr interval(const T& l, const T& u) noexcept(std::is_nothrow_copy_constructible_v<T>) : _lb(l), _ub(u) {}
	template<class T1> constexpr interval(const interval<T1> &src) noexcept(std::is_nothrow_constructible_v<T, T1>) : _lb(src.lower()), _ub(src.upper()) {}
	~interval() = default;

	interval& operator=(const interval& src) = default;
	interval& operator=(const T& src) noexcept(std::is_nothrow_copy_assignable_v<T>) { _lb = src; _ub = src; return *this; }
	template<class T1> interval& operator=(const interval<T1> &src) noexcept(std::is_nothrow_assignable_v<T, T1>) { _lb = src.lower(); _ub = src.upper(); return *this; }

	void assign(const T& l, const T& u) noexcept(std::is_nothrow_copy_assignable_v<T>) { _lb = l; _ub = u; }

	constexpr T lower() const { return _lb; }
	constexpr T upper() const { return _ub; }
	interval<T> median() const;	// from Boost
	T width() const;	// from Boost

	static interval empty() { return _empty; };	// intentionally value-copy
	static interval whole() { return _whole; };	// intentionally value-copy
	static interval hull(const T& x, const T& y);

	// these two should fail for unsigned integers
	void self_negate() { assign(-_ub, -_lb); };
	constexpr interval operator-() const { return interval(-_ub,-_lb);  };

	// operator==(interval,interval) doesn't work as expected
	constexpr bool contains(const_param_t<T> s) const { return _lb <= s && s <= _ub; }	// acts like R# rather than R in that infinity gets counted as contained
	constexpr bool contains(const interval& s) const { return _lb <= s._lb && s._ub <= _ub; }

	// users that want to be denormalized (lowerbound > upper bound legal), such as a true angle class, should compensate appropriately before using operators * or / and restore their normal form after.
	interval& operator+= (const interval& rhs);
	interval& operator-= (const interval& rhs);
	interval& operator*= (const interval& rhs);
	interval& operator/= (const interval& rhs);

	void self_intersect(const interval& x) {
		if (_lb < x._lb) _lb = x._lb;
		if (_ub > x._ub) _ub = x._ub;
	}
	void self_union(const interval& x) {
		if (_lb > x._lb) _lb = x._lb;
		if (_ub < x._ub) _ub = x._ub;
	}
	void self_union_lower(const_param_t<T> x) { if (_lb > x) _lb = x; }
	void self_union_upper(const_param_t<T> x) { if (_ub < x) _ub = x; }

//	catch these once the interval forms are stable
//	interval& operator+= (T const &r);
//	interval& operator-= (T const &r);
//	interval& operator*= (T const &r);
//	interval& operator/= (T const &r);
};

template<class T> interval<T> operator+(interval<T> lhs, const interval<T>& rhs) { return lhs += rhs; }
template<class T> interval<T> operator+(interval<T> lhs, const_param_t<T> rhs) { return lhs += rhs; }
template<class T> interval<T> operator+(const_param_t<T> lhs, interval<T> rhs) { return rhs += lhs; }

template<class T> interval<T> operator-(interval<T> lhs, const interval<T>& rhs) { return lhs -= rhs; }
template<class T> interval<T> operator-(interval<T> lhs, const_param_t<T> rhs) { return lhs -= rhs; }
template<class T> interval<T> operator-(const_param_t<T> lhs, interval<T> rhs) { rhs.self_negate();  return rhs += lhs; }

template<class T> interval<T> operator*(interval<T> lhs, const interval<T>& rhs) { return lhs *= rhs; }
template<class T> interval<T> operator*(interval<T> lhs, const_param_t<T> rhs) { return lhs *= rhs; }
template<class T> interval<T> operator*(const_param_t<T> lhs, interval<T> rhs) { return rhs *= lhs; }	// assumes commutative multiplication (true for R)

template<class T> interval<T> operator/(interval<T> lhs, const interval<T>& rhs) { return lhs /= rhs; }
template<class T> interval<T> operator/(interval<T> lhs, const_param_t<T> rhs) { return lhs /= rhs; }
template<class T> interval<T> operator/(const_param_t<T> lhs, const interval<T>& rhs) { return interval<T>(lhs) /= rhs; }

// we don't want to mess with operator== for intervals because it's counterintuitive
template<class T>
constexpr bool operator==(const interval<T>& i, const_param_t<T> s)
{
	return i.lower() == s && i.upper() == s;
}

static_assert(interval(1.0) == 1.0); // direct
static_assert(1.0 == interval(1.0)); // automatic
static_assert(interval(1.0) != 2.0);
static_assert(2.0 != interval(1.0));

// Boost library assumes policy involved for scalar inequality with respect to intervals.
// Use std::partial_ordering instead.
template<class T>
constexpr std::partial_ordering operator<=>(const interval<T>& i, const_param_t<T> s)
{
	if (s < i.lower()) return std::partial_ordering::greater;
	if (i.upper() < s) return std::partial_ordering::less;
	if (i == s) return std::partial_ordering::equivalent;
	return std::partial_ordering::unordered;
}

static_assert(interval(1.0) < 2.0);
static_assert(1.0 < interval(2.0));
static_assert(interval(2.0) > 1.0);
static_assert(2.0 > interval(1.0));
static_assert(interval(1.0) <= 1.0);
static_assert(1.0 <= interval(1.0));
static_assert(interval(1.0) >= 1.0);
static_assert(1.0 >= interval(1.0));

template<class T>
constexpr std::partial_ordering operator<=>(const interval<T>& lhs, const interval<T>& rhs)
{
	const auto r_upper_test = lhs <=> rhs.upper();	// switch statement hard-errors
	if (std::partial_ordering::greater == r_upper_test) return std::partial_ordering::greater;
	if (std::partial_ordering::unordered == r_upper_test) return std::partial_ordering::unordered;
	if (std::partial_ordering::equivalent == r_upper_test)
		return rhs.upper()==rhs.lower() ? std::partial_ordering::equivalent : std::partial_ordering::unordered;

	if (lhs.upper() < rhs) return std::partial_ordering::less;
	return std::partial_ordering::unordered;
}

template<class T> interval<T>  const interval<T>::_empty(std::numeric_limits<T>::has_quiet_NaN ? -std::numeric_limits<T>::quiet_NaN() : T(1), std::numeric_limits<T>::has_quiet_NaN ? std::numeric_limits<T>::quiet_NaN() : T(0));
template<class T> interval<T>  const interval<T>::_whole(std::numeric_limits<T>::has_infinity ? -std::numeric_limits<T>::infinity() : std::numeric_limits<T>::min(), std::numeric_limits<T>::has_infinity ? std::numeric_limits<T>::infinity() : std::numeric_limits<T>::max());

} // namespace math

// type_traits

template<class T>
auto norm(const zaimoni::math::interval<T>& x)
{
	const auto l_norm = norm(x.lower());
	const auto u_norm = norm(x.upper());
	return l_norm < u_norm ? u_norm : l_norm;
}

template<class T>
int sgn(const math::interval<T>& x) {
	if (is_positive(x.lower())) return 1;
	if (is_negative(x.upper())) return -1;
	return 0;
}

namespace math {

template<class T>
interval<T> interval<T>::hull(const T& x, const T& y)
{
	if (isNaN(x)) {
		if (isNaN(y)) throw numeric_error("interval::hull cannot recover from double-NaN");
		return interval<T>(y);
	}
	else if (isNaN(y)) return interval<T>(x);
	return x <= y ? interval<T>(x, y) : interval<T>(y, x);
}

template<class T> interval<T> square(const interval<T>& x) {
	if (0 != sgn(x) || is_zero(x.lower()) || is_zero(x.upper())) return x * x;
	const bool favor_ub = x.upper() >= x.lower();
	interval<T> tmp(favor_ub ? T(0) : x.lower(), favor_ub ? x.upper() : -T(0));
	return tmp * tmp;
}

template<class T> interval<T> sqrt(const interval<T>& x) {
	if (isNaN(x)) return x;
	if (T(0) > x.lower()) throw numeric_error("interval sqrt domain error");
	if (is_zero(x.upper())) return 0.0;
	if (isINF(x.lower())) return std::numeric_limits<T>::infinity();
	T lb(0);
	T ub(std::numeric_limits<T>::infinity());
	if (!is_zero(x.lower())) {
		bits::round_set<T>(FE_DOWNWARD);
		lb = sqrt(x.lower());
	}
	if (!isINF(x.upper())) {
		bits::round_set<T>(FE_UPWARD);
		ub = sqrt(x.upper());
	}
	return interval<T>(lb, ub);
}

// build out floating point, etc. powers when needed
// this is a "final evaluate", handle algebraic reduction elsewhere
template<class T> interval<T> pow(const interval<T>& x, int e) {
	if (isNaN(x)) return x;
	if (0 == e) {
		if (is_zero(x)) throw numeric_error("0^0 undefined without further guidance");
		return 1.0;	// but if we have some sort of spread then we likely want the limit behavior which is 1
	}
	if (1 == e) return x;
	if (-1 == e) return 1.0 / x;
	if (0 == e % 2) return pow(square(x), e / 2);	// this catches 2's complement MIN_INT
	if (0 > e) return pow(1.0 / x, -e);
	return x * pow(square(x), e / 2);
}

} // namespace math

} // namespace zaimoni

#define ISK_INTERVAL zaimoni::math::interval
#include "_interval.hpp"

template<class T>
zaimoni::math::interval<T> intersect(const zaimoni::math::interval<T>& x, const zaimoni::math::interval<T>& y)
{
	if (zaimoni::isNaN(x)) {
		if (zaimoni::isNaN(y)) throw zaimoni::math::numeric_error("interval::intersect cannot recover from double-NaN");
		return y;
	}
	else if (zaimoni::isNaN(y)) return x;
	if (x.lower() > y.upper() || y.lower() > x.upper()) return zaimoni::math::interval<T>::empty();
	return zaimoni::math::interval<T>(x.lower() < y.lower() ? y.lower() : x.lower(), x.upper() < y.upper() ? x.upper() : y.upper());
}

namespace zaimoni {
// cmath and type_traits extensions go here so they are seen before they are used.  They exist in namespace zaimoni rather than namespace zaimoni::math
// signBit is not here because it's hard to get right

namespace math {

// fe rounding mode adjustments are not thread-safe; will need a static lock for that
namespace bits {

	template<class T>
	void op_add_assign(T& lhs, const T& rhs, int mode) {
#ifdef ZAIMONI_USING_STACKTRACE
		zaimoni::ref_stack<zaimoni::stacktrace, const char*> log(zaimoni::stacktrace::get(), __PRETTY_FUNCTION__);
#endif
		if (trivial<T>::sum(lhs, rhs)) return;
		if (would_overflow<T>::sum(lhs, rhs)) throw numeric_error("+: overflow");
		fesetround(mode);
		lhs += rhs;
	}

	template<class T>
	typename std::enable_if<op_works<T>::negate, void>::type
	op_diff_assign(T& lhs, const T& rhs, int mode) {
		return op_add_assign(lhs, -rhs, mode);	// XXX \todo strictly speaking, wrong for signed integers as exact min will fail negation
	}
}

template<class T>
interval<T> interval<T>::median() const	// from Boost
{
	const auto l_norm = norm(_lb);
	const auto u_norm = norm(_ub);
	bool l_smaller = l_norm < u_norm;
	interval<T> midpoint(l_smaller ? _ub : _lb);
	midpoint -= interval<T>(l_smaller ? _lb : _ub);
	interval<T> ret(l_smaller ? _lb : _ub);
	return ret += scalBn(midpoint, -1);
}

template<class T>
T interval<T>::width() const	// from Boost
{
	bits::round_set<T>(FE_UPWARD);
	return _ub - _lb;
}

template<class T>
interval<T>& interval<T>::operator+=(const interval<T>& rhs)
{
#ifdef ZAIMONI_USING_STACKTRACE
	zaimoni::ref_stack<zaimoni::stacktrace, const char*> log(zaimoni::stacktrace::get(), __PRETTY_FUNCTION__);
#endif
	bits::op_add_assign(_lb, rhs._lb, FE_DOWNWARD);
	bits::op_add_assign(_ub, rhs._ub, FE_UPWARD);
	return *this;
}

template<class T>
interval<T>& interval<T>::operator-=(const interval<T>& rhs)
{
	bits::op_diff_assign(_lb, rhs._ub, FE_DOWNWARD);
	bits::op_diff_assign(_ub, rhs._lb, FE_UPWARD);
	return *this;
}

template<class T>
interval<T>& interval<T>::operator*=(const interval<T>& rhs)
{
//	const int baseline = bits::round_get<T>();
	// zero and infinity matter here
	const int sign_code = 3 * sgn(*this) + sgn(rhs);
	// we need overflow-detecting wrappers, etc.
	switch (sign_code)
	{
	case 4:	// +*+ is +
		bits::round_set<T>(FE_DOWNWARD);
		_lb *= rhs._lb;
		bits::round_set<T>(FE_UPWARD);
		_ub *= rhs._ub;
		return *this;
	case -4:	// -*- is +
		bits::round_set<T>(FE_DOWNWARD);
		{
		const T tmp_lb(_ub*rhs._ub);
		bits::round_set<T>(FE_UPWARD);
		_ub = _lb * rhs._lb;
		_lb = tmp_lb;
		}
		return *this;
	case 2:	// +*- is -
		bits::round_set<T>(FE_DOWNWARD);
		{
		const T tmp_lb(_ub*rhs._lb);
		bits::round_set<T>(FE_UPWARD);
		_ub = _lb * rhs._ub;
		_lb = tmp_lb;
		}
		return *this;
	case -2:	// -*+ is -
		bits::round_set<T>(FE_DOWNWARD);
		_lb *= rhs._ub;
		bits::round_set<T>(FE_UPWARD);
		_ub *= rhs._lb;
		return *this;
	case 3:		// + * contains 0
		if (isINF(lower())) throw numeric_error("*: NaN");
		if (is_zero(rhs)) return (*this = T(0));	// XXX \todo not quite right as sign of zero would matter for floating-point
		bits::round_set<T>(FE_DOWNWARD);
		_lb = _ub * rhs._lb;
		bits::round_set<T>(FE_UPWARD);
		_ub *= rhs._ub;
		return *this;
	case -3:	// - * contains 0
		if (isINF(upper())) throw numeric_error("*: NaN");
		if (is_zero(rhs)) return (*this = T(0));	// XXX \todo not quite right as sign of zero would matter for floating-point
		bits::round_set<T>(FE_UPWARD);
		_ub = rhs._lb*_lb;
		bits::round_set<T>(FE_DOWNWARD);
		_lb *= rhs._ub;
		return *this;
	case 1:		// contains 0 * +
		if (isINF(rhs.lower())) throw numeric_error("*: NaN");
		if (is_zero(*this)) return *this;
		bits::round_set<T>(FE_DOWNWARD);
		_lb *= rhs._ub;
		bits::round_set<T>(FE_UPWARD);
		_ub *= rhs._ub;
		return *this;
	case -1:	// contains 0 * -
		if (isINF(rhs.upper())) throw numeric_error("*: NaN");
		if (is_zero(*this)) return *this;	// XXX \todo not quite right as sign of zero would matter for floating-point
		bits::round_set<T>(FE_DOWNWARD);
		{
		const T tmp_lb(_ub*rhs._lb);
		bits::round_set<T>(FE_UPWARD);
		assign(tmp_lb, _lb*rhs._lb);
		}
		return *this;
	case 0:		// contains 0 * contains 0
		if (is_zero(*this)) return *this;	// XXX \todo not quite right as sign of zero would matter for floating-point
		if (is_zero(rhs)) return (*this = T(0));	// XXX \todo not quite right as sign of zero would matter for floating-point
		// unfortunately, we have to account for topological rays containing zero: these can generate naive 0*infinity which is disallowed
		if (isINF(_ub)) {
			if (isINF(_lb)) return *this;	// entire real number line
			if (is_negative(rhs._lb) && is_positive(rhs._ub)) {
				_lb = -_ub;
				return *this;	// again, entire real number line.  This is not a throw since we are in R rather than R#: a multiply by exactly zero will restore precision.
			}
			if (isINF(rhs._ub)) {
				if (isINF(rhs._lb)) return (*this = rhs);	// entire real number line
				if (is_negative(_lb)) {
					_lb = -_ub;
					return *this;	// again, entire real number line.  This is not a throw since we are in R rather than R#: a multiply by exactly zero will restore precision.
				}
				return *this;	// [0,infinity)*[0,infinity) is ok
			}
			// [..., infinity]*[0,finite] or [finite,0]
			if (!is_negative(_lb)) {	// non-negative lower bound just gets mapped to 0, infinity
				if (is_positive(rhs._ub)) {
					_lb = 0;
					return *this;
				} else {
					assign(-_ub, 0);
					return *this;
				}
			} else if (is_positive(rhs._ub)) {
				bits::round_set<T>(FE_DOWNWARD);
				_lb *= rhs._ub;
				return *this;
			} else {
				bits::round_set<T>(FE_UPWARD);
				assign(-_ub, _lb*rhs._lb);
				return *this;
			}
		} else if (isINF(rhs._ub)) {
			if (isINF(rhs._lb)) return (*this = rhs);	// entire real number line
			if (is_negative(_lb) && is_positive(_ub)) {
				assign(-rhs._ub, rhs._ub);
				return *this;	// again, entire real number line.  This is not a throw since we are in R rather than R#: a multiply by exactly zero will restore precision.
			}

			// [..., infinity]*[0,finite] or [finite,0]
			if (!is_negative(rhs._lb)) {	// non-negative lower bound just gets mapped to 0, infinity
				if (is_positive(_ub)) {
					assign(0, rhs._ub);
					return *this;
				} else {
					assign(-rhs._ub, 0);
					return *this;
				}
			} else if (is_positive(_ub)) {
				bits::round_set<T>(FE_DOWNWARD);
				assign(rhs._lb*_ub, rhs._ub);
				return *this;
			} else {
				bits::round_set<T>(FE_UPWARD);
				assign(-rhs._ub, rhs._lb*_lb);
				return *this;
			}
		} else if (isINF(_lb)) {
			if (is_negative(rhs._lb) && is_positive(rhs._ub)) {
				_ub = -_lb;
				return *this;	// again, entire real number line.  This is not a throw since we are in R rather than R#: a multiply by exactly zero will restore precision.
			}
			if (isINF(rhs._lb)) {
				if (is_positive(_ub)) {
					_ub = -_lb;
					return *this;	// again, entire real number line.  This is not a throw since we are in R rather than R#: a multiply by exactly zero will restore precision.
				}
				return *this;	// [0,infinity)*[0,infinity) is ok
			}
			// [-infinity,...]*[0,finite] or [finite,0]
			if (!is_positive(_ub)) {	// non-negative lower bound just gets mapped to 0, infinity
				if (is_negative(rhs._lb)) {
					_ub = 0;
					return *this;
				} else {
					assign(0, -_lb);
					return *this;
				}
			} else if (is_negative(rhs._lb)) {
				bits::round_set<T>(FE_UPWARD);
				_ub *= rhs._lb;
				return *this;
			} else {
				bits::round_set<T>(FE_DOWNWARD);
				assign(_ub*rhs._ub, -_lb);
				return *this;
			}
		} else if (isINF(rhs._lb)) {
			if (is_negative(_lb) && is_positive(_ub)) {
				assign(rhs._lb, -rhs._lb);
				return *this;	// again, entire real number line.  This is not a throw since we are in R rather than R#: a multiply by exactly zero will restore precision.
			}
			// [0,finite] or [finite,0]*[-infinity,...]
			if (!is_positive(rhs._ub)) {	// non-negative lower bound just gets mapped to 0, infinity
				if (is_negative(_lb)) {
					assign(rhs._lb, 0);
					return *this;
				} else {
					assign(0, -rhs._lb);
					return *this;
				}
			} else if (zaimoni::is_negative(_lb)) {
				bits::round_set<T>(FE_UPWARD);
				assign(rhs._lb, rhs._ub*_lb);
				return *this;
			} else {
				bits::round_set<T>(FE_DOWNWARD);
				assign(rhs._ub*_ub, -rhs._lb);
				return *this;
			}
		}
		// everything finite now
		bits::round_set<T>(FE_DOWNWARD);
		const T lb_1(_lb*rhs._ub);
		const T lb_2(_ub*rhs._lb);
		bits::round_set<T>(FE_UPWARD);
		const T ub_1(_lb*rhs._lb);
		const T ub_2(_ub*rhs._ub);
		assign((lb_1 < lb_2 ? lb_1 : lb_2), (ub_1 < ub_2 ? ub_1 : ub_2));
		return *this;
	}
	_fatal_code("*= : unhandled", 3);
}

template<class T>
interval<T>& interval<T>::operator/=(const interval<T>& rhs)
{
	//	const int baseline = bits::round_get<T>();
	// zero and infinity matter here
	const int sign_code = 3 * sgn(*this) + sgn(rhs);
	// we need overflow-detecting wrappers, etc.
	switch (sign_code)
	{
	case -3:
	case 0:
	case 3:	// division by zero risk.
		if (!is_positive(rhs._lb) && !is_negative(rhs._ub)) throw numeric_error("/: division by zero");	// a zero-endpoint is handled by mapping to infinity.
		if (!std::numeric_limits<T>::has_infinity) _fatal_code("/= : type needs infinity but doesn't have it", 3);
	}
	switch (sign_code)
	{
	case 4:	// +/+ is +
		bits::round_set<T>(FE_DOWNWARD);
		_lb /= rhs._ub;
		bits::round_set<T>(FE_UPWARD);
		_ub /= rhs._lb;
		return *this;
	case -4: // -/- is +
		bits::round_set<T>(FE_DOWNWARD);
		{
		const T tmp_lb(_ub / rhs._lb);
		bits::round_set<T>(FE_UPWARD);
		assign(tmp_lb, _lb / rhs._ub);
		}
		return *this;
	case 2: // +/- is -
		bits::round_set<T>(FE_DOWNWARD);
		{
		T tmp_lb(_ub / rhs._ub);
		bits::round_set<T>(FE_UPWARD);
		assign(tmp_lb, _lb / rhs._lb);
		}
		return *this;
	case -2: // -/+ is -
		bits::round_set<T>(FE_DOWNWARD);
		_lb /= rhs._lb;
		bits::round_set<T>(FE_UPWARD);
		_ub /= rhs._ub;
		return *this;
	case 1:	// ?? / + conserves endpoint sign
		bits::round_set<T>(FE_DOWNWARD);
		_lb /= rhs._lb;
		bits::round_set<T>(FE_UPWARD);
		_ub /= rhs._lb;
		return *this;
	case -1:	// ?? / - flips endpoint sign
		bits::round_set<T>(FE_DOWNWARD);
		{
		T tmp_lb(_ub*rhs._ub);
		bits::round_set<T>(FE_UPWARD);
		assign(tmp_lb, _lb*rhs._ub);
		}
		return *this;
	}
	// sign_code is one of -3, 0, or 3 at this point; rhs contains zero but survived the divide-by-zero trap
	const int rhs_zero_code = is_zero(rhs._ub) - is_zero(rhs._lb);	// -1 for zero lower bound, 1 for zero upper bound; otherwise ideally would hard-error
	switch (sign_code + rhs_zero_code)
	{
	case 4:	// +/[0,...]
		bits::round_set<T>(FE_DOWNWARD);
		_lb /= rhs._ub;
		_ub = std::numeric_limits<T>::infinity();
		return *this;
	case 2:	// +/[...,0]
		bits::round_set<T>(FE_UPWARD);
		assign(-std::numeric_limits<T>::infinity(), _lb/rhs._lb);
		return *this;
	case -2:	// -/[0,...]
		_lb = -std::numeric_limits<T>::infinity();
		bits::round_set<T>(FE_UPWARD);
		_ub /= rhs._ub;
		return *this;
	case -4:	// -/[...,0]
		bits::round_set<T>(FE_DOWNWARD);
		assign(_ub / rhs._lb, std::numeric_limits<T>::infinity());
		return *this;
	}
	if (is_zero(*this)) return *this;	// not quite correct (doesn't handle negative zero); could be much earlier except this is a special case
	const int lhs_zero_code = 3*(is_zero(_ub) - is_zero(_lb));	// -3 for zero lower bound, 3 for zero upper bound; 0 is "contains zero"
	switch (lhs_zero_code + rhs_zero_code)
	{
	case -1:
	case 1:
		return (*this = _whole);	// have lost all precision; good candidate for a policy
	case 4:		// [0,...]/[0,...]
	case -4:	// [...,0]/[...,0]
		assign(0, std::numeric_limits<T>::infinity());
		return *this;
	case 2:		// [0,...]/[...,0]
	case -2:	// [...,0]/[0,....]
		assign(-std::numeric_limits<T>::infinity(),0);
		return *this;
	}
	_fatal_code("/= : unhandled", 3);
}

}	// namespace math
}	// namespace zaimoni


#endif