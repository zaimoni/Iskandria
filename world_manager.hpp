// world_manager.hpp

#ifndef WORLD_MANAGER_HPP
#define WORLD_MANAGER_HPP 1

#include <stdio.h>
#include <vector>
#include <memory>

#include "world_view.hpp"
#include "singleton.on.hpp"

namespace isk {

// Cf. http://dragonfly.wpi.edu/book/
// in general, for each type of game object of interest
// * global registry : best by type
// * ability to iterate efficiently: begin(), end(), size(), empty(), possibly c_begin(), c_end() : best by type
// * ability to load/save : handlers ok
// * ability to garbage-collect : handler ok

class WorldManager
{
	ISK_SINGLETON_HEADER_DEFAULT_CONSTRUCTOR_DESTRUCTOR(WorldManager);
	typedef void (*file_handler)(FILE*);
	typedef void (*gc_handler)();
	// want one set of these for each type of game object
private:
	std::vector<gc_handler> _update_handlers;
	std::vector<gc_handler> _gc_handlers;
	std::vector<file_handler> _load_handlers;
	std::vector<file_handler> _save_handlers;
	std::vector<std::weak_ptr<WorldView> > _cameras;
public:
	void update();
	void gc();	// request removing all dead objects

	// also responsible for load/save; C error code convention
	int load(FILE* src);
	int save(FILE* dest);

	void track(const std::shared_ptr<WorldView>& src) { if (src) _cameras.push_back(src); }

	// not so mechanical
	void draw();

	// these typically accept static member functions
	void register_update(gc_handler src);
	void register_gc(gc_handler src);
	void register_load(file_handler src);
	void register_save(file_handler src);
};

}	// namespace isk

#include "singleton.off.hpp"

#endif
