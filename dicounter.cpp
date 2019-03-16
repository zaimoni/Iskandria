// dicounter.cpp
// pure test driver

#include "dicounter.hpp"

// example build line (have to copy from *.hpp to *.cpp or else main not seen
// g++ -std=c++11 -odicounter.exe -D__STDC_LIMIT_MACROS dicounter.cpp -Llib/host.isk -lz_stdio_log
// If doing INFORM-based debugging
// g++ -std=c++11 -odicounter.exe -D__STDC_LIMIT_MACROS dicounter.cpp -Llib/host.isk -lz_log_adapter -lz_stdio_log -lz_format_util

#include "test_driver.h"

int main(int argc, char* argv[])
{	// parse options
	char buf[100];

	STRING_LITERAL_TO_STDOUT("starting main\n");

	dicounter test;
	INFORM(test.positive());
	INFORM(test.negative());
	INFORM(test.add_capacity());
	INFORM(test.sub_capacity());
	uintmax_t src = UINTMAX_MAX;
	test.add(src);
	assert(UINTMAX_MAX==test.positive());
	assert(0==test.negative());
	assert(0==test.add_capacity());

	STRING_LITERAL_TO_STDOUT("tests finished\n");
}
