#include "mass.hpp"
#include "Zaimoni.STL/Logging.h"

mass::mass(measured _m, fundamental_constants::units _u, const interval& src)
: _x(src),_mode((assert(SCHWARZSCHILD_RADIUS>=_m),assert(MASS <=_m),assert(fundamental_constants::SYSTEMS_COUNT >_u),assert(((unsigned char)(-1)-1)/SCHWARZSCHILD_RADIUS>=_u), _u*SCHWARZSCHILD_RADIUS + (_m - 1)))
{
}

mass::interval mass::m() const 
{
	const auto m_code = measurement_code();
	if (MASS == m_code) return _x;
	const auto u_code = system_code();
	if (fundamental_constants::PLANCK == u_code) return _x;	// all correction multipliers are 1 in Planck units
	const fundamental_constants& system = fundamental_constants::get(u_code);
	switch (m_code)
	{
	case ENERGY: return _x / square(system.c);
	case ASTRODYNAMIC: return _x / system.G;
	case SCHWARZSCHILD_RADIUS: return _x / system.G * square(system.c);
	default: _fatal_code("unhandled measurement code", 3);
	}
}

mass::interval mass::E() const
{
	const auto m_code = measurement_code();
	if (ENERGY == m_code) return _x;
	const auto u_code = system_code();
	if (fundamental_constants::PLANCK == u_code) return _x;	// all correction multipliers are 1 in Planck units
	const fundamental_constants& system = fundamental_constants::get(u_code);
	switch (m_code)
	{
	case MASS: return _x * square(system.c);
	case ASTRODYNAMIC: return _x / system.G * square(system.c);
	case SCHWARZSCHILD_RADIUS: return _x / system.G * square(square(system.c));
	default: _fatal_code("unhandled measurement code", 3);
	}
}

mass::interval mass::Schwarzschild_r() const
{
	const auto m_code = measurement_code();
	if (SCHWARZSCHILD_RADIUS == m_code) return _x;
	const auto u_code = system_code();
	if (fundamental_constants::PLANCK == u_code) return _x;	// all correction multipliers are 1 in Planck units
	const fundamental_constants& system = fundamental_constants::get(u_code);
	switch (m_code)
	{
	case MASS: return _x * system.G / square(system.c);
	case ENERGY: return _x * system.G / square(square(system.c));
	case ASTRODYNAMIC: return _x / square(system.c);
	default: _fatal_code("unhandled measurement code", 3);
	}
}

mass::interval mass::GM() const
{
	const auto m_code = measurement_code();
	if (ASTRODYNAMIC == m_code) return _x;
	const auto u_code = system_code();
	if (fundamental_constants::PLANCK == u_code) return _x;	// all correction multipliers are 1 in Planck units
	const fundamental_constants& system = fundamental_constants::get(u_code);
	switch (m_code)
	{
	case MASS: return _x * system.G;
	case ENERGY:  return _x * system.G / square(system.c);
	case SCHWARZSCHILD_RADIUS: return _x * square(system.c);
	default: _fatal_code("unhandled measurement code", 3);
	}
}

mass::interval mass::restmass_zero_momentum() const
{
	const auto u_code = system_code();
	if (fundamental_constants::PLANCK == u_code) return _x;	// all correction multipliers are 1 in Planck units
	const fundamental_constants& system = fundamental_constants::get(u_code);
	return E() / system.c;
}

mass::interval mass::DeBroglie_wavelength() const
{
	const auto u_code = system_code();
	if (fundamental_constants::PLANCK == u_code) return _x/(2.0*interval_shim::pi);	// h is 2pi in Planck units
	const fundamental_constants& system = fundamental_constants::get(u_code);
	return E() / system.h;
}


bool operator==(const mass& lhs, const mass& rhs)
{
	if (lhs._mode == rhs._mode && lhs._x.upper()==lhs._x.lower() && rhs._x==lhs._x.upper()) return true;
	if (zaimoni::is_zero(lhs._x) && zaimoni::is_zero(rhs._x)) return true;
	return false;	// we don't try to do unit conversions
}

#ifdef TEST_APP2
// example build line
// g++ -std=c++14 -otest.exe -D__STDC_LIMIT_MACROS -DTEST_APP2 mass.cpp constants.cpp -Llib\host.isk -lz_stdio_c -lz_log_adapter -lz_stdio_log -lz_format_util

#include "test_driver.h"

int main(int argc, char* argv[])
{	// parse options
	char buf[100];

	STRING_LITERAL_TO_STDOUT("starting main\n");

	STRING_LITERAL_TO_STDOUT("\none kilogram:\n");
	mass one_kg(mass::MASS, fundamental_constants::MKS, 1.0);
	assert(fundamental_constants::MKS == one_kg.system_code());
	assert(mass::MASS == one_kg.measurement_code());

	INTERVAL_TO_STDOUT(one_kg.m(), " kg\n");
	INTERVAL_TO_STDOUT(one_kg.E(), " J\n");
	INTERVAL_TO_STDOUT(one_kg.Schwarzschild_r(), " m\n");
	INTERVAL_TO_STDOUT(one_kg.GM(), " m^3 s^-2\n");

	STRING_LITERAL_TO_STDOUT("\none joule:\n");
	mass one_joule(mass::ENERGY, fundamental_constants::MKS, 1.0);
	assert(fundamental_constants::MKS == one_joule.system_code());
	assert(mass::ENERGY == one_joule.measurement_code());

	INTERVAL_TO_STDOUT(one_joule.m(), " kg\n");
	INTERVAL_TO_STDOUT(one_joule.E(), " J\n");
	INTERVAL_TO_STDOUT(one_joule.Schwarzschild_r(), " m\n");
	INTERVAL_TO_STDOUT(one_joule.GM(), " m^3 s^-2\n");

	STRING_LITERAL_TO_STDOUT("\nunit Schwarzschild radius:\n");
	mass one_meter(mass::SCHWARZSCHILD_RADIUS, fundamental_constants::MKS, 1.0);
	assert(fundamental_constants::MKS == one_meter.system_code());
	assert(mass::SCHWARZSCHILD_RADIUS == one_meter.measurement_code());

	INTERVAL_TO_STDOUT(one_meter.m(), " kg\n");
	INTERVAL_TO_STDOUT(one_meter.E(), " J\n");
	INTERVAL_TO_STDOUT(one_meter.Schwarzschild_r(), " m\n");
	INTERVAL_TO_STDOUT(one_meter.GM(), " m^3 s^-2\n");

	STRING_LITERAL_TO_STDOUT("\nunit gravitational potential GM\n");
	mass one_GM(mass::ASTRODYNAMIC, fundamental_constants::MKS, 1.0);
	assert(fundamental_constants::MKS == one_GM.system_code());
	assert(mass::ASTRODYNAMIC == one_GM.measurement_code());

	INTERVAL_TO_STDOUT(one_GM.m(), " kg\n");
	INTERVAL_TO_STDOUT(one_GM.E(), " J\n");
	INTERVAL_TO_STDOUT(one_GM.Schwarzschild_r(), " m\n");
	INTERVAL_TO_STDOUT(one_GM.GM(), " m^3 s^-2\n");

	STRING_LITERAL_TO_STDOUT("\ntests finished\n");
}

#endif
