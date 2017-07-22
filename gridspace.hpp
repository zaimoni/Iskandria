// gridspace.hpp

#ifndef GRIDSPACE_HPP
#define GRIDSPACE_HPP

#include "object.hpp"
#include "coord_chart.hpp"

namespace iskandria {
namespace grid {

// requirements:
// * we must be able to state the position of a 2-d grid in the co-rotating coordinate system of a celestial object
// * we must be able to state the position of an agent or craft in a 2-d grid
// * we must be able to state the position of an agent in a craft

class cartesian_2d final : public isk::Object
{
public:
	typedef typename zaimoni::math::Cartesian_vector<ptrdiff_t,2>::coord_type coord_type;
private:
//	std::vector<std::weak_ptr<agent> > _agents;
//	std::vector<std::weak_ptr<craft> > _crafts;
	coord_type _lb;
	coord_type _ub;

	cartesian_2d(FILE* src);
public:
	cartesian_2d() = default;
	ZAIMONI_DEFAULT_COPY_DESTROY_ASSIGN(cartesian_2d);

	static std::weak_ptr<cartesian_2d> track(std::shared_ptr<cartesian_2d> src);

	// this has to integrate with the world manager
	static void world_setup();
	static std::weak_ptr<cartesian_2d> read_synthetic_id(FILE* src);
	static void write_synthetic_id(const std::shared_ptr<cartesian_2d>& src,FILE* dest);
private:
	void save(FILE* dest);
	static void update_all();
	static void gc_all();
	static void load_all(FILE* src);
	static void save_all(FILE* dest);

	static std::vector<std::shared_ptr<cartesian_2d> >& cache();
};

}	// namespace grid
}	// namespace iskandria

#endif
