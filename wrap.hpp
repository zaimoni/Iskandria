#ifndef WRAP_HPP
#define WRAP_HPP 1

#include "object.hpp"
#include "world_manager.hpp"

namespace isk {

template<class T>
class Wrap : public Object
{
public:
	T _x;	// the type being enabled for the ECS simulation

	Wrap() = default;
	Wrap(const Wrap& src) = default;
	Wrap(Wrap&& src) = default;
	Wrap(const T& src) : _x(src) {};
	Wrap(T&& src) : _x(std::move(src)) {};
	~Wrap() = default;
	Wrap& operator=(const Wrap& src) = default;
	Wrap& operator=(Wrap&& src) = default;

	static std::weak_ptr<Wrap> track(const std::shared_ptr<Wrap>& src)
	{
		if (!src.get()) return std::weak_ptr<Wrap>();
		cache().push_back(src);
		return src;
	}

	static void world_setup()
	{
		isk::WorldManager& cosmos = isk::WorldManager::get();
		cosmos.register_update(update_all);
		cosmos.register_gc(gc_all);
		cosmos.register_load(load_all);
		cosmos.register_save(save_all);
	}

	std::weak_ptr<Wrap> read_synthetic_id(FILE* src)
	{
		return isk::Object::read_synthetic_id(cache(), src);
	}

	void write_synthetic_id(const std::shared_ptr<Wrap>& src, FILE* dest)
	{
		isk::Object::write_synthetic_id(cache().size(), src, dest);
	}

private:
	static std::vector<std::shared_ptr<Wrap> >& cache()
	{
		static std::vector<std::shared_ptr<Wrap> > ooao;
		return ooao;
	}

	// save

	static void update_all()
	{
		std::vector<std::shared_ptr<Wrap> > staging;
		for (auto i : cache()) {
			Wrap* tmp = i.get();
			if (!tmp) continue;
			// just because it's going away doesn't mean it can't do things
			// do not insert into cache() here, insert to staging instead
		}
		if (!staging.empty()) cache().insert(cache().end(), staging.begin(), staging.end());
	}

	static void gc_all() { isk::Object::gc_all(cache()); }

	static void load_all(FILE* src)
	{
		size_t tmp;
		ZAIMONI_FREAD_OR_DIE(size_t, tmp, src)
		if (0 == tmp) {
			cache().clear();
			return;
		}
		std::vector<std::shared_ptr<Wrap> > dest(tmp);
		while (0 < tmp--) dest.push_back(std::shared_ptr<Wrap>(zaimoni::read<T*>(src)));
		swap(dest, cache());
	}

	static void save_all(FILE* dest)
	{
		gc_all();
		isk::Object::init_synthetic_ids(cache());
		size_t tmp = cache().size();
		ZAIMONI_FWRITE_OR_DIE(size_t, tmp, dest)
		for (auto i : cache()) zaimoni::write(*i,dest);
	}

};

}	// namespace isk

#endif
