// celestial.cpp

#include "celestial.hpp"

namespace iskandria {

celestial_body::celestial_body(FILE* src)
{	// XXX
}

void celestial_body::save(FILE* dest)
{	// XXX
}

}	// namespace iskandria

#ifdef TEST_APP2
// fast compile test
// g++ -std=c++14 -otest.exe -Os -DTEST_APP2 celestial.cpp world_manager.cpp -Llib\host.isk -lz_stdio_c -lz_stdio_log
int main(int argc, char* argv[])
{
	iskandria::celestial_body::world_setup();
	return 0;
}
#endif
