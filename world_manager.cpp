// world_manager.cpp

#include "world_manager.hpp"
#include "display_manager.hpp"

#include "singleton.on.hpp"

namespace isk {

ISK_SINGLETON_BODY(WorldManager)

void WorldManager::update()
{
	if (_update_handlers.empty()) return;
	gc();
	for(auto& x : _update_handlers) x();
}

void WorldManager::gc()
{
	if (_gc_handlers.empty()) return;
	for(auto& x : _gc_handlers) x();
}

int WorldManager::load(FILE* src)
{
	if (!src) return -1;
	if (_load_handlers.empty()) return -1;
	for(auto& x : _load_handlers) x(src);
	return 0;
}

int WorldManager::save(FILE* dest)
{
	if (!dest) return -1;
	if (_save_handlers.empty()) return -1;
	gc();
	for(auto& x : _save_handlers) x(dest);
	return 0;
}

/*
	Want the definition to be visible in grep so just the function body here
*/
#define REGISTER_BODY(DEST)	\
	if (!src) return;	\
	if (!DEST.empty())	{	\
		auto fail = DEST.end();	\
		if (fail != find(DEST.begin(),fail,src)) return;	\
	}	\
	DEST.push_back(src)

// these typically accept static member functions
void WorldManager::register_update(gc_handler src)
{
	REGISTER_BODY(_update_handlers);
}

void WorldManager::register_gc(gc_handler src)
{
	REGISTER_BODY(_gc_handlers);
}

void WorldManager::register_load(file_handler src)
{
	REGISTER_BODY(_load_handlers);
}

void WorldManager::register_save(file_handler src)
{
	REGISTER_BODY(_save_handlers);
}

#undef REGISTER_BODY

void WorldManager::draw()
{
	auto ub = _cameras.size();
	if (0 >= ub) return;
	auto clip = DisplayManager::get().clip_rect_logical();
	// countdown loop theoretically allows pushing on new world views within a world view safely,
	// not just realtime cleanup
	do {
		if (auto view = _cameras[--ub].lock()) {
			if (view->draw(clip)) break;
		} else {
			_cameras.erase(_cameras.begin() + ub);
		}
	} while (0 < ub);
}

}	// namespace isk

#ifdef TEST_APP
// fast compile test
// g++ -std=c++11 -otest.exe -Os -DTEST_APP world_manager.cpp
int main(int argc, char* argv[])
{
	isk::WorldManager::get();
	return 0;
}
#endif
