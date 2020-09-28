#include "telephoto_grid.hpp"
#include "display_manager.hpp"
#include "sprite.hpp"
#include "css_SFML.hpp"
#include "voxelspace.hpp"

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
	using pos = decltype(_map->_x)::coord_type;
	auto origin_tl = (view_logical.br_c() - view_logical.tl_c()) / 2 + view_logical.tl_c() - origin_offset;
	// \todo we'd rather not recompute the "canonical coordinates" (to be rotated) about (0,0,0) in the z=0 plane
	const std::vector<pos> _canonical_row(NW_row((origin_tl[0] - view_logical.tl_c()[0]) / tile_span[0], (view_logical.br_c()[0] - origin_tl[0]) / tile_span[0], 0));
	auto origin_mid = origin_tl + origin_offset;
	const std::vector<pos> _canonical_next_row(NW_row((origin_mid[0] - view_logical.tl_c()[0]) / tile_span[0], (view_logical.br_c()[0] - origin_mid[0]) / tile_span[0], 1));
	auto y_abs_min = (origin_mid[1] - view_logical.tl_c()[1]) / tile_span[1];
	auto y_max = (view_logical.br_c()[1] - origin_tl[1]) / tile_span[1];
	std::vector<pos> _canonical_coordinates;
	{	// \todo? function target
	auto i = y_abs_min - 1;
	do {
		std::remove_reference_t<decltype(_canonical_row.front())> delta{ -i, i, 0 };
		auto y_ref = origin_tl[1] -i * tile_span[1];
		if (y_ref >= view_logical.tl_c()[1]) {
			for (decltype(auto) x : _canonical_row) { _canonical_coordinates.push_back(x + delta); }
		}
		y_ref += origin_offset[1];
		assert(y_ref >= view_logical.tl_c()[1]);
		for (decltype(auto) x : _canonical_next_row) { _canonical_coordinates.push_back(x + delta); }
	} while (0 <= --i);
	i = 0;
	while (++i <= y_max) {
		std::remove_reference_t<decltype(_canonical_row.front())> delta{ -i, i, 0 };
		auto y_ref = origin_tl[1] + i * tile_span[1];
		assert(y_ref >= view_logical.tl_c()[1]);
		for (decltype(auto) x : _canonical_row) { _canonical_coordinates.push_back(x + delta); }
		y_ref += origin_offset[1];
		if (y_ref >= view_logical.tl_c()[1]) {
			for (decltype(auto) x : _canonical_next_row) { _canonical_coordinates.push_back(x + delta); }
		}
	}
	}	// end function target

	// \todo get tile data for rotated coordinates, in render order...we draw bottom-to-top but fetch top-to-bottom
	std::vector<std::pair<pos, std::string> > image_keys;
	std::vector<pos> retread_coordinates;
	while (!_canonical_coordinates.empty()) {	// \todo working copy after this is cached
		const auto canon(std::move(_canonical_coordinates.back()));
		_canonical_coordinates.pop_back();
		decltype(auto) actual = canon + _origin;
		// \todo rotate coordinates as needed
		auto icons(_map->_x.render_tiles_isometric(actual, iskandria::compass::NW)); // \todo this needs to resolve to "underlying" maps as well
		if (!icons.empty()) {
			const auto start = icons.begin();
			auto iter = icons.end();
			do {
				--iter;
				// \todo check for view blocking
				image_keys.push_back(std::pair(actual, std::move(*iter)));
			} while (start != iter);
		}
		// prepare to recurse
		actual += {-1, -1 , -1};	// \todo rotate as needed
		if (_map->_x.contains(actual)) retread_coordinates.push_back(actual); // \todo this needs to allow resolving to "underlying" maps as well
	}
	// \todo reverse the image_keys setup to drawing order

	// we would rather reuse DOM elements
	static const std::string CSS_tag("telephoto_grid");	// should only be one of us (unlike some other imaginable camera types)
	auto prior_DOM = DisplayManager::get().remove_by_CSS_class(CSS_tag);

	for (decltype(auto) x : image_keys) {
		auto raw_img = DisplayManager::get().getTexture(x.second);
		css::wrap<isk::Sprite> staging(new isk::Sprite(raw_img));
		// ....
	}

	return false;
}

}
