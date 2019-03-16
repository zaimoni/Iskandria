// int_range.cpp
// pure test driver

#include "int_range.hpp"

// example build line (have to copy from *.hpp to *.cpp or else main not seen
// g++ -oint_range.exe -Llib/host int_range.cpp -lz_stdio_log
// If doing INFORM-based debugging
// g++ -std=c++11 -oint_range.exe int_range.cpp -Llib/host.isk -lz_log_adapter -lz_stdio_log -lz_format_util

#include "test_driver.h"

int main(int argc, char* argv[])
{	// parse options
	char buf[100];

	STRING_LITERAL_TO_STDOUT("starting main\n");

	// some basic compile tests
	zaimoni::math::int_range<uintmax_t> unsigned_test(1,10);
	zaimoni::math::int_range<intmax_t> signed_test(1,10);

	unsigned_test.crbegin();
	unsigned_test.crend();

	INFORM(unsigned_test.front());
	INFORM(signed_test.back());

	unsigned_test.pop_front();
	signed_test.pop_back();

	INFORM(unsigned_test.front());
	INFORM(signed_test.back());

	unsigned_test==unsigned_test;
	signed_test<signed_test;

	swap(unsigned_test,unsigned_test);

	INFORM("checking iteration");
	for(auto i : unsigned_test) INFORM(i);

	STRING_LITERAL_TO_STDOUT("tests finished\n");
}
