#include "telephoto_grid.hpp"
#include "display_manager.hpp"

namespace isk {

void telephoto_grid::facing(iskandria::compass::XCOMlike f) {
	auto delta = iskandria::compass::inv_rotate(f, (iskandria::compass::XCOMlike)_facing);
	if (iskandria::compass::NW > delta && iskandria::compass::NE < delta)
		_facing = iskandria::compass::rotate((iskandria::compass::XCOMlike)_facing, (iskandria::compass::XCOMlike)(2 * (delta / 2)));
}

static std::vector<iskandria::grid::cartesian<3>::coord_type> NW_row(const int l_span, const int r_span, const int x_delta)
{
	assert(0 <= l_span);
	assert(0 <= r_span);
	std::vector<iskandria::grid::cartesian<3>::coord_type> ret;

	auto i = l_span - 1;
	do {
		ret.push_back({ -i+ x_delta, i, 0 });
	} while (0 <= --i);
	i = 0;
	while (++i <= r_span) ret.push_back({ i+ x_delta, -i, 0 });

	return ret;
}

// returns true if and only if "modal" i.e. should not draw anything earlier in the stack
bool telephoto_grid::draw(const zaimoni::gdi::box<zaimoni::math::vector<int, 2> >& view_logical) {
	// incoming dimensions are logical-screen dimensions
	// our origin will be "near the center of the screen"
	static constexpr const zaimoni::math::vector<int, 2> origin_offset = { DisplayManager::ISOMETRIC_TRIANGLE_WIDTH, DisplayManager::ISOMETRIC_TRIANGLE_HEIGHT };
	static constexpr const zaimoni::math::vector<int, 2> tile_span = { DisplayManager::ISOMETRIC_HEX_WIDTH, DisplayManager::ISOMETRIC_HEX_HEIGHT };
	auto origin_tl = (view_logical.br_c() - view_logical.tl_c()) / 2 + view_logical.tl_c() - origin_offset;
	// \todo we'd rather not recompute the "canonical coordinates" (to be rotated) about (0,0,0) in the z=0 plane
	std::vector<decltype(_map->_x)::coord_type> _canonical_row(NW_row((origin_tl[0] - view_logical.tl_c()[0]) / tile_span[0], (view_logical.br_c()[0] - origin_tl[0]) / tile_span[0], 0));
	origin_tl += origin_offset;
	std::vector<decltype(_map->_x)::coord_type> _canonical_next_row(NW_row((origin_tl[0] - view_logical.tl_c()[0]) / tile_span[0], (view_logical.br_c()[0] - origin_tl[0]) / tile_span[0], 0));
	std::vector<decltype(_map->_x)::coord_type> _canonical_coordinates;

	// \todo get tile data for rotated coordinates, in render order...we draw bottom-to-top but fetch top-to-bottom
	// we would rather reuse DOM elements
	return false;
}

}
