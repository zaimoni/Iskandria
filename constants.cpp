// constants.cpp

#include "constants.hpp"

// CODATA 2010
#define SI_CODATA_C 299792458.0

#define SI_TO_CGS_DISTANCE_SCALE 100.0

// default-initialize to SI i.e. MKS
fundamental_constants::fundamental_constants()
:	distance_unit(1.0),
	time_unit(1.0),
	mass_unit(1.0),
	temperature_unit(1.0),
 	c(299792458.0),	// CODATA 2010/2014; m/s
//	G(6.67304e-11,6.67464e-11),	// CODATA 2010; m^3 kg^-1 s^-2
	G(6.67377e-11,6.67439e-11),	// CODATA 2014; m^3 kg^-1 s^-2
//	k(1.3806475e-23,1.3806501e-23),	// CODATA 2010; J/K i.e. m^2 kg s^-2 K^-1
	k(1.38064773e-23,1.38064921e-23),	// CODATA 2014; J/K i.e. m^2 kg s^-2 K^-1
//	h_bar(1.054571679e-34,1.054571773e-34)	// CODATA 2010; J s i.e. m^2 kg s^-1
	h_bar(1.054571787e-34,1.054571813e-34)	// CODATA 2014; J s i.e. m^2 kg s^-1
{
}

void fundamental_constants::mult_scale_distance(boost::numeric::interval<double> x)
{
	const boost::numeric::interval<double> x2(square(x));
	distance_unit /= x;
	c *= x;
	G *= x;
	G *= x2;
	k *= x2;
	h_bar *= x2;
}

void fundamental_constants::div_scale_distance(boost::numeric::interval<double> x)
{
	const boost::numeric::interval<double> x2(square(x));
	distance_unit *= x;
	c /= x;
	G /= x;
	G /= x2;
	k /= x2;
	h_bar /= x2;
}

void fundamental_constants::mult_scale_time(boost::numeric::interval<double> x)
{
	const boost::numeric::interval<double> x2(square(x));
	time_unit /= x;
	c /= x;
	G /= x2;
	k /= x2;
	h_bar /= x;
}

void fundamental_constants::div_scale_time(boost::numeric::interval<double> x)
{
	const boost::numeric::interval<double> x2(square(x));
	time_unit *= x;
	c *= x;
	G *= x2;
	k *= x2;
	h_bar *= x;
}

void fundamental_constants::mult_scale_mass(boost::numeric::interval<double> x)
{
	mass_unit /= x;
	G /= x;
	k *= x;
	h_bar *= x;
}

void fundamental_constants::div_scale_mass(boost::numeric::interval<double> x)
{
	mass_unit *= x;
	G *= x;
	k /= x;
	h_bar /= x;
}

void fundamental_constants::mult_scale_temperature(boost::numeric::interval<double> x)
{
	temperature_unit /= x;
	k /= x;
}

void fundamental_constants::div_scale_temperature(boost::numeric::interval<double> x)
{
	temperature_unit *= x;
	k *= x;
}

void fundamental_constants::geometrize()
{
/*
: 	c(299792458.0),	// CODATA 2010; m/s
	G(6.67304e-11,6.67464e-11),	// CODATA 2010; m^3 kg^-1 s^-2
	k(1.3806475e-23,1.3806501e-23),	// CODATA 2010; J/K i.e. m^2 kg s^-2 K^-1
	h_bar(1.054571679e-34,1.054571773e-34)	// CODATA 2010; J s i.e. m^2 kg s^-1

    geometrized:
    1 = dist time^-1
    1 = dist^3 mass^-1 time^-2
    1 = dist^2 mass time^-2 temperature^-1
    1 = dist^2 mass time^-1

    dimensions of
    G/c^2: dist mass^-1
    k/c^2: mass temperature^-1
    c^2/k: temperature mass^-1
    h_bar/c : dist mass

    G*h_bar/c^3: dist^2
    h_bar*c/G: mass^2   
 */
	const boost::numeric::interval<double> geo_dist_squared_1 = (G/pow(c,3))*h_bar;
	const boost::numeric::interval<double> geo_dist_squared_2 = (h_bar/pow(c,3))*G;
	const boost::numeric::interval<double> geo_dist_squared_3 = (G*h_bar)/pow(c,3);
	const boost::numeric::interval<double> geo_dist_squared = intersect(intersect(geo_dist_squared_1,geo_dist_squared_2),geo_dist_squared_3);

	const boost::numeric::interval<double> geo_time_squared_1 = (G/pow(c,5))*h_bar;
	const boost::numeric::interval<double> geo_time_squared_2 = (h_bar/pow(c,5))*G;
	const boost::numeric::interval<double> geo_time_squared_3 = (G*h_bar)/pow(c,5);
	const boost::numeric::interval<double> geo_time_squared = intersect(intersect(geo_time_squared_1,geo_time_squared_2),geo_time_squared_3);

	const boost::numeric::interval<double> geo_mass_squared_1 = (h_bar/G)*c;
	const boost::numeric::interval<double> geo_mass_squared_2 = (c/G)*h_bar;
	const boost::numeric::interval<double> geo_mass_squared_3 = (h_bar*c)/G;
	const boost::numeric::interval<double> geo_mass_squared = intersect(intersect(geo_mass_squared_1,geo_mass_squared_2),geo_mass_squared_3);

	const boost::numeric::interval<double> geo_temperature_1 = (square(c)/k)*sqrt(geo_mass_squared);
	const boost::numeric::interval<double> geo_temperature_2 = (sqrt(geo_mass_squared)/k)*square(c);
	const boost::numeric::interval<double> geo_temperature_3 = (square(c)*sqrt(geo_mass_squared))/k;
	const boost::numeric::interval<double> geo_temperature = intersect(intersect(geo_temperature_1,geo_temperature_2),geo_temperature_3);

	div_scale_distance(sqrt(geo_dist_squared));
	div_scale_time(sqrt(geo_time_squared));
	div_scale_mass(sqrt(geo_mass_squared));
	div_scale_temperature(geo_temperature);

	// set geometrized constants to 1
	c = 1.0;
	G = 1.0;
	k = 1.0;
	h_bar = 1.0;
}

const fundamental_constants& SI_units()
{
	static fundamental_constants* x = NULL;
	if (x) return *x;
	x = new fundamental_constants();
	return *x;
}

const fundamental_constants& CGS_units()
{
	static fundamental_constants* x = NULL;
	if (x) return *x;
	x = new fundamental_constants();
	x->mult_scale_distance(100.0);	// 100 cm to 1 m
	x->mult_scale_mass(1000.0);	// 1000 g to 1 kg
	return *x;
}

const fundamental_constants& geometrized_units()
{
	static fundamental_constants* x = NULL;
	if (x) return *x;
	x = new fundamental_constants();
	x->geometrize();
	return *x;
}

const fundamental_constants& solar_system_units()
{
	static fundamental_constants* x = NULL;
	if (x) return *x;
	x = new fundamental_constants();
	const boost::numeric::interval<double> SI_G(x->G);

	// time unit: 1 Earth year
	// distance unit: 1 AU (semimajor axis of Earth's orbit)
	// * 2012: 1 AU := 149,597,870,700 meters
	// mass unit: sum of Earth and Sun rest masses (or in practice, just the sun's rest mass)
	// celestial mechanics: gravitational parameter GM
	// for the earth and sun, this is known to very high precision
	// https://en.wikipedia.org/wiki/Standard_gravitational_parameter
	// geocentric (Earth) : 398600.4418±0.0008 km3 s-2
	// heliocentric (Sun) : 1.32712440018 x 10^20 (± 8 x 10^9) m3 s-2 http://ssd.jpl.nasa.gov/?constants
	
	x->div_scale_distance(1.495978707e11);	// AU definition
	x->div_scale_time(86400*365.25636);	// sidereal year, quasar reference frame
	x->div_scale_mass(boost::numeric::interval<double>(1.32712440010e20,1.32712440026e20)/SI_G);
	return *x;
}

#ifdef TEST_APP
// example build line
// g++ -oconstants.exe -DTEST_APP constants.cpp

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// console-mode application
#define STRING_LITERAL_TO_STDOUT(A) fwrite(A,sizeof(A)-1,1,stdout)
#define C_STRING_TO_STDOUT(A) fwrite(A,strlen(A),1,stdout)
#define STL_PTR_STRING_TO_STDOUT(A) fwrite((A)->data(),(A)->size(),1,stdout)

#define INTERVAL_TO_STDOUT(A,UNIT)	\
	if (A.lower()==A.upper()) {	\
		sprintf(buf,"%.16g",A.lower());	\
		C_STRING_TO_STDOUT(buf);	\
		STRING_LITERAL_TO_STDOUT(UNIT);	\
	} else {	\
		STRING_LITERAL_TO_STDOUT("[");	\
		sprintf(buf,"%.16g",A.lower());	\
		C_STRING_TO_STDOUT(buf);	\
		STRING_LITERAL_TO_STDOUT(", ");	\
		sprintf(buf,"%.16g",A.upper());	\
		C_STRING_TO_STDOUT(buf);	\
		STRING_LITERAL_TO_STDOUT("]");	\
		STRING_LITERAL_TO_STDOUT(UNIT);	\
	}

int main(int argc, char* argv[])
{	// parse options
	char buf[100];

	STRING_LITERAL_TO_STDOUT("starting main\n");

	STRING_LITERAL_TO_STDOUT("speed of light\n");
	INTERVAL_TO_STDOUT(SI_units().c," m/s\n");
	INTERVAL_TO_STDOUT(CGS_units().c," cm/s\n");
	INTERVAL_TO_STDOUT(geometrized_units().c," geometrized distance/time\n");
	INTERVAL_TO_STDOUT(solar_system_units().c," AU/sidereal year\n");

	STRING_LITERAL_TO_STDOUT("\nNewtonian G\n");
	INTERVAL_TO_STDOUT(SI_units().G,"\n");
	INTERVAL_TO_STDOUT(CGS_units().G,"\n");
	INTERVAL_TO_STDOUT(geometrized_units().G,"\n");
	INTERVAL_TO_STDOUT(solar_system_units().G,"\n");

	STRING_LITERAL_TO_STDOUT("\nBoltzmann constant k\n");
	INTERVAL_TO_STDOUT(SI_units().k,"\n");
	INTERVAL_TO_STDOUT(CGS_units().k,"\n");
	INTERVAL_TO_STDOUT(geometrized_units().k,"\n");
	INTERVAL_TO_STDOUT(solar_system_units().k,"\n");

	STRING_LITERAL_TO_STDOUT("\nPlanck constant over 2*pi h_bar\n");
	INTERVAL_TO_STDOUT(SI_units().h_bar,"\n");
	INTERVAL_TO_STDOUT(CGS_units().h_bar,"\n");
	INTERVAL_TO_STDOUT(geometrized_units().h_bar,"\n");
	INTERVAL_TO_STDOUT(solar_system_units().h_bar,"\n");

	STRING_LITERAL_TO_STDOUT("\nIAU units in SI\n");
	INTERVAL_TO_STDOUT(solar_system_units().distance_unit," m\n");
	INTERVAL_TO_STDOUT(solar_system_units().time_unit," s\n");
	INTERVAL_TO_STDOUT(solar_system_units().mass_unit," kg\n");
	INTERVAL_TO_STDOUT(solar_system_units().temperature_unit," K\n");

	STRING_LITERAL_TO_STDOUT("\nGeometrized units in SI\n");
	INTERVAL_TO_STDOUT(geometrized_units().distance_unit," m\n");
	INTERVAL_TO_STDOUT(geometrized_units().time_unit," s\n");
	INTERVAL_TO_STDOUT(geometrized_units().mass_unit," kg\n");
	INTERVAL_TO_STDOUT(geometrized_units().temperature_unit," K\n");

	const boost::numeric::interval<double> geo_dist_squared = SI_units().G*SI_units().h_bar/(SI_units().c*square(SI_units().c));
	STRING_LITERAL_TO_STDOUT("\nGeometrized distance unit squared\n");
	INTERVAL_TO_STDOUT(geo_dist_squared," m^2\n");

	const boost::numeric::interval<double> geo_time_squared = geo_dist_squared/square(SI_units().c);
	STRING_LITERAL_TO_STDOUT("\nGeometrized time unit squared\n");
	INTERVAL_TO_STDOUT(geo_time_squared," s^2\n");

	const boost::numeric::interval<double> geo_mass_squared = SI_units().h_bar*SI_units().c/SI_units().G;
	STRING_LITERAL_TO_STDOUT("\nGeometrized mass unit squared\n");
	INTERVAL_TO_STDOUT(geo_mass_squared," kg^2\n");

	const boost::numeric::interval<double> geo_temperature = square(SI_units().c)/SI_units().k*sqrt(geo_mass_squared);
	STRING_LITERAL_TO_STDOUT("\nGeometrized temperature unit squared\n");
	INTERVAL_TO_STDOUT(geo_temperature," K\n");

	fundamental_constants test_geometrization;
	test_geometrization.div_scale_distance(sqrt(geo_dist_squared));
	test_geometrization.div_scale_time(sqrt(geo_time_squared));
	test_geometrization.div_scale_mass(sqrt(geo_mass_squared));
	test_geometrization.div_scale_temperature(geo_temperature);
	STRING_LITERAL_TO_STDOUT("\nCross-check geometrization\n");
	INTERVAL_TO_STDOUT(test_geometrization.c,"\n");
	INTERVAL_TO_STDOUT(test_geometrization.G,"\n");
	INTERVAL_TO_STDOUT(test_geometrization.k,"\n");
	INTERVAL_TO_STDOUT(test_geometrization.h_bar,"\n");	

	STRING_LITERAL_TO_STDOUT("tests finished\n");
}

#endif
