#ifndef GRIDTILE_HPP
#define GRIDTILE_HPP 1

#include "XCOMlike.hpp"
#include "Zaimoni.STL/Logging.h"
#include "Zaimoni.STL/Pure.C/stdio_c.h"
#include <string>
#include <vector>
#include <memory>

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

// DATA DESIGN: while voxel bounds needs to be synchronized with Cartesian<3>, actually depending on Cartesian
// causes an include graph cycle. [also, the typical value is (1,1,1) so this should be sparse data]

// _img_path specifies a filepath or CGI transform instructions for DisplayManager
class floor_model
{
private:
	std::string _id;	// should be unique
	std::string _name;	// suitable for hovertext (fully identified)
	std::string _img_path_nw;	// reference view; viewpoint facing NW
	std::string _img_path_ne;	// viewpoint facing NE
	std::string _img_path_se;	// viewpoint facing SE
	std::string _img_path_sw;	// viewpoint facing SW
	// VAPORWARE ceiling tiles are just floor tiles viewed from below
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

	static auto get(const std::string& id) {
		decltype(auto) _models = cache();
		size_t ub = _models.size();
		while (0 < ub) {
			auto x = _models[--ub].lock();
			if (!x) {
				_models.erase(_models.begin() + ub);
				continue;
			}
			if (id == x->_id) return x;
		}
		return decltype(_models.front().lock())();
	}

	std::shared_ptr<floor_model> read_synthetic_id(FILE* src)
	{
		decltype(auto) _cache = cache();
		uintmax_t new_id = read_uintmax(_cache.size(), src);
		SUCCEED_OR_DIE(_cache.size() >= new_id);
		SUCCEED_OR_DIE(0 < new_id);
		return _cache[new_id - 1].lock();
	}

	void write_synthetic_id(const std::shared_ptr<floor_model>& src, FILE* dest)
	{
		decltype(auto) _cache = cache();
		const size_t ub = _cache.size();
		size_t i = ub;
		do  {
			decltype(auto) x = _cache[--i];
			if (x.lock() == src) {
				write_uintmax(ub, i, dest);
				return;
			}
		} while(0 < ub);
	}

private:
	static std::vector<std::weak_ptr<floor_model> >& cache()
	{
		static std::vector<std::weak_ptr<floor_model> > ooao;
		return ooao;
	}
};

class wall_model
{
private:
	std::string _id;	// should be unique
	std::string _name;	// suitable for hovertext (fully identified)
	std::string _img_path_outside_w;	// reference orientation NW for these
	std::string _img_path_outside_n;
	std::string _img_path_inside_e;	// reference orientation SE for these
	std::string _img_path_inside_s;

	static std::vector<std::weak_ptr<wall_model> > _models;
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

	static auto get(const std::string& id) {
		decltype(auto) _models = cache();
		size_t ub = _models.size();
		while (0 < ub) {
			auto x = _models[--ub].lock();
			if (!x) {
				_models.erase(_models.begin() + ub);
				continue;
			}
			if (id == x->_id) return x;
		}
		return decltype(_models.front().lock())();
	}

	std::shared_ptr<wall_model> read_synthetic_id(FILE* src)
	{
		decltype(auto) _cache = cache();
		uintmax_t new_id = read_uintmax(_cache.size(), src);
		SUCCEED_OR_DIE(_cache.size() >= new_id);
		SUCCEED_OR_DIE(0 < new_id);
		return _cache[new_id - 1].lock();
	}

	void write_synthetic_id(const std::shared_ptr<wall_model>& src, FILE* dest)
	{
		decltype(auto) _cache = cache();
		const size_t ub = _cache.size();
		size_t i = ub;
		do {
			decltype(auto) x = _cache[--i];
			if (x.lock() == src) {
				write_uintmax(ub, i, dest);
				return;
			}
		} while (0 < ub);
	}

private:
	static std::vector<std::weak_ptr<wall_model> >& cache()
	{
		static std::vector<std::weak_ptr<wall_model> > ooao;
		return ooao;
	}
};

// this will have to transcode to FILE*
class map_cell
{
private:
	// raw pointers lose on tile loading
	std::shared_ptr<floor_model> _floor;
	std::shared_ptr<wall_model> _wall_w;
	std::shared_ptr<wall_model> _wall_n;
public:
	map_cell() = default;
	~map_cell() = default;
	map_cell(const map_cell& src) = default;
	map_cell(map_cell&& src) = default;
	map_cell& operator=(const map_cell & src) = default;
	map_cell& operator=(map_cell&& src) = default;
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
