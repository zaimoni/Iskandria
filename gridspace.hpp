// gridspace.hpp

#ifndef GRIDSPACE_HPP
#define GRIDSPACE_HPP

#include "coord_chart.hpp"

namespace iskandria {
namespace grid {

// requirements:
// * we must be able to state the position of a 2-d grid in the co-rotating coordinate system of a celestial object
// * we must be able to state the position of an agent or craft in a 2-d grid
// * we must be able to state the position of an agent in a craft

class cartesian_2d final
{
public:
	typedef typename zaimoni::math::Cartesian_vector<ptrdiff_t,2>::coord_type coord_type;
	typedef unsigned char orientation_type;
private:
//	std::vector<std::weak_ptr<agent> > _agents;
//	std::vector<std::weak_ptr<craft> > _crafts;
	coord_type _lb;
	coord_type _ub;

public:
	cartesian_2d() = default;
	cartesian_2d(FILE* src);
	ZAIMONI_DEFAULT_COPY_DESTROY_ASSIGN(cartesian_2d);
	void save(FILE* dest) const;
};

}	// namespace grid
}	// namespace iskandria

namespace zaimoni {
	template<>
	struct rw_mode<iskandria::grid::cartesian_2d>
	{
		enum {
			group_write = 3,
			group_read = 3
		};
	};
}

#endif
