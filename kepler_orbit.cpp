#include "kepler_orbit.hpp"

namespace kepler {

const orbit::interval& orbit::m_div_a() const {
	if (have_m_div_a) return _m_div_a;
	have_m_div_a = true;
	return (_m_div_a = _m.GM() / _orbit.a());
}

const orbit::interval& orbit::one_minus_e_div_one_plus_e() const {
	if (have_one_minus_e_div_one_plus_e) return _one_minus_e_div_one_plus_e;
	have_one_minus_e_div_one_plus_e = true;
	return (_one_minus_e_div_one_plus_e = (1.0 - _orbit.e()) / (1.0 + _orbit.e()));	// numerator wants support from conic class
}

const orbit::interval& orbit::m_div_specific_angular_momentum() const {
	if (have_m_div_specific_angular_momentum) return _m_div_specific_angular_momentum;
	have_m_div_specific_angular_momentum = true;
	return (_m_div_specific_angular_momentum = _m.GM() / specific_relative_angular_momentum());
}

const decltype(orbit::_mean_anomaly_scale)& orbit::mean_anomaly_scale() const {
	if (have_mean_anomaly_scale) return _mean_anomaly_scale;
	have_mean_anomaly_scale = true;
	return (_mean_anomaly_scale = 360.0 / sqrt(period_squared()));
}

orbit::vector orbit::v(const orbit::true_anomaly& theta) const {
	interval _sin;
	interval _cos;
	theta.sincos(_sin, _cos);
	interval tmp = m_div_specific_angular_momentum();
	return zaimoni::make<vector>()(-_sin * tmp, tmp * (_orbit.e() + _cos));
}

orbit::interval orbit::d_polar_r_dt(const orbit::true_anomaly& theta) const {
	interval _sin;
	interval _cos;
	theta.sincos(_sin, _cos);
	return m_div_specific_angular_momentum() * _orbit.e() * _sin;
}

orbit::interval orbit::polar_r(const orbit::eccentric_anomaly& E) const {
	interval _sin;
	interval _cos;
	E.sincos(_sin, _cos);
	return _orbit.a() * (1.0 - _orbit.e() * _cos);
}

orbit::conic orbit::_from_perihelion_aphelion(const interval& barycentric_perihelion, const interval& barycentric_aphelion) {
	interval major = (barycentric_perihelion + barycentric_aphelion) / 2.0;
	interval minor = sqrt(barycentric_perihelion * barycentric_aphelion);
	return conic(zaimoni::math::conic_tags::ellipse(), major, minor);
}

}	// namespace kepler

#ifdef TEST_APP3
// fast compile test
// g++ -std=c++14 -otest.exe -Os  -D__STDC_LIMIT_MACROS -DTEST_APP2 conic.test.cpp constants.cpp -Llib\host.isk -lz_stdio_c -lz_log_adapter -lz_stdio_log -lz_format_util
#include "arithmetic.hpp"
#include "Zaimoni.STL/var.hpp"

#include "test_driver.h"

int main(int argc, char* argv[])
{
	typedef zaimoni::math::conic conic;

	// unverified mockups
	mass sun(mass::ASTRODYNAMIC, fundamental_constants::MKS, mass::interval(1.32712440009e20,1.32712440027e20));
	mass jupiter(mass::ASTRODYNAMIC, fundamental_constants::MKS, mass::interval(1.26686525e17, 1.26686543e17));
	mass saturn(mass::ASTRODYNAMIC, fundamental_constants::MKS, mass::interval(3.7931178e16, 3.7931196e16));

	// \todo reduced mass of the above three (yes, testing infrastructure for a wargame scenario)
	std::shared_ptr<zaimoni::var<double> > one(new zaimoni::var<double>(1));		// will need this later on; others are code coverage
	std::shared_ptr<zaimoni::var<float> > one_f(new zaimoni::var<float>(1));
	std::shared_ptr<zaimoni::var<long double> > one_l(new zaimoni::var<long double>(1));

	std::shared_ptr<zaimoni::var<ISK_INTERVAL<double> > > one_i(new zaimoni::var<ISK_INTERVAL<double> >(1));
	std::shared_ptr<zaimoni::var<ISK_INTERVAL<float> > > one_if(new zaimoni::var< ISK_INTERVAL<float> >(1));
	std::shared_ptr<zaimoni::var<ISK_INTERVAL<long double> > > one_il(new zaimoni::var< ISK_INTERVAL<long double> >(1));

	// inline prototype of reduced mass calculation as follows:
	auto inv_reduced_mass = new zaimoni::sum<typename zaimoni::_type_of<double>::type>();
	inv_reduced_mass->append_term(new zaimoni::quotient<typename zaimoni::_type_of<double>::type>(one, new zaimoni::var<mass::interval>(sun.GM())));
	inv_reduced_mass->append_term(new zaimoni::quotient<typename zaimoni::_type_of<double>::type>(one, new zaimoni::var<mass::interval>(jupiter.GM())));
	inv_reduced_mass->append_term(new zaimoni::quotient<typename zaimoni::_type_of<double>::type>(one, new zaimoni::var<mass::interval>(saturn.GM())));
	STRING_LITERAL_TO_STDOUT("example inverse reduced mass (GM)\n");
	INFORM(inv_reduced_mass->to_s().c_str());

	zaimoni::product<typename zaimoni::_type_of<double>::type> code_coverage;
	zaimoni::quotient<typename zaimoni::_type_of<double>::type> code_coverage2;
	zaimoni::quotient<typename zaimoni::_type_of<double>::type> reduced_mass(one,inv_reduced_mass);

	// reduced mass of n bodies is the harmonic mean of their masses, divided by n .. i.e. the n multipler on top is dropped
	STRING_LITERAL_TO_STDOUT("example reduced mass (GM)\n");
	INFORM(reduced_mass.to_s().c_str());
	while(reduced_mass.self_eval()) INFORM(reduced_mass.to_s().c_str());

	// \todo units conversion...put astronomical unit AU somewhere, then use it below
	conic unit_circle(1);
	STRING_LITERAL_TO_STDOUT("unit circle\n");

	conic unit_parabola(conic::parabola(), 1, conic::semi_major_axis());
	STRING_LITERAL_TO_STDOUT("unit parabola\n");
	conic unit_parabola2(conic::parabola(), 1, conic::semi_latus_rectum());
	STRING_LITERAL_TO_STDOUT("unit parabola #2\n");
	conic unit_parabola3(conic::parabola(), 1, conic::focal_parameter());
	STRING_LITERAL_TO_STDOUT("unit parabola #3\n");

	conic unit_circle2(conic::ellipse(), 1, 1);
	STRING_LITERAL_TO_STDOUT("unit circle #2\n");
	conic elllipse_21_1(conic::ellipse(), 2, 1);
	STRING_LITERAL_TO_STDOUT("2:1 ellipse\n");
	conic elllipse_21_2(conic::ellipse(), 1, 2);
	STRING_LITERAL_TO_STDOUT("2:1 ellipse #2\n");

	conic hyperbola_21(conic::hyperbola(), 2, 1);
	STRING_LITERAL_TO_STDOUT("2:1 hyperbola\n");
	conic hyperbola_12(conic::hyperbola(), 1, 2);
	STRING_LITERAL_TO_STDOUT("1:2 hyperbola\n");

	return 0;
}
#endif

