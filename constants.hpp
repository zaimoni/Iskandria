// constants.hpp
// fundamental constants of physics

#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <boost/numeric/interval.hpp>

// SI values are to be imported from CODATA
class fundamental_constants {
public:
	// tracking representative units
	boost::numeric::interval<double> distance_unit;	// in meters
	boost::numeric::interval<double> time_unit;	// in seconds
	boost::numeric::interval<double> mass_unit;	// in kilograms
	boost::numeric::interval<double> temperature_unit;	// in kelvin

	// these four are the geometrizable constants: set all four to 1 to uniquely solve the above units.
	// These three are from Misner/Thorne/Wheeler
	boost::numeric::interval<double> c;	// speed of light in vacuum
	boost::numeric::interval<double> G;	// Newtonian G
	boost::numeric::interval<double> k;	// Boltzmann constant
	// No source for this; equates General Relatvity momentum and Quantum Mechanics momentum.
	boost::numeric::interval<double> h_bar;	// Planck constant/2pi

	fundamental_constants();	// default-constructs to SI units.

	void mult_scale_distance(boost::numeric::interval<double> x);
	void div_scale_distance(boost::numeric::interval<double> x);
	void mult_scale_time(boost::numeric::interval<double> x);
	void div_scale_time(boost::numeric::interval<double> x);
	void mult_scale_mass(boost::numeric::interval<double> x);
	void div_scale_mass(boost::numeric::interval<double> x);
	void mult_scale_temperature(boost::numeric::interval<double> x);
	void div_scale_temperature(boost::numeric::interval<double> x);
	void geometrize();
};

const fundamental_constants& SI_units();	// i.e. MKS
const fundamental_constants& CGS_units();
const fundamental_constants& geometrized_units();
const fundamental_constants& solar_system_units();

// deal with English units later; there are too many choices of length unit to make an informed decision.

#endif
