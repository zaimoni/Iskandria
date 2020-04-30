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
		{
		const auto& _cache = cache();
		auto& _staged = staging();
		auto c_end = _cache.end();
		auto s_end = _staged.end();
		if (   s_end == std::find(_staged, _staged.begin(), s_end, src)
			&& c_end == std::find(_cache, _cache.begin(), c_end, src))
			_staged.push_back(src);
		}
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

	static std::vector<std::shared_ptr<Wrap> >& staging()
	{
		static std::vector<std::shared_ptr<Wrap> > ooao;
		return ooao;
	}

	static void take_live()
	{
		auto& staged = staging();
		if (!staged.empty()) {
			{
			auto& _cache = cache();
			_cache.insert(_cache.end(), staged.begin(), staged.end());
			}
			typename std::remove_reference<decltype(staged)>::type discard;
			swap(staged, discard);
		}
	}

	// save

	static void update_all()
	{
		take_live();
		for (auto i : cache()) {
			Wrap* tmp = i.get();
			if (!tmp) continue;
			// just because it's going away doesn't mean it can't do things
			// do not insert into cache() here, insert to staging instead
		}
		take_live();
	}

	static void gc_all() { isk::Object::gc_all(cache()); }

	static void load_all(FILE* src)
	{
		{
		typename std::remove_reference<decltype(staging())>::type discard;
		swap(staging(), discard);
		}
		auto& _cache = cache();
		{
		typename std::remove_reference<decltype(staging())>::type discard;
		swap(_cache, discard);
		}

		size_t tmp;
		ZAIMONI_FREAD_OR_DIE(size_t, tmp, src)
		if (0 == tmp) return;

		std::vector<std::shared_ptr<Wrap> > dest;
		while (0 < tmp--) dest.push_back(std::shared_ptr<Wrap>(new Wrap(zaimoni::read<T>(src))));
		swap(dest, cache());
	}

	static void save_all(FILE* dest)
	{
		gc_all();
		take_live();
		auto& _cache = cache();

		isk::Object::init_synthetic_ids(_cache);
		size_t tmp = _cache.size();
		ZAIMONI_FWRITE_OR_DIE(size_t, tmp, dest)
		for (auto& i : _cache) zaimoni::write(i->_x,dest);
	}
};

}	// namespace isk

#endif
