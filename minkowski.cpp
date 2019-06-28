// minkowski.cpp

#include "minkowski.hpp"

#include <stdexcept>

namespace iskandria {

minkowski::minkowski(FILE* src)
{
	static_assert(sizeof(_system) == sizeof(unsigned char), "save/load broken");
	if (1!=fread(&_system, sizeof(_system), 1, src)) throw std::runtime_error("savefile truncated");
}

void minkowski::save(FILE* dest) const
{
	static_assert(sizeof(_system) == sizeof(unsigned char), "save/load broken");
	if (1 != fwrite(&_system, sizeof(_system), 1, dest)) throw std::runtime_error("savefile truncated");
}

}	// namespace iskandria

#ifdef TEST_APP3
// fast compile test
// g++ -std=c++14 -otest.exe -Os -D__STDC_LIMIT_MACROS -DTEST_APP3 minkowski.cpp -Llib\host.isk -lz_stdio_c -lz_stdio_log
int main(int argc, char* argv[])
{
	return 0;
}
#endif
