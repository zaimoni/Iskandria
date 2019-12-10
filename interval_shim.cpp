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

	constexpr const auto inf_f = zaimoni::binary<float>(std::numeric_limits<float>::infinity());
	constexpr const auto inf_d = zaimoni::binary<double>(std::numeric_limits<double>::infinity());
	constexpr const auto inf_ld = zaimoni::binary<long double>(std::numeric_limits<long double>::infinity());
	INFORM("binary<float>(inf)");
	INFORM(inf_f._x);
	INFORM(inf_f.sign);
	INFORM(inf_f.exp);
	INFORM(inf_f.exponent());
	INFORM(inf_f.mant);
	INFORM(inf_f.mantissa());
	INFORM("binary<double>(inf)");
	INFORM(inf_d._x);
	INFORM(inf_d.sign);
	INFORM(inf_d.exp);
	INFORM(inf_d.exponent());
	INFORM(inf_d.mant);
	INFORM(inf_d.mantissa());
	INFORM("binary<long double>(inf)");
	INFORM(inf_ld._x);
	INFORM(inf_ld.sign);
	INFORM(inf_ld.exp);
	INFORM(inf_ld.exponent());
	INFORM(inf_ld.mant);
	INFORM(inf_ld.mantissa());

	// most significant bit: qNaN
	constexpr const auto qnan_f = zaimoni::binary<float>(std::numeric_limits<float>::quiet_NaN());
	constexpr const auto qnan_d = zaimoni::binary<double>(std::numeric_limits<double>::quiet_NaN());
	constexpr const auto qnan_ld = zaimoni::binary<long double>(std::numeric_limits<long double>::quiet_NaN());
	INFORM("binary<float>(qnan)");
	INFORM(qnan_f._x);
	INFORM(qnan_f.sign);
	INFORM(qnan_f.exp);
	INFORM(qnan_f.exponent());
	INFORM(qnan_f.mant);
	INFORM(qnan_f.mantissa());
	INFORM("binary<double>(qnan)");
	INFORM(qnan_d._x);
	INFORM(qnan_d.sign);
	INFORM(qnan_d.exp);
	INFORM(qnan_d.exponent());
	INFORM(qnan_d.mant);
	INFORM(qnan_d.mantissa());
	INFORM("binary<long double>(qnan)");
	INFORM(qnan_ld._x);
	INFORM(qnan_ld.sign);
	INFORM(qnan_ld.exp);
	INFORM(qnan_ld.exponent());
	INFORM(qnan_ld.mant);
	INFORM(qnan_ld.mantissa());

	// least significant bit: sNaN
	constexpr const auto snan_f = zaimoni::binary<float>(std::numeric_limits<float>::signaling_NaN());
	constexpr const auto snan_d = zaimoni::binary<double>(std::numeric_limits<double>::signaling_NaN());
	constexpr const auto snan_ld = zaimoni::binary<long double>(std::numeric_limits<long double>::signaling_NaN());
	INFORM("binary<float>(snan)");
	INFORM(snan_f._x);
	INFORM(snan_f.sign);
	INFORM(snan_f.exp);
	INFORM(snan_f.exponent());
	INFORM(snan_f.mant);
	INFORM(snan_f.mantissa());
	INFORM("binary<double>(snan)");
	INFORM(snan_d._x);
	INFORM(snan_d.sign);
	INFORM(snan_d.exp);
	INFORM(snan_d.exponent());
	INFORM(snan_d.mant);
	INFORM(snan_d.mantissa());
	INFORM("binary<long double>(snan)");
	INFORM(snan_ld._x);
	INFORM(snan_ld.sign);
	INFORM(snan_ld.exp);
	INFORM(snan_ld.exponent());
	INFORM(snan_ld.mant);
	INFORM(snan_ld.mantissa());

	INFORM("zaimoni::isFinite");
//	constexpr const auto is_finite_constexpr = zaimoni::isFinite(one_d._x);
	INC_INFORM("1:");
	INC_INFORM(zaimoni::isFinite(one_f._x) ? " true" : " false");
	INC_INFORM(zaimoni::isFinite(one_d._x) ? " true" : " false");
	INFORM(zaimoni::isFinite(one_ld._x) ? " true" : " false");
	INC_INFORM("-1:");
	INC_INFORM(zaimoni::isFinite(neg_one_f._x) ? " true" : " false");
	INC_INFORM(zaimoni::isFinite(neg_one_d._x) ? " true" : " false");
	INFORM(zaimoni::isFinite(neg_one_ld._x) ? " true" : " false");
	INC_INFORM("inf:");
	INC_INFORM(zaimoni::isFinite(inf_f._x) ? " true" : " false");
	INC_INFORM(zaimoni::isFinite(inf_d._x) ? " true" : " false");
	INFORM(zaimoni::isFinite(inf_ld._x) ? " true" : " false");
	INC_INFORM("qNaN:");
	INC_INFORM(zaimoni::isFinite(qnan_f._x) ? " true" : " false");
	INC_INFORM(zaimoni::isFinite(qnan_d._x) ? " true" : " false");
	INFORM(zaimoni::isFinite(qnan_ld._x) ? " true" : " false");
	INC_INFORM("sNaN:");
	INC_INFORM(zaimoni::isFinite(snan_f._x) ? " true" : " false");
	INC_INFORM(zaimoni::isFinite(snan_d._x) ? " true" : " false");
	INFORM(zaimoni::isFinite(snan_ld._x) ? " true" : " false");

	STRING_LITERAL_TO_STDOUT("tests finished\n");
}


