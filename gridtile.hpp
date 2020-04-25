#ifndef GRIDTILE_HPP
#define GRIDTILE_HPP 1

#include "gridspace.hpp"
#include "XCOMlike.hpp"

namespace iskandria {
namespace grid {

// Cf. XCOM. We have two types of tiles: floor, and wall.
// We want the isometric display mode to use approximately equilateral triangles; e.g. side length 8/height 7 or side length 15/height 13
// * clean tiling with 15/13: result image is 30x27
// We also want the side length to be even.
// The voxel count per unit side should be a power of 2 (e.g. 16)
// We are optimized for 3+1 Newtonian spacetime display

// anchor point for floor model is 0,0,0 x-y-z cartesian std orientation; unit box is (0,0,0),(16,16,16) i.e. legal coordinates 0..15

// VAPORWARE: both floor and tile data are to be loaded from hard drive (e.g., JSON format).
// the fact a tileset has been loaded is part of the savefile
// the reference data in the tileset is *not* part of the savefile.

// _img_path specifies a filepath or CGI transform instructions for DisplayManager
class floor_model
{
private:
	std::string _id;	// should be unique
	std::string _name;	// suitable for hovertext (fully identified)
	cartesian<3> _voxel_bounds;
	std::string _img_path_nw;	// reference view; viewpoint facing NW
	std::string _img_path_ne;	// viewpoint facing NE
	std::string _img_path_se;	// viewpoint facing SE
	std::string _img_path_sw;	// viewpoint facing SW
	// VAPORWARE ceiling tiles are just floor tiles viewed from below

	static std::vector<floor_model> _models;
public:
	enum reserved {
		NONE = 0
	};

	floor_model() = default;
	~floor_model() = default;
	floor_model(const floor_model& src) = default;
	floor_model(floor_model&& src) = default;
	floor_model& operator=(const floor_model& src) = default;
	floor_model& operator=(floor_model && src) = default;

	static const floor_model* get(const std::string& id) {
		for (const auto& x : _models) if (id == x._id) return &x;
		return 0;
	}
};

class wall_model
{
private:
	std::string _id;	// should be unique
	std::string _name;	// suitable for hovertext (fully identified)
	cartesian<3> _voxel_bounds;
	std::string _img_path_outside_w;	// reference orientation NW for these
	std::string _img_path_outside_n;
	std::string _img_path_inside_e;	// reference orientation SE for these
	std::string _img_path_inside_s;

	static std::vector<wall_model> _models;
public:
	enum reserved {
		NONE = 0
	};

	wall_model() = default;
	~wall_model() = default;
	wall_model(const wall_model& src) = default;
	wall_model(wall_model && src) = default;
	wall_model& operator=(const wall_model & src) = default;
	wall_model& operator=(wall_model && src) = default;

	static const wall_model* get(const std::string& id) {
		for (const auto& x : _models) if (id == x._id) return &x;
		return 0;
	}
};

// standard orientation is N (this does not line up with trigonometry)
// we generally want to be able to rotate about any coordinate plane, but default to xy plane (first two coordinates)
// we generally want to be able to do any non-interpolating rotation

enum {
	VOXEL_EDGE = 16
};

// offset code is
// 0: on-pixel exactly
// 1: +1/2 x
// 2: +1/2 y
// 3: +1/2 x,y
// for floating-point numerals, only need to use offset code 0
template<class T>
bool exact_rotate(const T& origin_x, const T& origin_y, unsigned int offset_code, iskandria::compass::XCOMlike new_facing, T& x, T& y)
{
	if (origin_x == x && origin_y == y && 0 == offset_code) return true;	// no-op
	switch (new_facing)
	{
	case iskandria::compass::N: return true;	// no-op
	case iskandria::compass::S: break;	// 180 degrees, always can handle
	case iskandria::compass::E:
	case iskandria::compass::W:
		if (0 == offset_code || 3 == offset_code) break;
		return false;
	default: return false;	// can't handle it
	}
	const bool low_x = (x <= origin_x);
	const bool low_y = (y <= origin_y);
	const bool x_half = (offset_code & 1);
	const bool y_half = (offset_code & 2);
	std::pair<T, T> abs_delta(low_x ? origin_x-x : x-origin_x-x_half ,low_y ? origin_y - y : y - origin_y - y_half);
	switch (new_facing)
	{
	case iskandria::compass::S:
		// invariant: x_half==y_half
		x = low_x ? origin_x + x_half + abs_delta.first : origin_x - abs_delta.first;
		y = low_y ? origin_y + y_half + abs_delta.second : origin_y - abs_delta.second;
		return true;
	case iskandria::compass::W:
		y = !low_x ? origin_y + y_half + abs_delta.first : origin_y - abs_delta.first;
		x = low_y ? origin_x + x_half + abs_delta.second : origin_x - abs_delta.second;
		return true;
	case iskandria::compass::E:
		y = low_x ? origin_y + y_half + abs_delta.first : origin_y - abs_delta.first;
		x = !low_y ? origin_x + x_half + abs_delta.second : origin_x - abs_delta.second;
		return true;
	default: return false;	// invariant error
	}
}

template<class T>
bool exact_reflect(const T& origin_x, const T& origin_y, unsigned int offset_code, iskandria::compass::XCOMlike new_facing, T& x, T& y)
{
	if (origin_x == x && origin_y == y && 0 == offset_code) return true;	// no-op
	// we're just interested in the reflection axis, not its orientation
	const bool low_x = (x <= origin_x);
	const bool low_y = (y <= origin_y);
	const bool x_half = (offset_code & 1);
	const bool y_half = (offset_code & 2);
	std::pair<T, T> abs_delta(low_x ? origin_x - x : x - origin_x - x_half, low_y ? origin_y - y : y - origin_y - y_half);
	switch (new_facing)
	{
	case iskandria::compass::N:		// y-axis
	case iskandria::compass::S:
		if (origin_y == y && !y_half) return true;	// no-op
		y = low_y ? origin_y+y_half+abs_delta.second : origin_y - abs_delta.second;
		return true;
	case iskandria::compass::E:	// x-axis
	case iskandria::compass::W:
		if (origin_x == x && !x_half) return true;	// no-op
		x = low_x ? origin_x + x_half + abs_delta.second : origin_x - abs_delta.second;
		return true;
	case iskandria::compass::NE:
	case iskandria::compass::SW:
		break;
	case iskandria::compass::NW:
	case iskandria::compass::SE:
		break;
	}
	return false;	// not handled
}

}	// namespace grid
}	// namespace iskandria
#endif
