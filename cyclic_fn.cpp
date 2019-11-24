// cyclic_fn.cpp
// pure test driver

#include "cyclic_fn.hpp"
#include <functional>

// example build line (have to copy from *.hpp to *.cpp or else main not seen
// If doing INFORM-based debugging
// g++ -std=c++11 -ocyclic_fn.exe cyclic_fn.cpp -Llib/host.isk -lz_log_adapter -lz_stdio_log -lz_format_util

#include "test_driver.h"

int main(int argc, char* argv[])
{	// parse options
	char buf[100];

	STRING_LITERAL_TO_STDOUT("starting main\n");

	const signed char tmp[2] = {1,-1};

	zaimoni::math::mod_n::cyclic_fn_enumerated<2,signed char> test(tmp);

	INFORM(test(0));
	INFORM(test(1));
	INFORM(test(2));

	std::function<signed char (uintmax_t)> test_fn = test;

	INFORM(test_fn(0));
	INFORM(test_fn(1));
	INFORM(test_fn(2));

	STRING_LITERAL_TO_STDOUT("tests finished\n");
}
