// lossy.cpp
// pure test driver

#include "lossy.hpp"

// example build line (have to copy from *.hpp to *.cpp or else main not seen
// g++ -olossy.exe -Llib/host lossy.cpp -lz_stdio_log
// If doing INFORM-based debugging
// g++ -std=c++11 -olossy.exe lossy.cpp -Llib/host.isk -lz_log_adapter -lz_stdio_log -lz_format_util

#include <stdlib.h>
#include <stdio.h>

// console-mode application
#define STRING_LITERAL_TO_STDOUT(A) fwrite(A,sizeof(A)-1,1,stdout)
#define C_STRING_TO_STDOUT(A) fwrite(A,strlen(A),1,stdout)
#define STL_STRING_TO_STDOUT(A) fwrite((A).data(),(A).size(),1,stdout)
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
