// gridspace.cpp

#include "gridspace.hpp"
#include "civilization.hpp"

#include "world_manager.hpp"

namespace iskandria {
namespace grid {


cartesian_2d::cartesian_2d(FILE* src)
{	// XXX
	zaimoni::read(_lb,src);
	zaimoni::read(_ub,src);
};

void cartesian_2d::save(FILE* dest)
{	// XXX
	zaimoni::write(_lb,dest);
	zaimoni::write(_ub,dest);
}


std::vector<std::shared_ptr<cartesian_2d> >& cartesian_2d::cache()
{
	static std::vector<std::shared_ptr<cartesian_2d> > ooao;
	return ooao;
}

std::weak_ptr<cartesian_2d> cartesian_2d::track(std::shared_ptr<cartesian_2d> src)
{
	if (!src.get()) return std::weak_ptr<cartesian_2d>();
	cache().push_back(src);
	return src;
}

void cartesian_2d::world_setup()
{
	isk::WorldManager& cosmos = isk::WorldManager::get();
	cosmos.register_update(update_all);
	cosmos.register_gc(gc_all);
	cosmos.register_load(load_all);
	cosmos.register_save(save_all);
}

std::weak_ptr<cartesian_2d> cartesian_2d::read_synthetic_id(FILE* src)
{
	return isk::Object::read_synthetic_id(cache(), src);
}

void cartesian_2d::write_synthetic_id(const std::shared_ptr<cartesian_2d>& src,FILE* dest)
{
	isk::Object::write_synthetic_id(cache().size(), src, dest);
}

void cartesian_2d::gc_all()	// XXX could go in header, but this *has* to exist enough to give a function pointer
{
	isk::Object::gc_all(cache());
}

void cartesian_2d::load_all(FILE* src)
{
	size_t tmp;
	ZAIMONI_FREAD_OR_DIE(size_t,tmp,src)
	if (0==tmp) {
		cache().clear();
		return;
	}
	std::vector<std::shared_ptr<cartesian_2d> > dest(tmp);
	while(0 < tmp--) dest.push_back(std::shared_ptr<cartesian_2d>(new cartesian_2d(src)));
	swap(dest,cache());
}


void cartesian_2d::save_all(FILE* dest)
{
	gc_all();
	isk::Object::init_synthetic_ids(cache());
	size_t tmp = cache().size();
	ZAIMONI_FWRITE_OR_DIE(size_t,tmp,dest)
	for(auto i : cache()) i->save(dest);
}

void cartesian_2d::update_all()
{
	std::vector<std::shared_ptr<cartesian_2d> > staging;
	for(auto i : cache()) {
		cartesian_2d* tmp = i.get();
		if (!tmp) continue;
		// just because it's going away doesn't mean it can't do things
		// do not insert into cache() here, insert to staging instead
	}
	if (!staging.empty()) cache().insert(cache().end(),staging.begin(),staging.end());
}

}	// namespace grid
}	// namespace iskandria

#ifdef TEST_APP3
// fast compile test
// g++ -std=c++14 -otest.exe -Os -D__STDC_LIMIT_MACROS -DTEST_APP3 gridspace.cpp agent.cpp craft.cpp world_manager.cpp -Llib\host.isk -lz_stdio_c -lz_stdio_log
int main(int argc, char* argv[])
{
	iskandria::grid::cartesian_2d::world_setup();
	return 0;
}
#endif
