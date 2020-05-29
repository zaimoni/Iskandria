#ifndef TELEPHOTO_GRID_HPP
#define TELEPHOTO_GRID_HPP 1

#include "world_view.hpp"
#include "gridspace.hpp"
#include "wrap.hpp"

namespace isk {

// the classic XCOM-like isometric camera view
class telephoto_grid : public WorldView
{
	std::shared_ptr< isk::Wrap<iskandria::grid::cartesian<3> > > _map;
	iskandria::grid::cartesian<3>::coord_type _origin;
	iskandria::grid::cartesian<3>::orientation _facing;
public:
	telephoto_grid() = default;
	telephoto_grid(const std::shared_ptr< isk::Wrap<iskandria::grid::cartesian<3> > >& view, const iskandria::grid::cartesian<3>::coord_type& o, iskandria::grid::cartesian<3>::orientation f)
		: _map(view), _origin(o), _facing(f) {}
	telephoto_grid(const telephoto_grid& src) = default;
	telephoto_grid(telephoto_grid&& src) = default;
	telephoto_grid& operator=(const telephoto_grid& src) = default;
	telephoto_grid& operator=(telephoto_grid&& src) = default;
	virtual ~telephoto_grid() = default;

	// returns true if and only if "modal" i.e. should not draw anything earlier in the stack
	bool draw(const zaimoni::gdi::box<zaimoni::math::vector<int, 2> >& view_logical) override { return false; }
};

}

#endif
