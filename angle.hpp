// angle.hpp

#ifndef ANGLE_HPP
#define ANGLE_HPP 1

#include "interval_shim.hpp"
#include "Zaimoni.STL/Compiler.h"
#include <vector>

namespace zaimoni {
namespace circle {

// Object here is to make the periodicity of trigonometric functions exactly represented
// 360 degrees is 360*60*60 seconds: 2^7 is a factor
// so representing 360 degrees as 10125 is fine; scaling is *225/8

// XXX decision *not* to template feels weirder now than in 2006ish.

class angle {
public:
	typedef interval_shim::interval interval;
	typedef zaimoni::math::Interval<0, typename interval::base_type> radian;
	typedef zaimoni::math::Interval<1, typename interval::base_type> degree;
	static constexpr const interval sin_cos_range_for_real_domain = interval(-1, 1);

private:
	interval _theta;

	constexpr explicit angle(typename interval::base_type x) : _theta(x) {}
	constexpr explicit angle(typename interval::base_type lb, typename interval::base_type ub) : _theta(lb, ub) {}

public:
	angle() = default;
	explicit angle(const degree& src) : _theta(src) { _degree_to_standard_form(); };
	explicit angle(const radian& src) : _theta(src) { _radian_to_standard_form(); };
	angle(const angle& lb, const angle& ub);
	ZAIMONI_DEFAULT_COPY_DESTROY_ASSIGN(angle);

	bool is_whole_circle() const {return 10125<=_theta.upper()-_theta.lower();};
	degree deg() const {return degree((_theta*8.0)/225.0);}
//	XOPEN standard provides M_PI, not ISO C
//	interval radians() const {return (((_theta*8)/225)/180)*M_PI;}
	radian rad() const {return radian(((_theta*2.0)*interval_shim::pi)/1125.0);}

	angle lower() const { return angle(_theta.lower()); }
	angle upper() const { return angle(_theta.upper()); }
	bool is_exact() const { return _theta.lower() == _theta.upper(); }
	bool contains(const angle& x) const { return _theta.contains(x._theta); }
	friend void init(const angle& src, zaimoni::_fp_stats<double>& l_stat, zaimoni::_fp_stats<double>& u_stat);
	bool container_lt(const angle& rhs) const;
	bool lower_lt(const angle& rhs) const { return _theta.lower() < rhs._theta.lower(); }
	bool upper_lt(const angle& rhs) const { return _theta.upper() < rhs._theta.upper(); }

	friend bool operator==(const angle& lhs, const angle& rhs) { return lhs.is_exact() && rhs.is_exact() && lhs.lower() == rhs.lower(); }
	friend angle operator-(const angle& x) { return angle(-x.upper(), -x.lower()); }

	angle operator+=(const angle& src);
	angle operator-=(const angle& src);

	angle& operator*=(const interval& rhs);
	angle& operator*=(typename const_param<interval::base_type>::type rhs);
	void self_scalBn(int scale);	// works on "primary" value; true division would give a range out

	std::vector<angle> contains_ref_angles() const;

	// more FORTRAN approach to retrieving reference angles
	static constexpr size_t ref_angle_maxsize = 17;	// needs regression-testing
	size_t contains_ref_angles(angle* dest) const;

	void sincos(interval& _sin, interval& _cos) const;

private:
	void _standard_form();
	void _degree_to_standard_form();
	void _radian_to_standard_form();

	static void _apply_180_degrees_shift(interval x, interval& _sin, interval& _cos);
	static void _apply_sector(interval x, interval& _sin, interval& _cos);
	static void _apply_y_reflect(interval x, interval& _sin, interval& _cos);
	static void _apply_x_reflect(interval x, interval& _sin, interval& _cos);
	static void _sincos(interval x, interval& _sin, interval& _cos);
	static void _radian_sincos(interval radians, interval& _sin, interval& _cos);
	static void _internal_sincos(typename const_param<interval::base_type>::type x, interval& _sin, interval& _cos);
};

inline angle operator+(const angle& lhs, const angle& rhs) { return angle(lhs) += rhs; }
inline angle operator-(const angle& lhs, const angle& rhs) { return angle(lhs) -= rhs; }

inline angle operator*(const angle& lhs, const angle::interval& rhs) { return angle(lhs) *= rhs; }
inline angle operator*(const angle& lhs, typename const_param<angle::interval::base_type>::type rhs) { return angle(lhs) *= rhs; }
inline angle operator*(const angle::interval& lhs, const angle& rhs) { return angle(rhs) *= lhs; }	// assumes commutative multiplication (true when base type is in real numbers R)
inline angle operator*(typename const_param<angle::interval::base_type>::type lhs, const angle& rhs) { return angle(rhs) *= lhs; }

struct ref_angle {
	inline static const angle zero = angle(angle::degree(0));
	inline static const angle half_circle = angle(angle::degree(180));
	inline static const angle span_half_circle = angle(angle::degree(0, 180));
	inline static const angle span_neg_half_circle = angle(angle::degree(-180,0));
};

struct angle_compare {
	bool operator()(const angle& lhs, const angle& rhs) const { return lhs.container_lt(rhs); };
};

// some uses need to track "what kind of angle".
template<int code>
class Angle : public angle {
public:
	Angle() = default;
	explicit Angle(const degree& theta) : angle(theta) {};
	explicit Angle(const radian& theta) : angle(theta) {};
	explicit Angle(angle theta) : angle(theta) {};
	ZAIMONI_DEFAULT_COPY_DESTROY_ASSIGN(Angle);

	Angle lower() const { return Angle(angle::lower()); }
	Angle upper() const { return Angle(angle::upper()); }

	friend Angle operator-(const Angle& x) { return Angle(-x); }

	Angle& operator*=(const interval& rhs) { static_cast<angle>(*this) *= rhs; return *this; };
	Angle& operator*=(typename const_param<interval::base_type>::type rhs) { static_cast<angle>(*this) *= rhs; return *this; };
};

}	// namespace circle
}	// namespace zaimoni

#ifdef VAR_HPP
#include "Zaimoni.STL/bits/_angle_var.hpp"
#endif

#endif
