// cyclic_fn.hpp

#ifndef CYCLIC_FN_HPP
#define CYCLIC_FN_HPP 1

#include "z_n.hpp"

namespace zaimoni {
namespace math {

namespace mod_n {

template<uintmax_t n,class T>
class cyclic_fn_enumerated
{
private:
	T _x[n];
//	mutable unsigned char _bitmap;
public:
	cyclic_fn_enumerated(const T* src) {memmove(_x,src,n*sizeof(T));};
	ZAIMONI_DEFAULT_COPY_DESTROY_ASSIGN(cyclic_fn_enumerated);

	T operator()(Z_<n> i) const {return _x[i];}
};

}
}	// namespace math
}	// namespace zaimoni

#ifdef TEST_APP
// example build line (have to copy from *.hpp to *.cpp or else main not seen
// g++ -olossy.exe -Llib/host -DTEST_APP lossy.cpp -lz_stdio_log
// If doing INFORM-based debugging
// g++ -std=c++11 -ocyclic_fn.exe -DTEST_APP cyclic_fn.cpp -Llib/host.isk -lz_log_adapter -lz_stdio_log -lz_format_util

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

#endif

#endif
