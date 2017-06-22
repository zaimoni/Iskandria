// has_header_typetraits.cpp
// tests for C++11 std::unique_ptr
// (C)2015 Kenneth Boyd, license: Boost.txt */

#include <type_traits>

#include <stdio.h>
#define STRING_LITERAL_TO_STDOUT(A) fwrite(A,sizeof(A)-1,1,stdout)

int main()
{
	STRING_LITERAL_TO_STDOUT("#define ZAIMONI_HAS_HEADER_TYPE_TRAITS 1\n");
	return 0;	
}
