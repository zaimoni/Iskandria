// celestial.cpp

#include "celestial.hpp"

#include "world_manager.hpp"

namespace iskandria {

celestial_body::celestial_body(FILE* src)
{	// XXX
};

void celestial_body::save(FILE* dest)
{	// XXX
}


std::vector<std::shared_ptr<celestial_body> >& celestial_body::cache()
{
	static std::vector<std::shared_ptr<celestial_body> > ooao;
	return ooao;
}

std::weak_ptr<celestial_body> celestial_body::track(std::shared_ptr<celestial_body> src)
{
	if (!src.get()) return std::weak_ptr<celestial_body>();
	cache().push_back(src);
	return src;
}

void celestial_body::world_setup()
{
	isk::WorldManager& cosmos = isk::WorldManager::get();
	cosmos.register_update(update_all);
	cosmos.register_gc(gc_all);
	cosmos.register_load(load_all);
	cosmos.register_save(save_all);
}

std::weak_ptr<celestial_body> celestial_body::read_synthetic_id(FILE* src)
{
	return isk::Object::read_synthetic_id(cache(), src);
}

void celestial_body::write_synthetic_id(const std::shared_ptr<celestial_body>& src,FILE* dest)
{
	isk::Object::write_synthetic_id(cache().size(), src, dest);
}

void celestial_body::gc_all()	// XXX could go in header, but this *has* to exist enough to give a function pointer
{
	isk::Object::gc_all(cache());	
}

void celestial_body::load_all(FILE* src)
{
	size_t tmp;
	ZAIMONI_FREAD_OR_DIE(size_t,tmp,src)
	if (0==tmp) {
		cache().clear();
		return;
	}
	std::vector<std::shared_ptr<celestial_body> > dest(tmp);
	while(0 < tmp--) dest.push_back(std::shared_ptr<celestial_body>(new celestial_body(src)));
	swap(dest,cache());
}


void celestial_body::save_all(FILE* dest)
{
	gc_all();
	isk::Object::init_synthetic_ids(cache());
	size_t tmp = cache().size();
	ZAIMONI_FWRITE_OR_DIE(size_t,tmp,dest)
	for(auto i : cache()) i->save(dest);
}

void celestial_body::update_all()
{
	std::vector<std::shared_ptr<celestial_body> > staging;
	for(auto i : cache()) {
		celestial_body* tmp = i.get();
		if (!tmp) continue;
		// just because it's going away doesn't mean it can't gravitate, etc.
		// cache() must not be modified in this loop; use staging for that
		// ....
	}
	if (!staging.empty()) cache().insert(cache().end(),staging.begin(),staging.end());
}

}	// namespace iskandria

#ifdef TEST_APP2
// fast compile test
// g++ -std=c++11 -otest.exe -Os -DTEST_APP2 celestial.cpp world_manager.cpp -Llib\host.isk -lz_stdio_c -lz_stdio_log
int main(int argc, char* argv[])
{
	iskandria::celestial_body::world_setup();
	return 0;
}
#endif
