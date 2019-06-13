#include "kepler_orbit.hpp"

#ifdef TEST_APP3
// fast compile test
// g++ -std=c++14 -otest.exe -Os  -D__STDC_LIMIT_MACROS -DTEST_APP2 conic.test.cpp constants.cpp -Llib\host.isk -lz_stdio_c -lz_log_adapter -lz_stdio_log -lz_format_util
#include <memory>

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

	// reduced mass of n bodies is the harmonic mean of their masses, divided by n .. i.e. the n multipler on top is dropped

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

