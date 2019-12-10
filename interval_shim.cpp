#include "interval_shim.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "test_driver.h"

int main(int argc, char* argv[])
{	// parse options
	char buf[100];

	STRING_LITERAL_TO_STDOUT("starting main\n");

	const interval_shim::interval ref_pi(M_PI, nextafter(M_PI, 4));	// CRC Handbook says floating point representation of M_PI is "low"; this brackets
	INFORM(interval_shim::pi);
	INFORM(ref_pi);
	INFORM(interval_shim::pi.lower() == ref_pi.lower() ? "true" : "false");
	INFORM(interval_shim::pi.upper() == ref_pi.upper() ? "true" : "false");

	const interval_shim::interval ref_sqrt_2 = sqrt(interval_shim::interval(2));
	INFORM(ref_sqrt_2);
	INFORM(nextafter(ref_sqrt_2.lower(),2) == ref_sqrt_2.upper() ? "true" : "false");
	INFORM(interval_shim::SQRT2.lower() == ref_sqrt_2.lower() ? "true" : "false");
	INFORM(interval_shim::SQRT2.upper() == ref_sqrt_2.upper() ? "true" : "false");
	INFORM(square(interval_shim::SQRT2).contains(2) ? "true" : "false");

	const interval_shim::interval ref_sqrt_3 = sqrt(interval_shim::interval(3));
	INFORM(ref_sqrt_3);
	INFORM(nextafter(ref_sqrt_3.lower(), 3) == ref_sqrt_3.upper() ? "true" : "false");
	INFORM(interval_shim::SQRT3.lower() == ref_sqrt_3.lower() ? "true" : "false");
	INFORM(interval_shim::SQRT3.upper() == ref_sqrt_3.upper() ? "true" : "false");
	INFORM(square(interval_shim::SQRT3).contains(3) ? "true" : "false");

	constexpr const auto one_f = zaimoni::binary<float>(1);
	constexpr const auto one_d = zaimoni::binary<double>(1);
	constexpr const auto one_ld = zaimoni::binary<long double>(1);
	INFORM("binary<float>(1)");
	INFORM(one_f._x);
	INFORM(one_f.sign);
	INFORM(one_f.exp);
	INFORM(one_f.exponent());
	INFORM(one_f.mant);
	INFORM(one_f.mantissa());
	INFORM("binary<double>(1)");
	INFORM(one_d._x);
	INFORM(one_d.sign);
	INFORM(one_d.exp);
	INFORM(one_d.exponent());
	INFORM(one_d.mant);
	INFORM(one_d.mantissa());
	INFORM("binary<long double>(1)");
	INFORM(one_ld._x);
	INFORM(one_ld.sign);
	INFORM(one_ld.exp);
	INFORM(one_ld.exponent());
	INFORM(one_ld.mant);
	INFORM(one_ld.mantissa());

	constexpr const auto neg_one_f = zaimoni::binary<float>(-1);
	constexpr const auto neg_one_d = zaimoni::binary<double>(-1);
	constexpr const auto neg_one_ld = zaimoni::binary<long double>(-1);
	INFORM("binary<float>(-1)");
	INFORM(neg_one_f._x);
	INFORM(neg_one_f.sign);
	INFORM(neg_one_f.exp);
	INFORM(neg_one_f.exponent());
	INFORM(neg_one_f.mant);
	INFORM(neg_one_f.mantissa());
	INFORM("binary<double>(-1)");
	INFORM(neg_one_d._x);
	INFORM(neg_one_d.sign);
	INFORM(neg_one_d.exp);
	INFORM(neg_one_d.exponent());
	INFORM(neg_one_d.mant);
	INFORM(neg_one_d.mantissa());
	INFORM("binary<long double>(-1)");
	INFORM(neg_one_ld._x);
	INFORM(neg_one_ld.sign);
	INFORM(neg_one_ld.exp);
	INFORM(neg_one_ld.exponent());
	INFORM(neg_one_ld.mant);
	INFORM(neg_one_ld.mantissa());

	STRING_LITERAL_TO_STDOUT("tests finished\n");
}


