// has_std_copy_n.cpp
// tests for C++11 std::copy_n
// (C)2015 Kenneth Boyd, license: Boost.txt */

#include <algorithm>

#include <stdio.h>
#define STRING_LITERAL_TO_STDOUT(A) fwrite(A,sizeof(A)-1,1,stdout)

int main()
{
	char dest[sizeof("Test")];
	const char* src = "Test";
	std::copy_n(src,sizeof("Test"),dest);

	STRING_LITERAL_TO_STDOUT("#define ZAIMONI_HAS_STD_COPY_N 1\n");
	return 0;	
}
