// minkowski.cpp

#include "minkowski.hpp"

#include "world_manager.hpp"

namespace iskandria {

minkowski::minkowski(FILE* src)
{	// XXX
}

void minkowski::save(FILE* dest)
{	// XXX
}


std::vector<std::shared_ptr<minkowski> >& minkowski::cache()
{
	static std::vector<std::shared_ptr<minkowski> > ooao;
	return ooao;
}

std::weak_ptr<minkowski> minkowski::track(std::shared_ptr<minkowski> src)
{
	if (!src.get()) return std::weak_ptr<minkowski>();
	cache().push_back(src);
	return src;
}

void minkowski::world_setup()
{
	isk::WorldManager& cosmos = isk::WorldManager::get();
	cosmos.register_update(update_all);
	cosmos.register_gc(gc_all);
	cosmos.register_load(load_all);
	cosmos.register_save(save_all);
}

std::weak_ptr<minkowski> minkowski::read_synthetic_id(FILE* src)
{
	return isk::Object::read_synthetic_id(cache(), src);
}

void minkowski::write_synthetic_id(const std::shared_ptr<minkowski>& src,FILE* dest)
{
	isk::Object::write_synthetic_id(cache().size(), src, dest);
}

void minkowski::gc_all()	// XXX could go in header, but this *has* to exist enough to give a function pointer
{
	isk::Object::gc_all(cache());
}

void minkowski::load_all(FILE* src)
{
	size_t tmp;
	ZAIMONI_FREAD_OR_DIE(size_t,tmp,src)
	if (0==tmp) {
		cache().clear();
		return;
	}
	std::vector<std::shared_ptr<minkowski> > dest(tmp);
	while(0 < tmp--) dest.push_back(std::shared_ptr<minkowski>(new minkowski(src)));
	swap(dest,cache());
}


void minkowski::save_all(FILE* dest)
{
	gc_all();
	isk::Object::init_synthetic_ids(cache());
	size_t tmp = cache().size();
	ZAIMONI_FWRITE_OR_DIE(size_t,tmp,dest)
	for(auto i : cache()) i->save(dest);
}

void minkowski::update_all()
{
	std::vector<std::shared_ptr<minkowski> > staging;
	for(auto i : cache()) {
		minkowski* tmp = i.get();
		if (!tmp) continue;
		// just because it's going away doesn't mean it can't do things
		// do not insert into cache() here, insert to staging instead
	}
	if (!staging.empty()) cache().insert(cache().end(),staging.begin(),staging.end());
}

}	// namespace iskandria

#ifdef TEST_APP3
// fast compile test
// g++ -std=c++14 -otest.exe -Os -D__STDC_LIMIT_MACROS -DTEST_APP3 minkowski.cpp world_manager.cpp -Llib\host.isk -lz_stdio_c -lz_stdio_log
int main(int argc, char* argv[])
{
	iskandria::minkowski::world_setup();
	return 0;
}
#endif
