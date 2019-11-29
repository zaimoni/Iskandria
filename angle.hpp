// angle.hpp

#ifndef ANGLE_HPP
#define ANGLE_HPP 1

#include "interval_shim.hpp"
#include "Zaimoni.STL/Compiler.h"

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
private:
	interval _theta;
public:
	angle() = default;
	explicit angle(const degree& src) : _theta(src) { _degree_to_standard_form(); };
	explicit angle(const radian& src) : _theta(src) { _radian_to_standard_form(); };
	ZAIMONI_DEFAULT_COPY_DESTROY_ASSIGN(angle);

	bool is_whole_circle() const {return 10125<=_theta.upper()-_theta.lower();};
	interval degrees() const {return (_theta*8.0)/225.0;}
//	XOPEN standard provides M_PI, not ISO C
//	interval radians() const {return (((_theta*8)/225)/180)*M_PI;}
	interval radians() const {return ((_theta*2.0)*interval_shim::pi)/1125.0;}

	angle& operator*=(const interval& rhs);
	angle& operator*=(typename const_param<interval::base_type>::type rhs);

	void sincos(interval& _sin, interval& _cos) const;
private:
	void _standard_form();
	void _degree_to_standard_form();
	void _radian_to_standard_form();

	static void _sincos(interval x, interval& _sin, interval& _cos);
	static void _radian_sincos(interval radians, interval& _sin, interval& _cos);
};

inline angle operator*(const angle& lhs, const angle::interval& rhs) { return angle(lhs) *= rhs; }
inline angle operator*(const angle& lhs, typename const_param<angle::interval::base_type>::type rhs) { return angle(lhs) *= rhs; }
inline angle operator*(const angle::interval& lhs, const angle& rhs) { return angle(rhs) *= lhs; }	// assumes commutative multiplication (true when base type is in real numbers R)
inline angle operator*(typename const_param<angle::interval::base_type>::type lhs, const angle& rhs) { return angle(rhs) *= lhs; }

// some uses need to track "what kind of angle".
template<int code>
class Angle : public angle {
public:
	Angle() = default;
	explicit Angle(const degree& theta) : angle(theta) {};
	explicit Angle(const radian& theta) : angle(theta) {};
	explicit Angle(angle theta) : angle(theta) {};
	ZAIMONI_DEFAULT_COPY_DESTROY_ASSIGN(Angle);

	Angle& operator*=(const interval& rhs) { static_cast<angle>(*this) *= rhs; return *this; };
	Angle& operator*=(typename const_param<interval::base_type>::type rhs) { static_cast<angle>(*this) *= rhs; return *this; };
};

}	// namespace circle
}	// namespace zaimoni

#endif
