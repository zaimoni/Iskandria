// object.hpp

#ifndef OBJECT_HPP
#define OBJECT_HPP 1

#include <stddef.h>
#include <vector>
#include <memory>
#include <type_traits>

#include "Zaimoni.STL/Logging.h"
#include "Zaimoni.STL/Pure.C/stdio_c.h"

namespace isk {

// used as a base class for those objects meant to be managed by isk::WorldManager.  Expected API to support is
/*
	static std::weak_ptr<celestial_body> register(celestial_body* src);
	static std::weak_ptr<celestial_body> register(std::shared_ptr<celestial_body> src);

	// this has to integrate with the world manager	
	static void world_setup();
	static std::weak_ptr<celestial_body> read_synthetic_id(FILE* src);
	static void write_synthetic_id(std::shared_ptr<celestial_body>,FILE* dest);
private:
	save(FILE* dest);
	static void update_all();
	static void gc_all();
	static load_all(FILE* src);;
	static save_all(FILE* dest);
	static void init_synthetic_ids();	// before save

	static std::vector<std::shared_ptr<celestial_body> > cache();
*/
// that is, we know we have to support std::vector<std::shared_ptr<Derived> >
struct Object
{
protected:
	ptrdiff_t _synthetic_id;	// XXX overflows when C array dereferencing fails.
	size_t _bitmap;
public:
	void id(ptrdiff_t src) { _synthetic_id = src; }
	ptrdiff_t id() const {return _synthetic_id;};

	void gc_this(bool gc) { if (gc) _bitmap &= 1U; else _bitmap &= ~(1U); };
	bool gc_this() const { return _bitmap & 1U;};

protected:
	template<class Derived>
	static typename std::enable_if<std::is_base_of<Object,Derived>::value, void>::type init_synthetic_ids(std::vector<std::shared_ptr<Derived> >& _cache)
	{
		ptrdiff_t new_id = 0;
		for(auto i : _cache) {
			assert(i.get());	// should have been garbage-collected first
			i->id(++new_id);
		}
	}

	template<class Derived>
	static typename std::enable_if<std::is_base_of<Object,Derived>::value, void>::type gc_all(std::vector<std::shared_ptr<Derived> >& _cache)
	{
		std::vector<std::shared_ptr<Derived> > dest(_cache.size());
		size_t n = 0;
		for(auto i : _cache) {
			const Derived* const tmp = i.get();
			if (!tmp) continue;
			if (tmp->gc_this()) {
				if (1 >= i.use_count()) continue;
				i->gc_this(false);	// not allowed to GC multiple-owner objects
			}
			dest[n++] = i;
		}
		if (n < dest.size()) {
			dest.resize(n);
			dest.shrink_to_fit();	// XXX non-binding request
		}
		swap(_cache,dest);
	}

	template<class Derived>
	static typename std::enable_if<std::is_base_of<Object,Derived>::value, std::weak_ptr<Derived> >::type read_synthetic_id(const std::vector<std::shared_ptr<Derived> >& _cache, FILE* src)
	{
		uintmax_t new_id = read_uintmax(_cache.size(),src);
		SUCCEED_OR_DIE(_cache.size()>=new_id);
		SUCCEED_OR_DIE(0<new_id);
		return _cache[new_id-1];
	}

	template<class Derived>
	static typename std::enable_if<std::is_base_of<Object,Derived>::value, void >::type write_synthetic_id(const size_t ub, const std::shared_ptr<Derived>& src, FILE* dest)
	{
		assert(src.get());
		write_uintmax(ub,src.get()->id(),dest);
	}
};

}	// namnespace isk

namespace zaimoni {

template<class Derived>
typename std::enable_if<std::is_base_of<isk::Object, Derived>::value, std::weak_ptr<Derived> >::type  read(FILE* src)
{
	return Derived::read_synthetic_id(src);
}

template<class Derived>
typename std::enable_if<std::is_base_of<isk::Object, Derived>::value, void >::type
write(const std::weak_ptr<Derived>& src, FILE* dest)
{
	Derived::write_synthetic_id(src.lock(), dest);
}

template<class Derived>
typename std::enable_if<std::is_base_of<isk::Object, Derived>::value, void >::type
write(const std::shared_ptr<Derived>& src, FILE* dest)
{
	Derived::write_synthetic_id(src, dest);
}

}	// namespace zaimoni

#endif
