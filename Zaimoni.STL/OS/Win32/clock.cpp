// clock.cpp

#include "../clock.hpp"
#include <windows.h>

namespace zaimoni {

const unsigned int Clock::scale = 1000;	// how many units per second

unsigned long Clock::n_seconds()
{
	return timeGetTime();
}

}

#ifdef TEST_APP
// fast compile test; doesn't exercise anything
// g++ -oclock.exe -DTEST_APP clock.cpp -lwinmm
int main(int argc, char* argv[])
{
	zaimoni::Clock test;
}
#endif
