// gridspace.cpp

#include "gridspace.hpp"
#include "Zaimoni.STL/Pure.C/stdio_c.h"

namespace iskandria {
namespace grid {


cartesian_2d::cartesian_2d(FILE* src)
{	// XXX
	zaimoni::read(_lb,src);
	zaimoni::read(_ub,src);
}

void cartesian_2d::save(FILE* dest) const
{	// XXX
	zaimoni::write(_lb,dest);
	zaimoni::write(_ub,dest);
}

}	// namespace grid
}	// namespace iskandria

#ifdef TEST_APP3
// fast compile test
// g++ -std=c++14 -otest.exe -Os -D__STDC_LIMIT_MACROS -DTEST_APP3 gridspace.cpp agent.cpp craft.cpp world_manager.cpp -Llib\host.isk -lz_stdio_c -lz_stdio_log
int main(int argc, char* argv[])
{
	return 0;
}
#endif
