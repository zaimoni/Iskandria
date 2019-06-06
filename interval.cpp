#include "interval.hpp"
#define CONSTANTS_ISK_INTERVAL 1
#include "interval_shim.hpp"

// purely a test driver.

// fast compile test
// g++ -std=c++14 -otest.exe -Os -D__STDC_LIMIT_MACROS interval.cpp -Llib\host.isk -lz_stdio_c -lz_log_adapter -lz_stdio_log -lz_format_util
int main(int argc, char* argv[])
{
	// compile-time checks
	INFORM("long double intervals");
	INFORM(zaimoni::math::interval<long double>::empty());
	INFORM(zaimoni::math::interval<long double>::whole());
	{
	const auto unit = zaimoni::math::interval<long double>::hull(0, 1);
	INFORM(unit);
	INFORM(unit.width());
	INFORM(unit.median());
	INFORM(zaimoni::norm(unit));
	assert(!zaimoni::is_zero(unit));
	assert(zaimoni::contains_zero(unit));
	assert(!zaimoni::is_positive(unit));
	assert(!zaimoni::is_negative(unit));
	}

	INFORM("\nlong long intervals");
	INFORM(zaimoni::math::interval<long long>::empty());
	INFORM(zaimoni::math::interval<long long>::whole());
	{
	const auto unit = zaimoni::math::interval<long long>::hull(0, 1);
	INFORM(unit);
	INFORM(unit.width());
	INFORM(unit.median());
	INFORM(zaimoni::norm(unit));
	assert(!zaimoni::is_zero(unit));
	assert(zaimoni::contains_zero(unit));
	assert(!zaimoni::is_positive(unit));
	assert(!zaimoni::is_negative(unit));
	}

	zaimoni::isINF(1);

	INFORM("\nDone");

	return 0;
}

