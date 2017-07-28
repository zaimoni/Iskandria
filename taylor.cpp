// taylor.cpp

#include "taylor.hpp"
#include "cyclic_fn.hpp"


// want:
// function object x -> (-1)^x
// function object x -> f(x/2) if even, 0 if odd
// function object x -> x-1
// cos is then x -> -1^(x/2) when even, 0 if odd
// sin is then x -> -1^((x-1)/2) when odd, 0 when even
// so gateway function is a Kronecker delta on an x_0 mod 2
// if that survives, call f((x-x_0)/2): f: x -> (-1)^x o g: x -> (x - x_0)/2

namespace zaimoni {
namespace math {

// figure out where this goes later
static const zaimoni::math::mod_n::cyclic_fn_enumerated<2,signed char>&  alternator()
{
	static const signed char tmp[2] = {1,-1};
	static zaimoni::math::mod_n::cyclic_fn_enumerated<2,signed char> ret(tmp);
	return ret;
}


const TaylorSeries<int>& cos()
{
	static TaylorSeries<int> ret(alternator());
	return ret;
}

const TaylorSeries<int>& sin()
{
	static TaylorSeries<int> ret(alternator());
	return ret;
}


}	// namespace math
}	// namespace zaimoni

#ifdef TEST_APP2
// example build line (have to copy from *.hpp to *.cpp or else main not seen
// g++ -olossy.exe -Llib/host -DTEST_APP lossy.cpp -lz_stdio_log
// If doing INFORM-based debugging
// g++ -std=c++11 -otaylor.exe -DTEST_APP2 -D__STDC_LIMIT_MACROS taylor.cpp -Llib/host.isk -lz_log_adapter -lz_stdio_log -lz_format_util

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


	STRING_LITERAL_TO_STDOUT("tests finished\n");
}

#endif

