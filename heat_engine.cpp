#include "heat_engine.hpp"

#ifdef TEST_APP3
// example build line
// g++ -std=c++14 -otest.exe -D__STDC_LIMIT_MACROS -DTEST_APP3 heat_engine.cpp mass.cpp constants.cpp interval_shim.cpp -Llib\host.isk -lz_stdio_c -lz_log_adapter -lz_stdio_log -lz_format_util

#include "test_driver.h"

int main(int argc, char* argv[])
{	// parse options
	char buf[100];

	STRING_LITERAL_TO_STDOUT("starting main\n");

	STRING_LITERAL_TO_STDOUT("\ntests finished\n");
}

#endif

