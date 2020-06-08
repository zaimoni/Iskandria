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
	telephoto_grid() noexcept : _facing(iskandria::compass::NW) {}
	telephoto_grid(const std::shared_ptr< isk::Wrap<iskandria::grid::cartesian<3> > >& view, const iskandria::grid::cartesian<3>::coord_type& o, iskandria::grid::cartesian<3>::orientation f) noexcept
		: _map(view), _origin(o), _facing(f) {}
	telephoto_grid(const telephoto_grid& src) = default;
	telephoto_grid(telephoto_grid&& src) = default;
	telephoto_grid& operator=(const telephoto_grid& src) = default;
	telephoto_grid& operator=(telephoto_grid&& src) = default;
	virtual ~telephoto_grid() = default;

	// may need to encode some sort of rotation scheme as well, so retain typecasting for now
	iskandria::compass::XCOMlike facing() const { return (iskandria::compass::XCOMlike)_facing; }
	void facing(iskandria::compass::XCOMlike f) {
		auto delta = iskandria::compass::inv_rotate(f, (iskandria::compass::XCOMlike)_facing);
		if (iskandria::compass::NW > delta && iskandria::compass::NE < delta)
			_facing = iskandria::compass::rotate((iskandria::compass::XCOMlike)_facing, (iskandria::compass::XCOMlike)(2*(delta/2)));
	}

	// returns true if and only if "modal" i.e. should not draw anything earlier in the stack
	bool draw(const zaimoni::gdi::box<zaimoni::math::vector<int, 2> >& view_logical) override {
		// incoming dimensions are logical-screen dimensions
		// our origin will be "near the center of the screen"
		static constexpr const zaimoni::math::vector<int, 2> origin_offset = { DisplayManager::ISOMETRIC_TRIANGLE_WIDTH, DisplayManager::ISOMETRIC_TRIANGLE_HEIGHT };
		static constexpr const zaimoni::math::vector<int, 2> tile_span = { DisplayManager::ISOMETRIC_HEX_WIDTH, DisplayManager::ISOMETRIC_HEX_HEIGHT };
		auto origin_tl = (view_logical.br_c() - view_logical.tl_c()) / 2 + view_logical.tl_c() - origin_offset;
		// \todo we'd rather not recompute the "canonical coordinates" (to be rotated) about (0,0,0) in the z=0 plane
		// \todo get tile data for rotated coordinates, in render order
		return false;
	}
};

}

#endif
