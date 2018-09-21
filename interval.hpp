#ifndef INTERVAL_HPP
#define INTERVAL_HPP 1

// interval.hpp
// interval arithmatic.  Generally follows Boost API design decisions, but specified to not have the critical bugs that should have blocked release (but didn't)

#include <stdexcept>
#include <fenv.h>
#include "Zaimoni.STL/augment.STL/cmath"
#include "Zaimoni.STL/Logging.h"

namespace zaimoni {

namespace math {

// subclassing so we can distinguish our runtime errors from others
class numeric_error : public std::runtime_error {
public:
	explicit numeric_error(std::string e) : std::runtime_error(e) {}
	explicit numeric_error(const char* const e) : std::runtime_error(e) {}
};

template<class T>
class trivial
{
public:
	// 0: no action
	// -1: keep LHS; 1: keep RHS
	// 3: cancelled (replace with 0)
	static int sum(typename const_param<T>::type lhs, typename const_param<T>::type rhs) {
		if (isINF(lhs)) {
			if (isINF(rhs) && signBit(lhs) != signBit(rhs)) throw numeric_error("+: NaN");
			return -1;
		} else if (isINF(rhs) || is_zero(lhs)) {
			return 1;
		} else if (is_zero(rhs)) return -1;
//		else if (are_add_inverses(lhs, rhs)) return 3;	// not defined yet
		return 0;
	}

	// 0: no action
	// -1: keep LHS; 1: keep RHS
	// -2: keep LHS,negated; 2: keep RHS, negated
	static int product(typename const_param<T>::type lhs, typename const_param<T>::type rhs) {
		const bool is_negative = (signbit(lhs)!=signbit(rhs));
		if (isinf(lhs)) {
			if (contains_zero(rhs)) throw numeric_error("*: NaN");
			return (signbit(lhs) == is_negative) ? -1 : -2;
		} else if (isinf(rhs)) {
			if (contains_zero(lhs)) throw numeric_error("*: NaN");
			return (signbit(rhs) == is_negative) ? 1 : 2;
		} else if (is_zero(lhs)) {
			return (!op_works<T>::has_negative_zero || signbit(lhs) == is_negative) ? -1 : -2;
		} else if (is_zero(rhs)) {
			return (!op_works<T>::has_negative_zero || signbit(rhs) == is_negative) ? 1 : 2;
		} else if (is_one(lhs)) {
			return 1;
		} else if (is_one(rhs)) {
			return -1;
		} else if (is_negative_one(lhs)) {
			return 2;
		} else if (is_negative_one(rhs)) {
			return -2;
		} // else if (are_mult_inverses(lhs, rhs)) return 3;	// requires <cmath> for floating point

		return 0;
	}
};

template<class T>
typename std::enable_if<op_works<T>::negate,int>::type
trivial_diff(typename const_param<T>::type lhs, typename const_param<T>::type rhs) {
	return trivial<T>::sum(lhs, -rhs);
}

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

	static const interval<T> _empty;
	static const interval<T> _whole;
public:
	constexpr interval() = default;
	constexpr interval(const interval& src) = default;
	constexpr explicit interval(const T& src) : _lb(src), _ub(src) {};	// this constructor could get expensive.  We can revert the explicit modifier later, but for now use this to expose API issues.
	constexpr interval(const T& l, const T& u) : _lb(l), _ub(u) {};
	template<class T1> constexpr interval(const interval<T1> &src) : _lb(src._lb), _ub(src._ub) {};

	interval& operator=(const interval& src) = default;
	interval& operator=(const T& src) { _lb = src; _ub = src; return *this; }
	template<class T1> interval& operator=(const interval<T1> &src) { _lb = src._lb; _ub = src._ub; return *this; }

	void assign(const T& l, const T& u) { _lb = l; _ub = u; }

	const T& lower() const { return _lb; }
	const T& upper() const { return _ub; }
	interval<T> median() const;	// from Boost
	T width() const;	// from Boost

	static interval empty() { return _empty; };	// intentionally value-copy
	static interval whole() { return _whole; };	// intentionally value-copy
	static interval hull(const T& x, const T& y);

	// these two should fail for unsigned integers
	void self_negate() { assign(-_ub, -_lb); };
	interval operator-() { return interval(-_ub,-_lb);  };

	// operator==(interval,interval) doesn't work as expected
	bool contains(typename const_param<T>::type s) { return _lb <= s && s <= _ub; }	// acts like R# rather than R in that infinity gets counted as contained

	// users that want to be denormalized (lowerbound > upper bound legal), such as a true angle class, should compensate appropriately before using operators * or / and restore their normal form after.
	interval& operator+= (const interval& rhs);
	interval& operator-= (const interval& rhs);
	interval& operator*= (const interval& rhs);
	interval& operator/= (const interval& rhs);

//	catch these once the interval forms are stable
//	interval& operator+= (T const &r);
//	interval& operator-= (T const &r);
//	interval& operator*= (T const &r);
//	interval& operator/= (T const &r);
};

// we don't want to mess with operator== for intervals because it's counterintuitive
template<class T>
bool operator==(const interval<T>& i, typename const_param<T>::type s)
{
	return i.lower() == s && i.upper() == s;
}

template<class T>
bool operator==(typename const_param<T>::type s, const interval<T>& i)
{
	return i.lower() == s && i.upper() == s;
}

template<class T>
bool operator!=(const interval<T>& i, typename const_param<T>::type s)
{
	return i.lower() != s || i.upper() != s;
}

template<class T>
bool operator!=(typename const_param<T>::type s, const interval<T>& i)
{
	return i.lower() != s || i.upper() != s;
}

// Boost library assumes policy involved for scalar inequality with respect to intervals

template<class T> interval<T>  const interval<T>::_empty(std::numeric_limits<T>::has_quiet_NaN ? -std::numeric_limits<T>::quiet_NaN() : T(1), std::numeric_limits<T>::has_quiet_NaN ? std::numeric_limits<T>::quiet_NaN() : T(0));
template<class T> interval<T>  const interval<T>::_whole(std::numeric_limits<T>::has_infinity ? -std::numeric_limits<T>::infinity() : std::numeric_limits<T>::min(), std::numeric_limits<T>::has_infinity ? std::numeric_limits<T>::infinity() : std::numeric_limits<T>::max());


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

} // namespace math
} // namespace zaimoni

// Logging.h support -- outside of namespace to match Logging.h

template<class T>
void INFORM(const zaimoni::math::interval<T>& x)
{
	if (x.lower() == x.upper()) {
		INFORM(x.upper());
		return;
	}
	INC_INFORM("[");
	INC_INFORM(x.lower());
	INC_INFORM(",");
	INC_INFORM(x.upper());
	INFORM("]");
}

template<class T>
void INC_INFORM(const zaimoni::math::interval<T>& x)
{
	if (x.lower() == x.upper()) {
		INC_INFORM(x.upper());
		return;
	}
	INC_INFORM("[");
	INC_INFORM(x.lower());
	INC_INFORM(",");
	INC_INFORM(x.upper());
	INC_INFORM("]");
}

namespace zaimoni {
// cmath and type_traits extensions go here so they are seen before they are used.  They exist in namespace zaimoni rather than namespace zaimoni::math
// signBit is not here because it's hard to get right

// type_traits
template<class T>
constexpr bool is_zero(const zaimoni::math::interval<T>& x)
{
	return is_zero(x.lower()) && is_zero(x.upper());
}

template<class T>
constexpr bool contains_zero(const zaimoni::math::interval<T>& x)
{
	return !is_positive(x.lower()) && !is_negative(x.upper());
}

template<class T>
constexpr bool is_positive(const zaimoni::math::interval<T>& x)
{
	return is_positive(x.lower());
}

template<class T>
constexpr bool is_negative(const zaimoni::math::interval<T>& x)
{
	return is_negative(x.upper());
}

template<class T>
constexpr auto norm(const zaimoni::math::interval<T>& x)
{ 
	const auto l_norm = norm(x.lower());
	const auto u_norm = norm(x.upper());
	return l_norm < u_norm ? u_norm : l_norm;
}

// cmath
template<class T>
constexpr bool isINF(const zaimoni::math::interval<T>& x)
{
	return isINF(x.lower()) && isINF(x.upper()) && signBit(x.lower())== signBit(x.upper());	// we do want numeric intervals of numeric intervals to be a compiler error
}

template<class T>
constexpr bool isFinite(const zaimoni::math::interval<T>& x)
{
	return isFinite(x.lower()) && isFinite(x.upper());
}

template<class T>
constexpr bool isNaN(const zaimoni::math::interval<T>& x)
{
	return isNaN(x.lower()) || isNaN(x.upper()) || x.lower()>x.upper();	// we count empty as NaN for arithmetic purposes.  It would be ok for set-theoretic purposes.
}

template<class T>
constexpr zaimoni::math::interval<T> scalBn(const zaimoni::math::interval<T>& x, int scale)
{
	return zaimoni::math::interval<T>(scalBn(x.lower(),scale),scalBn(x.upper(),scale));
}

namespace math {

// fe rounding mode adjustments are not thread-safe; will need a static lock for that
namespace bits {
	template<class T>
	typename std::enable_if<std::is_floating_point<T>::value, int>::type
	round_get() { return fegetround(); }

	template<class T>
	typename std::enable_if<std::is_integral<T>::value, int>::type
	round_get() {return FE_TONEAREST;}

	template<class T>
	typename std::enable_if<std::is_floating_point<T>::value, void>::type
	round_set(int mode) { fesetround(mode); }

	template<class T>
	typename std::enable_if<std::is_integral<T>::value, void>::type
	round_set(int mode) {}

	template<class T>
	void op_add_assign(T& lhs, const T& rhs, int mode) {
		switch (int code = trivial<T>::sum(lhs, rhs))
		{
		case 1:	lhs = rhs;	// intentional fall-through
		case -1: return;
		case 3:	// cancellation
			lhs = 0;
			return;
		};
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
				return this;
			} else {
				bits::round_set<T>(FE_UPWARD);
				assign(-_ub, _lb*rhs._lb);
				return this;
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
				return this;
			} else {
				bits::round_set<T>(FE_UPWARD);
				assign(-rhs._ub, rhs._lb*_lb);
				return this;
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
				return this;
			} else {
				bits::round_set<T>(FE_DOWNWARD);
				assign(_ub*rhs._ub, -_lb);
				return this;
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
			} else if (is_negative(_lb)) {
				bits::round_set<T>(FE_UPWARD);
				assign(rhs._lb, rhs._ub*_lb);
				return this;
			} else {
				bits::round_set<T>(FE_DOWNWARD);
				assign(rhs._ub*_ub, -rhs._lb);
				return this;
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
	case 1:	// ?? / - flips endpoint sign
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
	case 2:	// -/[0,...]
		_lb = -std::numeric_limits<T>::infinity();
		bits::round_set<T>(FE_UPWARD);
		_ub /= rhs._ub;
		return *this;
	case -4:	// -/[...,0]
		bits::round_set<T>(FE_DOWNWARD);
		assign(_ub / rhs._lb, std::numeric_limits<T>::infinity());
		return *this;
	}
	if (is_zero(this)) return *this;	// not quite correct (doesn't handle negative zero); could be much earlier except this is a special case
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