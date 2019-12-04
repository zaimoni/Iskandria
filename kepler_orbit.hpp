#ifndef KEPLER_ORBIT_HPP
#define KEPLER_ORBIT_HPP

#include "Zaimoni.STL/Logging.h"
#include "mass.hpp"
#include "conic.hpp"
#include "coord_chart.hpp"

namespace kepler {

// implied coordinate system is perifocal: https://en.wikipedia.org/wiki/Perifocal_coordinate_system
// for the Newtonian two-body problem, this is the same as barycentric coordinates
// also cf https://en.wikipedia.org/wiki/Apsis and https://en.wikipedia.org/wiki/Kepler%27s_laws_of_planetary_motion

// the periapsis (perihelion, etc.) is on the positive x-axis
// y-axis unit vector is at theta=90 degrees in polar coordinates
// when embedding in 3-space, the unit vector parallel to the angular momentum vector is the cross-product of the above two vectors.

// angle types
enum {
	Mean_Anomaly = 0,	// angle from center of circle whose orbital period is the orbit's orbital period
	Eccentric_Anomaly,	// angle from geometric center of elliptical orbit
	True_Anomaly		// angle from focus of orbit
};

class orbit
{
public:
	typedef fundamental_constants::interval interval;
	typedef zaimoni::math::Cartesian_vector<interval, 2>::coord_type vector;
	typedef zaimoni::math::conic conic;
	typedef zaimoni::circle::Angle<Mean_Anomaly> mean_anomaly;
	typedef zaimoni::circle::Angle<Eccentric_Anomaly> eccentric_anomaly;
	typedef zaimoni::circle::Angle<True_Anomaly> true_anomaly;
private:
	mass _m;	// usually specified as the reduced gravitational parameter.  Provides the unit system in use.
	conic _orbit;	// the actual orbit
	// while we can calculate the orbit from the perihelion/aphelion pair and vice versa,
	// we do want to record both as the construction parameters actually used are considered more accurate
	interval _pericenter;	// barycentric perihelion, etc.
	interval _apocenter;	/// barycentric aphelion, etc.
	// following cache variables do not actually need to reach the savefile
	mutable bool have_m_div_a = false;
	mutable interval _m_div_a;
	mutable bool have_one_minus_e_div_one_plus_e = false;
	mutable interval _one_minus_e_div_one_plus_e;
	mutable bool have_m_div_specific_angular_momentum = false;
	mutable interval _m_div_specific_angular_momentum;
	mutable bool have_mean_anomaly_scale = false;
	mutable interval _mean_anomaly_scale;
public:
	orbit() = default;
	orbit(const orbit& src) = default;
	orbit(const mass& m, const conic& o) : _m(m), _orbit(o), _pericenter((1.0-o.e())*o.a()), _apocenter((1.0 + o.e())* o.a()) {};
	orbit(const mass& m, const interval& barycentric_perihelion, const interval& barycentric_aphelion) : _m(m), _orbit(_from_perihelion_aphelion(barycentric_perihelion, barycentric_aphelion)), _pericenter(barycentric_perihelion), _apocenter(barycentric_aphelion) {};

	~orbit() = default;
	orbit& operator=(const orbit& src) = default;

	friend bool operator==(const orbit& lhs, const orbit& rhs) {
		return lhs._m == rhs._m && lhs._orbit == rhs._orbit && zaimoni::definitely<typename std::remove_cv<decltype(lhs._pericenter)>::type>::equal(lhs._pericenter,rhs._pericenter) && zaimoni::definitely<typename std::remove_cv<decltype(lhs._apocenter)>::type>::equal(lhs._apocenter,rhs._apocenter);
	}
	friend bool operator!=(const orbit& lhs, const orbit& rhs) { return !(lhs == rhs); }

	// accessors
	const mass& m() const { return _m; }
	const conic& o() const { return _orbit; }
	const interval& pericenter() const { return _pericenter; }
	const interval& apocenter() const { return _apocenter; }

	const interval& m_div_a() const;
	const interval& one_minus_e_div_one_plus_e() const;
	const interval& m_div_specific_angular_momentum() const;
	const decltype(_mean_anomaly_scale)& mean_anomaly_scale() const;

	interval v_pericenter() const { return sqrt(m_div_a() / one_minus_e_div_one_plus_e()); }
	interval v_apocenter() const { return sqrt(m_div_a() * one_minus_e_div_one_plus_e()); }
	interval specific_orbital_energy() const { return -0.5*m_div_a(); }
	interval specific_relative_angular_momentum() const { return sqrt((1.0 - square(_orbit.e()) * _m.GM() * _orbit.a())); }	// again, wants support from conic class
	interval geometric_mean_of_v_pericenter_v_apocenter() const { return sqrt(m_div_a()); }
	interval predicted_e() const { return (_apocenter-_pericenter) / (_apocenter + _pericenter); }	// some data normalization at construction time indicated

//	vector r(const angle& true_anomaly) {}
	vector v(const true_anomaly& theta) const;
	interval d_polar_r_dt(const true_anomaly& theta) const;
	interval polar_r(const eccentric_anomaly& E) const;
/*
	// we have: 0 degrees true anomaly = 0 degrees eccentric anomaly = 0 degrees mean anomaly
	// and 180 degrees true anomaly = 180 degrees eccentric anomaly = 180 degrees mean anomaly
	true_anomaly theta(const eccentric_anomaly& E) const {	// i.e. true anomaly
		// numerically solve: (1-_orbit.e())tan^2(theta) = (1+_orbit.e()) tan^2(eccentric_anomaly)
		// obviously need an alternate expression for near 90 degrees/270 degrees
		// fully multiplicative form: (1-_orbit.e())sin^2(theta)cos^2(eccentric_anomaly) = (1+_orbit.e()) sin^2(eccentric_anomaly)cos^2(theta)
		// i.e. we have unfixable direct solution for 90 degrees theta, eccentric anomaly -- but as a right triangle with one edge the focal
		// distance c we have other means there
	}
*/
	eccentric_anomaly E(const mean_anomaly& M);
	interval period_squared() const { return square(2.0 * interval_shim::pi) * pow(_orbit.a(), 3) / _m.GM(); }	// dimension time^2
//	mean_anomaly M(const zaimoni::circle::angle& t) { return mean_anomaly(mean_anomaly_scale() * t); }	// replace this

	// circular orbits (parabolic orbit values are twice this)
	interval circular_velocity_squared_from_radius(const interval& r) { return _m.GM() / r; }
	interval circular_radius_from_velocity_squared(const interval& v2) { return _m.GM() / v2; }
	// angular velocity omega in radians is v/r

private:
	static conic _from_perihelion_aphelion(const interval& barycentric_perihelion, const interval& barycentric_aphelion);
};

}	// namespace kepler

#endif
