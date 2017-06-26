// craft.cpp

#include "craft.hpp"
#include "civilization.hpp"

#include "world_manager.hpp"

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

static const craft_model* models = _models;
static const size_t models_len = STATIC_SIZE(_models);


craft::craft(FILE* src)
{	// XXX
};

void craft::save(FILE* dest)
{	// XXX
}


std::vector<std::shared_ptr<craft> >& craft::cache()
{
	static std::vector<std::shared_ptr<craft> > ooao;
	return ooao;
}

std::weak_ptr<craft> craft::track(std::shared_ptr<craft> src)
{
	if (!src.get()) return std::weak_ptr<craft>();
	cache().push_back(src);
	return src;
}

void craft::world_setup()
{
	isk::WorldManager& cosmos = isk::WorldManager::get();
	cosmos.register_update(update_all);
	cosmos.register_gc(gc_all);
	cosmos.register_load(load_all);
	cosmos.register_save(save_all);
}

std::weak_ptr<craft> craft::read_synthetic_id(FILE* src)
{
	return isk::Object::read_synthetic_id(cache(), src);
}

void craft::write_synthetic_id(const std::shared_ptr<craft>& src,FILE* dest)
{
	isk::Object::write_synthetic_id(cache().size(), src, dest);
}

void craft::gc_all()	// XXX could go in header, but this *has* to exist enough to give a function pointer
{
	isk::Object::gc_all(cache());
}

void craft::load_all(FILE* src)
{
	size_t tmp;
	ZAIMONI_FREAD_OR_DIE(size_t,tmp,src)
	if (0==tmp) {
		cache().clear();
		return;
	}
	std::vector<std::shared_ptr<craft> > dest(tmp);
	while(0 < tmp--) dest.push_back(std::shared_ptr<craft>(new craft(src)));
	swap(dest,cache());
}


void craft::save_all(FILE* dest)
{
	gc_all();
	isk::Object::init_synthetic_ids(cache());
	size_t tmp = cache().size();
	ZAIMONI_FWRITE_OR_DIE(size_t,tmp,dest)
	for(auto i : cache()) i->save(dest);
}

void craft::update_all()
{
	std::vector<std::shared_ptr<craft> > staging;
	for(auto i : cache()) {
		craft* tmp = i.get();
		if (!tmp) continue;
		// just because it's going away doesn't mean it can't do things
		// do not insert into cache() here, insert to staging instead
	}
	if (!staging.empty()) cache().insert(cache().end(),staging.begin(),staging.end());
}

}	// namespace iskandria

#ifdef TEST_APP2
// fast compile test
// g++ -std=c++11 -otest.exe -Os -DTEST_APP2 craft.cpp world_manager.cpp -Llib\host.isk -lz_stdio_c -lz_stdio_log
int main(int argc, char* argv[])
{
	iskandria::craft::world_setup();
	return 0;
}
#endif
