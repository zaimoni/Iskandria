// constants.hpp
// fundamental constants of physics

#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

// #define CONSTANTS_ISK_INTERVAL 1

#ifdef CONSTANTS_ISK_INTERVAL 
#include "interval.hpp"
#else
#include <boost/numeric/interval.hpp>
#endif

// SI values are to be imported from CODATA
class fundamental_constants {
public:
#ifdef CONSTANTS_ISK_INTERVAL 
	typedef zaimoni::math::interval<double> interval;
#else
	typedef boost::numeric::interval<double> interval;
#endif

	// conversion factors
	static const interval N_A;	// Avogadro constant

	// dimensionless constants
	static const interval inv_alpha;	// inverse fine structure constant
	static const interval alpha;	// fine structure constant

	// tracking representative units
	interval distance_unit;	// in meters
	interval time_unit;	// in seconds
	interval mass_unit;	// in kilograms
	interval temperature_unit;	// in kelvin

	// these four are the geometrizable constants: set all four to 1 to uniquely solve the above units.
	// These three are from Misner/Thorne/Wheeler
	interval c;	// speed of light in vacuum
	interval G;	// Newtonian G
	interval k;	// Boltzmann constant
	// No source for this; equates General Relatvity momentum and Quantum Mechanics momentum.
	interval h_bar;	// Planck constant/2pi

	// following are from atomic units
//	interval m_e;	// electron rest mass
//	interval Q_e;	// electron charge
//	interval k_e;	// colouub force constant

//	vacuum permeability := (2*fine_structure_constant/electron_charge^2) * (planck_constant/c) i.e. magnetic constant
//	we have by construction (Maxwell equations) vacuum permititivity*vacuum permeability = 1/c^2
//  so vacuum permittivity := electron_charge^2/(2 fine structure constant planck_constant c) i.e. electric constant

	fundamental_constants();	// default-constructs to SI units.

	void mult_scale_distance(interval x);
	void div_scale_distance(interval x);
	void mult_scale_time(interval x);
	void div_scale_time(interval x);
	void mult_scale_mass(interval x);
	void div_scale_mass(interval x);
	void mult_scale_temperature(interval x);
	void div_scale_temperature(interval x);
	void geometrize();
};

const fundamental_constants& SI_units();	// i.e. MKS
const fundamental_constants& CGS_units();
const fundamental_constants& geometrized_units();
const fundamental_constants& solar_system_units();

// deal with English units later; there are too many choices of length unit to make an informed decision.

#endif
