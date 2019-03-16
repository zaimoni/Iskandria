// lossy.cpp
// pure test driver

#include "lossy.hpp"

// example build line (have to copy from *.hpp to *.cpp or else main not seen
// g++ -olossy.exe -Llib/host lossy.cpp -lz_stdio_log
// If doing INFORM-based debugging
// g++ -std=c++11 -olossy.exe lossy.cpp -Llib/host.isk -lz_log_adapter -lz_stdio_log -lz_format_util

#include "test_driver.h"

int main(int argc, char* argv[])
{	// parse options
	char buf[100];

	STRING_LITERAL_TO_STDOUT("starting main\n");

	double lhs = 2.0;
	double rhs = 2.0;
	boost::numeric::interval<double> lhs_2 = 2.0;
	boost::numeric::interval<double> rhs_2 = 2.0;

	zaimoni::math::lossy<double>::sum(lhs,rhs);
	zaimoni::math::lossy<double>::product(lhs,rhs);

	zaimoni::math::lossy<double>::sum(lhs,rhs_2);
	zaimoni::math::lossy<double>::product(lhs,rhs_2);
	
	zaimoni::math::lossy<double>::sum(lhs_2,rhs);
	zaimoni::math::lossy<double>::product(lhs_2,rhs);

	zaimoni::math::lossy<double>::sum(lhs_2,rhs_2);
	zaimoni::math::lossy<double>::product(lhs_2,rhs_2);

	STRING_LITERAL_TO_STDOUT("tests finished\n");
}
