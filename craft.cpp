// craft.cpp

#include "craft.hpp"
#include "civilization.hpp"
#include "Zaimoni.STL/Compiler.h"
#include "Zaimoni.STL/Pure.C/stdio_c.h"

namespace iskandria {

#define RAILSHOTGUN "railshotgun"
#define RAILSHOTGUN_DESC "A railgun whose shots spread like a shotgun."

#define UV_LASER "ultraviolet laser"
#define UV_LASER_DESC "Designed to self-interfere at about twice its effective range in vacuum."

#define WASP_II "Wasp II"
#define WASP_II_DESC "Space fighter"

#define AMBLER_I "Ambler I"
#define AMBLER_I_DESC "Self-propelled heavy weapon mount."

static const craft_model _models[4] = {
	{CIV_NONE, DRIVE_ALT_AZIMUTH_MOUNT, RAILSHOTGUN, sizeof(RAILSHOTGUN)-1, RAILSHOTGUN_DESC, sizeof(RAILSHOTGUN_DESC)-1},
	{CIV_NONE, DRIVE_ALT_AZIMUTH_MOUNT, UV_LASER, sizeof(UV_LASER)-1, UV_LASER_DESC, sizeof(UV_LASER_DESC)-1},
	{CIV_TZARZ, DRIVE_RHO, WASP_II, sizeof(WASP_II)-1, WASP_II_DESC, sizeof(WASP_II_DESC)-1},
	{CIV_ISKANDRAN, DRIVE_TRACKS, AMBLER_I, sizeof(AMBLER_I)-1, AMBLER_I_DESC, sizeof(AMBLER_I_DESC)-1}
};

const craft_model* craft::models = _models;
const size_t craft::models_len = STATIC_SIZE(_models);

craft::craft(FILE* src)
{
	_model_index = zaimoni::read<decltype(_model_index)>(src,models_len);
}

void craft::save(FILE* dest) const
{
	zaimoni::write(_model_index,dest,models_len);
}

}	// namespace iskandria

#ifdef TEST_APP2
// fast compile test
// g++ -std=c++11 -otest.exe -Os -D__STDC_LIMIT_MACROS -DTEST_APP2 craft.cpp -Llib\host.isk -lz_stdio_c -lz_stdio_log
int main(int argc, char* argv[])
{
	return 0;
}
#endif
