// dicounter.cpp
// pure test driver

#include "dicounter.hpp"

// example build line (have to copy from *.hpp to *.cpp or else main not seen
// g++ -std=c++11 -odicounter.exe -DTEST_APP -D__STDC_LIMIT_MACROS dicounter.cpp -Llib/host.isk -lz_stdio_log
// If doing INFORM-based debugging
// g++ -std=c++11 -odicounter.exe -DTEST_APP -D__STDC_LIMIT_MACROS dicounter.cpp -Llib/host.isk -lz_log_adapter -lz_stdio_log -lz_format_util

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
