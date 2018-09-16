// cyclic_fn.cpp
// pure test driver

#include "cyclic_fn.hpp"

// example build line (have to copy from *.hpp to *.cpp or else main not seen
// If doing INFORM-based debugging
// g++ -std=c++11 -ocyclic_fn.exe cyclic_fn.cpp -Llib/host.isk -lz_log_adapter -lz_stdio_log -lz_format_util

#include <stdlib.h>
#include <stdio.h>
#include <functional>

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
