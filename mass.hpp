#ifndef MASS_HPP
#define MASS_HPP 1

#include "constants.hpp"

// unfortunately, there are several measurement systems that have more precise mass-related measurements than can be converted
// into human-scale e.g. SI units

// Cf. https://en.wikipedia.org/wiki/Standard_gravitiational_parameter

// move construction does nothing for this type
class mass
{
public:
	typedef fundamental_constants::interval interval;
	enum measured : unsigned char {
		NONE = 0,
		MASS,	// useless for rest-mass zero
		ENERGY,	// rest-energy for physical objects, but things w/rest mass zero still have energy
		ASTRODYNAMIC,	// GM
		SCHWARZSCHILD_RADIUS
	};
private:
	interval _x;	// the mass-related value
	unsigned char _mode;	// how we initialized it; encodes both "base system" and "measurement used"
public:
	mass() : _x(0),_mode(0) {}
	mass(measured _m, fundamental_constants::units _u, const interval& src);
	mass(const mass& src) = default;
	~mass() = default;
	mass& operator=(const mass& src) = default;

	fundamental_constants::units system_code() const { return (fundamental_constants::units)(_mode/SCHWARZSCHILD_RADIUS); }
	measured measurement_code() const { return (measured)((_mode%SCHWARZSCHILD_RADIUS)+1); }

	// readouts of interest
	dim_analysis::mass m() const;	// mass
	dim_analysis::energy E() const;	// energy
	dim_analysis::length Schwarzschild_r() const;	// Schwarzschild radius
	interval GM() const;	// standard gravitational parameter

	// could convert these to additional definition modes if needed
	dim_analysis::momentum restmass_zero_momentum() const;	// E/c
	dim_analysis::length DeBroglie_wavelength() const;		// also known as Compton wavelength; hc/E

	friend bool operator==(const mass& lhs, const mass& rhs);
};

#endif
