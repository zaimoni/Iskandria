#include "conic.hpp"

namespace zaimoni {
namespace math {

conic::conic(conic_tags::ellipse_t, const interval& src, const interval& src2)
{	// need to work around correct order of parameters
	assert(0 <= src.lower());
	assert(0 <= src2.lower());
	if (src2.upper() <= src.lower()) {
		init_ellipse(src, src2);
		return;
	}
	if (src.upper() <= src2.lower()) {
		init_ellipse(src2, src2);
		return;
	}
	// synthesize
	const bool src_lb_lower = src.lower() <= src2.lower();
	const bool src_ub_lower = src.upper() <= src2.upper();
	interval major(src_lb_lower ? src2.lower() : src.lower(), src_ub_lower ? src.upper() : src2.upper());
	interval minor(src_lb_lower ? src.lower() : src2.lower(), src_ub_lower ? src2.upper() : src.upper());
	init_ellipse(major, minor);
}

conic::conic(conic_tags::hyperbola_t, const interval& src, const interval& src2)
{	// need to work around correct order of parameters
	assert(0 <= src.lower());
	assert(0 <= src2.lower());
	init_hyperbola(src, src2);
}

bool operator==(const conic& lhs, const conic& rhs)
{
	return definitely_equal(lhs._a, rhs._a) && definitely_equal(lhs._b, rhs._b) && definitely_equal(lhs._c, rhs._c) &&
	       definitely_equal(lhs._e, rhs._e) && definitely_equal(lhs._l, rhs._l) && definitely_equal(lhs._p, rhs._p);
}

void conic::init_ellipse(const interval& major, const interval& minor)
{
	_a = major;
	_b = minor;
	// \todo start overprecise arithemetic
	interval major2 = square(major);
	interval minor2 = square(minor);
	interval c2(major2 - minor2);
	if (0 > c2.lower()) c2.assign(0, c2.upper());	// should only trigger when axis ranges overlap
	if (major2.upper() < c2.upper()) c2.assign(c2.lower(), major2.upper());
	_c = sqrt(c2);
	if (0.0 >= c2.upper()) {
		// circle!
		_e = 0.0;
		_l = major;
		_p = interval::whole().upper();
		return;
	}
	// two reasonable expressions for eccentricity
	interval minor_major_quotient2 = minor2 / major2;
	if (1 < minor_major_quotient2.upper()) minor_major_quotient2.assign(minor_major_quotient2.lower(), 1);	// again, should only trigger when axis ranges overlap
	interval e2_1 = c2 / major2;
	interval e2_2 = 1.0 - minor_major_quotient2;	// \todo can do better if minor_major_quotient2 is very small
	_e = sqrt(intersect(e2_1, e2_2));
	_l = minor2 / major;
	// two reasonable expressions for focal parameter
	interval p1(minor2 / _c);
	interval p2(_l / _e);
	_p = intersect(p1, p2);
}

void conic::init_hyperbola(const interval& major, const interval& minor)
{
	_a = major;
	_b = minor;
	// \todo start overprecise arithemetic
	interval major2 = square(major);
	interval minor2 = square(minor);
	interval c2(major2 + minor2);
	_c = sqrt(c2);

	// two reasonable expressions for eccentricity
	interval minor_major_quotient2 = minor2 / major2;
	interval e2_1 = c2 / major2;
	interval e2_2 = 1.0 + minor_major_quotient2;	// \todo can do better if minor_major_quotient2 is very small
	_e = sqrt(intersect(e2_1, e2_2));

	_e = sqrt(1.0 + minor2 / major2);
	_l = minor2 / major;
	_p = minor2 / _c;
}

}	// namespace math
}	// namespace zaimoni

#ifdef TEST_APP2
// fast compile test
// g++ -std=c++14 -otest.exe -Os  -D__STDC_LIMIT_MACROS -DTEST_APP2 conic.test.cpp interval_shim.cpp -Llib\host.isk -lz_stdio_c -lz_log_adapter -lz_stdio_log -lz_format_util

#include "test_driver.h"

int main(int argc, char* argv[])
{
	typedef zaimoni::math::conic conic;

	// note that actually accessing boost::numeric::interval<double>::whole() is a runtime error
	STRING_LITERAL_TO_STDOUT("spot-checking whole()");
	conic::interval::whole();
	STRING_LITERAL_TO_STDOUT("whole() ok\n");

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

