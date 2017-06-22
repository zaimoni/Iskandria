// has_std_unique_ptr.cpp
// tests for C++11 std::unique_ptr
// (C)2015 Kenneth Boyd, license: Boost.txt */

#include <memory>

#include <stdio.h>
#define STRING_LITERAL_TO_STDOUT(A) fwrite(A,sizeof(A)-1,1,stdout)

int main()
{
	std::unique_ptr<char> test;

	STRING_LITERAL_TO_STDOUT("#define ZAIMONI_HAS_STD_UNIQUE_PTR 1\n");
	return 0;	
}
