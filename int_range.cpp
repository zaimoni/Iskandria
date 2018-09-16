// int_range.cpp
// pure test driver

#include "int_range.hpp"

// example build line (have to copy from *.hpp to *.cpp or else main not seen
// g++ -oint_range.exe -Llib/host int_range.cpp -lz_stdio_log
// If doing INFORM-based debugging
// g++ -std=c++11 -oint_range.exe int_range.cpp -Llib/host.isk -lz_log_adapter -lz_stdio_log -lz_format_util

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
