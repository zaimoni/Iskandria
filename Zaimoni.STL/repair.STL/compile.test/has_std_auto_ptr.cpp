// has_std_auto_ptr.cpp
// tests for C++98 std::auto_ptr
// (C)2015 Kenneth Boyd, license: Boost.txt */

#include <memory>

#include <stdio.h>
#define STRING_LITERAL_TO_STDOUT(A) fwrite(A,sizeof(A)-1,1,stdout)

int main()
{
	std::auto_ptr<char> test;

	STRING_LITERAL_TO_STDOUT("#define ZAIMONI_HAS_STD_AUTO_PTR 1\n");
	return 0;	
}
