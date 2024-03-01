#include "telephoto_grid.hpp"
#include "display_manager.hpp"
#include "sprite.hpp"
#include "css_SFML.hpp"
#include "voxelspace.hpp"

// #define TELEPHOTO_GRID_DRAW_TRACE 1

namespace isk {

static constexpr std::optional<std::partial_ordering> _guard_render_before(const telephoto_grid::coord_type& lhs, const telephoto_grid::coord_type& rhs)
{
	if (lhs == rhs) return std::partial_ordering::equivalent;
#if PROTOTYPE
	//	auto abs_delta = zaimoni::pos_diff(lhs, rhs); // don't have this built out
	zaimoni::math::vector<size_t, 3> abs_delta;
	abs_delta[0] = zaimoni::pos_diff(lhs[0], rhs[0]);
	abs_delta[1] = zaimoni::pos_diff(lhs[1], rhs[1]);
	abs_delta[2] = zaimoni::pos_diff(lhs[2], rhs[2]);
#endif
	if (lhs[2] == rhs[2]) {
		// same z-coordinate
		auto delta = lhs - rhs; // won't fit on screen *long* before this overflows
		if (0 <= delta[0] && 0 <= delta[1]) return std::partial_ordering::greater; // southeast quadrant
		if (0 >= delta[0] && 0 >= delta[1]) return std::partial_ordering::less;	   // northwest quadrant
		return std::partial_ordering::unordered;
	}
#if PROTOTYPE
	// need to verify these before actually using
	if (abs_delta[0] > abs_delta[2] && 2 <= abs_delta[0] - abs_delta[2]) return std::partial_ordering::unordered;
	if (abs_delta[1] > abs_delta[2] && 2 <= abs_delta[1] - abs_delta[2]) return std::partial_ordering::unordered;
#endif
	return std::nullopt;
}

static_assert(telephoto_grid::coord_type({0, 0, 0}) == telephoto_grid::coord_type({ 0, 0, 0 }));
static_assert(telephoto_grid::coord_type({ 0, 0, 0 }) != telephoto_grid::coord_type({ 0, 0, 1 }));
static_assert(_guard_render_before(telephoto_grid::coord_type({ 0, 0, 0 }), telephoto_grid::coord_type({ 0, 0, 0 })));
static_assert(std::partial_ordering::equivalent == *_guard_render_before(telephoto_grid::coord_type({ 0, 0, 0 }), telephoto_grid::coord_type({ 0, 0, 0 })));

static_assert(_guard_render_before(telephoto_grid::coord_type({ 0, 1, 0 }), telephoto_grid::coord_type({ 0, 0, 0 })));
static_assert(std::partial_ordering::greater == *_guard_render_before(telephoto_grid::coord_type({ 0, 1, 0 }), telephoto_grid::coord_type({ 0, 0, 0 })));
static_assert(_guard_render_before(telephoto_grid::coord_type({ 0, 0, 0 }), telephoto_grid::coord_type({ 0, 1, 0 })));
static_assert(std::partial_ordering::less == *_guard_render_before(telephoto_grid::coord_type({ 0, 0, 0 }), telephoto_grid::coord_type({ 0, 1, 0 })));
static_assert(_guard_render_before(telephoto_grid::coord_type({ 1, 0, 0 }), telephoto_grid::coord_type({ 0, 0, 0 })));
static_assert(std::partial_ordering::greater == *_guard_render_before(telephoto_grid::coord_type({ 1, 0, 0 }), telephoto_grid::coord_type({ 0, 0, 0 })));
static_assert(_guard_render_before(telephoto_grid::coord_type({ 0, 0, 0 }), telephoto_grid::coord_type({ 1, 0, 0 })));
static_assert(std::partial_ordering::less == *_guard_render_before(telephoto_grid::coord_type({ 0, 0, 0 }), telephoto_grid::coord_type({ 1, 0, 0 })));
static_assert(_guard_render_before(telephoto_grid::coord_type({ 0, -1, 0 }), telephoto_grid::coord_type({ 0, 0, 0 })));
static_assert(std::partial_ordering::less == *_guard_render_before(telephoto_grid::coord_type({ 0, -1, 0 }), telephoto_grid::coord_type({ 0, 0, 0 })));
static_assert(_guard_render_before(telephoto_grid::coord_type({ 0, 0, 0 }), telephoto_grid::coord_type({ 0, -1, 0 })));
static_assert(std::partial_ordering::greater == *_guard_render_before(telephoto_grid::coord_type({ 0, 0, 0 }), telephoto_grid::coord_type({ 0, -1, 0 })));
static_assert(_guard_render_before(telephoto_grid::coord_type({ -1, 0, 0 }), telephoto_grid::coord_type({ 0, 0, 0 })));
static_assert(std::partial_ordering::less == *_guard_render_before(telephoto_grid::coord_type({ -1, 0, 0 }), telephoto_grid::coord_type({ 0, 0, 0 })));
static_assert(_guard_render_before(telephoto_grid::coord_type({ 0, 0, 0 }), telephoto_grid::coord_type({ -1, 0, 0 })));
static_assert(std::partial_ordering::greater == *_guard_render_before(telephoto_grid::coord_type({ 0, 0, 0 }), telephoto_grid::coord_type({ -1, 0, 0 })));

static_assert(_guard_render_before(telephoto_grid::coord_type({ 1, 0, 0 }), telephoto_grid::coord_type({ 0, 1, 0 })));
static_assert(std::partial_ordering::unordered == *_guard_render_before(telephoto_grid::coord_type({ 1, 0, 0 }), telephoto_grid::coord_type({ 0, 1, 0 })));
static_assert(_guard_render_before(telephoto_grid::coord_type({ -1, 0, 0 }), telephoto_grid::coord_type({ 0, -1, 0 })));
static_assert(std::partial_ordering::unordered == *_guard_render_before(telephoto_grid::coord_type({ -1, 0, 0 }), telephoto_grid::coord_type({ 0, -1, 0 })));

static_assert(!_guard_render_before(telephoto_grid::coord_type({ 0, 0, 0 }), telephoto_grid::coord_type({ -1, -1, 1 })));

static constexpr std::partial_ordering _render_before(telephoto_grid::coord_type lhs, telephoto_grid::coord_type rhs)
{
//	static const constexpr telephoto_grid::coord_type z_adjust({ -1, -1, 1 });

	assert(lhs[2] < rhs[2]);
//	zaimoni::math::rearrange_diff(lhs, rhs); // not built out
	while (true) {
		zaimoni::math::rearrange_diff(lhs[0], rhs[0]);
		zaimoni::math::rearrange_diff(lhs[1], rhs[1]);
		zaimoni::math::rearrange_diff(lhs[2], rhs[2]);

		auto delta_z = zaimoni::pos_diff(lhs[2], rhs[2]);
		auto test = zaimoni::force_consteval<static_cast<size_t>(std::numeric_limits<ptrdiff_t>::max())>;
		if (std::numeric_limits<ptrdiff_t>::max() < delta_z) delta_z = std::numeric_limits<ptrdiff_t>::max();
		if (0 < rhs[0] && zaimoni::force_consteval<static_cast<size_t>(std::numeric_limits<ptrdiff_t>::max())> -delta_z < rhs[0]) delta_z = zaimoni::force_consteval<static_cast<size_t>(std::numeric_limits<ptrdiff_t>::max())> -rhs[0];
		if (0 < rhs[1] && zaimoni::force_consteval<static_cast<size_t>(std::numeric_limits<ptrdiff_t>::max())> - delta_z < rhs[1]) delta_z = zaimoni::force_consteval<static_cast<size_t>(std::numeric_limits<ptrdiff_t>::max())> - rhs[1];
		if (0 == delta_z) return std::partial_ordering::unordered;
	//	rhs -= (ptrdiff_t)delta_z * z_adjust; // not built out
		rhs[0] += delta_z;
		rhs[1] += delta_z;
		rhs[2] -= delta_z;
		if (const auto test = _guard_render_before(lhs, rhs)) {
			if (std::partial_ordering::equivalent == *test) return std::partial_ordering::less;
			return *test;
		}
	};
}

// failed test
// static_assert(std::partial_ordering::less == _render_before(telephoto_grid::coord_type({ 0, 0, 0 }), telephoto_grid::coord_type({ -1, -1, 1 })));

static std::partial_ordering render_before(const telephoto_grid::coord_type& lhs, const telephoto_grid::coord_type& rhs)
{
	if (const auto test = _guard_render_before(lhs, rhs)) return *test;
	if (lhs[2] < rhs[2]) return _render_before(lhs, rhs);
	// function extraction target
	const auto ret = _render_before(rhs, lhs);
	if (std::partial_ordering::less == ret) return std::partial_ordering::greater;
	if (std::partial_ordering::greater == ret) return std::partial_ordering::less;
	return ret;
	// end function extraction target
}

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

	auto i = l_span;
	do {
		ret.push_back({ -i+ x_delta, i, 0 });
	} while (0 <= --i);
	i = 0;
	while (++i < r_span) ret.push_back({ i+ x_delta, -i, 0 });

	return ret;
}

// returns true if and only if "modal" i.e. should not draw anything earlier in the stack
bool telephoto_grid::draw(const zaimoni::gdi::box<zaimoni::math::vector<int, 2> >& view_logical) {
#if	TELEPHOTO_GRID_DRAW_TRACE
	static bool have_drawn = false;
#endif

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
	// would like an associative container for this, but std::map and std::unordered_map are questionable
	constexpr const auto zero = pos{ 0, 0, 0 };
	std::vector<std::pair<pos, std::pair<zaimoni::math::vector<int, 2>, int> > > _canonical_to_screen;
	for (decltype(auto) x : _canonical_coordinates) {
		if (const auto test = screen_delta(zero, x)) _canonical_to_screen.push_back(std::pair(x, *test));
	}

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

	// count down to get drawing order (roughly)
	ptrdiff_t ub = image_keys.size();
	while (0 <= --ub) {
		decltype(auto) x = image_keys[ub];
		auto raw_img = DisplayManager::get().getTexture(x.second);
		css::wrap<isk::Sprite> staging(new isk::Sprite(raw_img));
		staging.CSS_class(CSS_tag);
		// ....
	}
#if TELEPHOTO_GRID_DRAW_TRACE
	have_drawn = true;
#endif
	return false;
}

}
