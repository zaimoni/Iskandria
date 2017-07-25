// dicounter.hpp

#ifndef DICOUNTER_HPP
#define DICOUNTER_HPP

#include <utility>
#include "Zaimoni.STL/Logging.h"

// typically would use intmax_t, but would like to be able to fully represent a uintmax_t in either direction
struct dicounter : std::pair<uintmax_t,uintmax_t>
{
	dicounter() = default;
	ZAIMONI_DEFAULT_COPY_DESTROY_ASSIGN(dicounter);

	uintmax_t& positive() {return this->first;};
	uintmax_t positive() const {return this->first;};
	uintmax_t& negative() {return this->second;};
	uintmax_t negative() const {return this->second;};

#define ZAIMONI_ADD_DEF(CAPACITY,OP,POS,NEG)	\
	uintmax_t CAPACITY () const {return UINTMAX_MAX-POS;}	\
	void OP (uintmax_t& src) {	\
		if (0==src) return;	\
		if (0<NEG) {	\
			if (src <= NEG) {	\
				NEG -= src;	\
			} else {	\
				src -= NEG;	\
				NEG = 0;	\
				POS = src;	\
			}	\
			src = 0;	\
			return;	\
		}	\
		const uintmax_t test = UINTMAX_MAX-POS;	\
		if (test>=src) {	\
			POS += src;	\
			src = 0;	\
			return;	\
		} else if (0<test) {	\
			POS = UINTMAX_MAX;	\
			src -= test;	\
			return;	\
		}	\
	}	\
	void safe_##OP (uintmax_t src) {	\
		if (0==src) return;	\
		if (0<NEG) {	\
			if (src <= NEG) {	\
				NEG -= src;	\
			} else {	\
				src -= NEG;	\
				NEG = 0;	\
				POS = src;	\
			}	\
			return;	\
		}	\
		const uintmax_t test = UINTMAX_MAX-POS;	\
		assert(test>=src);	\
		POS += src;	\
		return;	\
	}

ZAIMONI_ADD_DEF(add_capacity,add,positive(),negative())
ZAIMONI_ADD_DEF(sub_capacity,sub,negative(),positive())

#undef ZAIMONI_ADD_DEF
};

#ifdef TEST_APP
// example build line (have to copy from *.hpp to *.cpp or else main not seen
// g++ -olossy.exe -Llib/host -DTEST_APP lossy.cpp -lz_stdio_log
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

#endif


#endif
