// constants.hpp
// fundamental constants of physics

#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include "interval_shim.hpp"
#include "dim_anal.hpp"

// This class does not directly reach the savefile.

// SI values are to be imported from CODATA
class fundamental_constants {
public:
	typedef interval_shim::interval interval;

	enum units : unsigned char {	// solar system would be reconstructed from savefile as a postprocessing stage to MKS
		MKS = 0,
		CGS,
		PLANCK
	};
	enum { SYSTEMS_COUNT = PLANCK+1 };

	static const fundamental_constants& get(units src);

	// conversion factors
	static const interval N_A;	// Avogadro constant

	// dimensionless constants
	static const interval inv_alpha;	// inverse fine structure constant
	static const interval alpha;	// fine structure constant

	// electroweak theory
	static const interval Z0_mass_GeV;
	static const interval W_mass_GeV;	// both W+ and W-
	static const interval Higgs_mass_GeV;
	static const interval cos_of_weak_mixing_angle;
	static const interval cos2_of_weak_mixing_angle;
	static const interval sin2_of_weak_mixing_angle;
	static const interval tan_of_weak_mixing_angle;
	static const interval tan2_of_weak_mixing_angle;
	static const interval Higgs_Lagrangian_g2_squared;
	static const interval Higgs_Lagrangian_g1_squared;
	static const interval Higgs_vacuum_expectation_value_GeV;
	static const interval Higgs_Lagrangian_mu_squared_GeV_squared;
	static const interval Higgs_Lagrangian_self_interaction_lambda;
	static const interval CODATA_sin2_of_weak_mixing_angle;
	static const interval PDG_sin2_of_weak_mixing_angle;
	static const interval PDG_sin2_of_weak_mixing_angle_effective;

	// tracking representative units
	dim_analysis::length distance_unit;	// in meters
	dim_analysis::time time_unit;	// in seconds
	dim_analysis::mass mass_unit;	// in kilograms; problematic for solar system units due to overprecision of the Sun's GM
	dim_analysis::temperature temperature_unit;	// in kelvin
	dim_analysis::charge charge_unit;	// in coulomb; not geometrizable, have to choose between clean force law and charge of electron=1

	// these four are the geometrizable constants: set all four to 1 to uniquely solve the above units.
	// These three are from Misner/Thorne/Wheeler
	dim_analysis::speed c;	// speed of light in vacuum
	dim_analysis::mult<dim_analysis::power<dim_analysis::speed, 3>::type, dim_analysis::div<dim_analysis::time, dim_analysis::mass>::type>::type G;	// Newtonian G
	dim_analysis::div<dim_analysis::energy, dim_analysis::temperature>::type k;	// Boltzmann constant
	// No source for this; equates General Relatvity momentum and Quantum Mechanics momentum.
	dim_analysis::mult<dim_analysis::energy, dim_analysis::time>::type h;	// Planck constant
	dim_analysis::mult<dim_analysis::energy, dim_analysis::time>::type h_bar;	// Planck constant/2pi

#if 0
	// following are from atomic units
	interval m_e;	// electron rest mass
//	interval k_e;	// coloumb force constant; 1/(4pi permittivity of free space)

//	vacuum permeability := (2*fine_structure_constant/electron_charge^2) * (planck_constant/c) i.e. magnetic constant
//	we have by construction (Maxwell equations) vacuum permititivity*vacuum permeability = 1/c^2
//  so vacuum permittivity := electron_charge^2/(2 fine structure constant planck_constant c) i.e. electric constant
	interval mu_0;	// permeability of free space; magnetic constant
	interval epsilon_0;	// permittivity of free space; electric constant
#endif
	// atomic units
	dim_analysis::mass amu_mass;
	dim_analysis::charge Q_e;	// electron charge; while this is numerically the same as the electron-volt energy unit in MKS, 
					// the dimensions are different so they scale differently.

	// empirical units of particle physics
	dim_analysis::energy eV;	// electron-volt: energy unit
	dim_analysis::mass eV_mass;	// eV/c^2
	dim_analysis::momentum eV_momentum;	// eV/c
	dim_analysis::temperature eV_temperature;	// eV/k
	dim_analysis::time eV_time;	// h-bar/eV
	dim_analysis::length eV_distance;	// (h-bar c)/eV

	fundamental_constants();	// default-constructs to SI units.

	// unit system rescaling operations
	void mult_scale_distance(interval x);
	void div_scale_distance(interval x);
	void mult_scale_time(interval x);
	void div_scale_time(interval x);
	void mult_scale_mass(interval x);
	void div_scale_mass(interval x);
	void mult_scale_temperature(interval x);
	void div_scale_temperature(interval x);
	void mult_scale_charge(interval x);
	void div_scale_charge(interval x);

	void mult_scale(const dim_analysis::mass& x) { mult_scale_mass(x.x()); }
	void div_scale(const dim_analysis::mass& x) { div_scale_mass(x.x()); }
	void mult_scale(const dim_analysis::length& x) { mult_scale_distance(x.x()); }
	void div_scale(const dim_analysis::length& x) { div_scale_distance(x.x()); }
	void mult_scale(const dim_analysis::time& x) { mult_scale_time(x.x()); }
	void div_scale(const dim_analysis::time& x) { div_scale_time(x.x()); }
	void mult_scale(const dim_analysis::temperature& x) { mult_scale_temperature(x.x()); }
	void div_scale(const dim_analysis::temperature& x) { div_scale_temperature(x.x()); }
	void mult_scale(const dim_analysis::charge& x) { mult_scale_charge(x.x()); }
	void div_scale(const dim_analysis::charge& x) { div_scale_charge(x.x()); }

	void geometrize();
	void rebuild_eV_units();
};

const fundamental_constants& SI_units();	// i.e. MKS
const fundamental_constants& CGS_units();
const fundamental_constants& geometrized_units();
const fundamental_constants& solar_system_units();

// deal with English units later; there are too many choices of length unit to make an informed decision.

#endif
