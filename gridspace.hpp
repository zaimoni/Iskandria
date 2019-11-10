// gridspace.hpp

#ifndef GRIDSPACE_HPP
#define GRIDSPACE_HPP

#include "matrix.hpp"
#include "Zaimoni.STL/GDI/box.hpp"
#include "Zaimoni.STL/Pure.C/stdio_c.h"

namespace iskandria {
namespace grid {

// requirements:
// * we must be able to state the position of a 2-d grid in the co-rotating coordinate system of a celestial object
// * we must be able to state the position of an agent or craft in a 2-d grid
// * we must be able to state the position of an agent in a craft

template<size_t N>
class cartesian final
{
	static_assert(2 <= N);
public:
	typedef typename zaimoni::math::vector<ptrdiff_t,N> coord_type;
	typedef unsigned char orientation_type;
private:
//	std::vector<std::weak_ptr<agent> > _agents;
//	std::vector<std::weak_ptr<craft> > _crafts;
	zaimoni::gdi::box<coord_type> _domain;
public:
	cartesian() = default;
	cartesian(FILE* src)
	{
		zaimoni::read(_domain.tl_c(), src);
		zaimoni::read(_domain.br_c(), src);
	}

	ZAIMONI_DEFAULT_COPY_DESTROY_ASSIGN(cartesian);
	void save(FILE* dest) const
	{
		zaimoni::write(_domain.tl_c(), dest);
		zaimoni::write(_domain.br_c(), dest);
	}
};

}	// namespace grid
}	// namespace iskandria

namespace zaimoni {
	template<size_t N>
	struct rw_mode<iskandria::grid::cartesian<N> >
	{
		enum {
			group_write = 3,
			group_read = 3
		};
	};
}

#endif
