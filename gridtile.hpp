#ifndef GRIDTILE_HPP
#define GRIDTILE_HPP 1

#include "XCOMlike.hpp"
#include "Zaimoni.STL/Logging.h"
#include "Zaimoni.STL/Pure.C/stdio_c.h"
#include "Zaimoni.STL/c_bitmap.hpp"
#include <string>
#include <vector>
#include <memory>

// \todo lift this somewhere relevant
// \todo eliminate when migrating to C++20
#define ZAIMONI_SIMULATE_RELOPS_WITH_ADL(TYPE)	\
	inline bool operator!=(const TYPE& lhs, const TYPE& rhs) { return !(lhs == rhs); }	\
	inline bool operator>(const TYPE& lhs, const TYPE& rhs) { return rhs < lhs; }	\
	inline bool operator<=(const TYPE& lhs, const TYPE& rhs) { return !(rhs < lhs); }	\
	inline bool operator>=(const TYPE& lhs, const TYPE& rhs) { return !(lhs < rhs); }

namespace zaimoni {
	class JSON;
}

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
class floor_model final
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

	explicit floor_model(const zaimoni::JSON& src);

	std::string image_key(iskandria::compass::XCOMlike f) {
		switch(f)
		{
		case iskandria::compass::NE: return _img_path_ne;
		case iskandria::compass::SE: return _img_path_se;
		case iskandria::compass::SW: return _img_path_sw;
#ifndef NDEBUG
		case iskandria::compass::N:
		case iskandria::compass::E:
		case iskandria::compass::S:
		case iskandria::compass::W: assert(0 && "orientation out of range");
#endif
		default: return _img_path_nw;
		}
	}

	friend bool operator==(const floor_model& lhs, const floor_model& rhs) { return lhs._id == rhs._id; }
	friend bool operator<(const floor_model& lhs, const floor_model& rhs) { return lhs._id < rhs._id; }

	static std::shared_ptr<floor_model> get(const std::string& id);
	static std::shared_ptr<floor_model> get(const zaimoni::JSON& src);
	static std::shared_ptr<floor_model> read_synthetic_id(FILE* src);
	static void write_synthetic_id(const std::shared_ptr<floor_model>& src, FILE* dest);

private:
	static std::vector<std::weak_ptr<floor_model> >& cache();
};

ZAIMONI_SIMULATE_RELOPS_WITH_ADL(floor_model)

class wall_model final
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

	explicit wall_model(const zaimoni::JSON& src);

	std::string image_key(iskandria::compass::XCOMlike f) {
		switch (f)
		{
		case iskandria::compass::N: return _img_path_outside_n;
		case iskandria::compass::S: return _img_path_inside_s;
		case iskandria::compass::E: return _img_path_inside_e;
#ifndef NDEBUG
		case iskandria::compass::NE:
		case iskandria::compass::SE:
		case iskandria::compass::SW:
		case iskandria::compass::NW: assert(0 && "orientation out of range");
#endif
		default: return _img_path_outside_w;
		}
	}

	friend bool operator==(const wall_model& lhs, const wall_model& rhs) { return lhs._id == rhs._id; }
	friend bool operator<(const wall_model& lhs, const wall_model& rhs) { return lhs._id < rhs._id; }

	static std::shared_ptr<wall_model> get(const std::string& id);
	static std::shared_ptr<wall_model> get(const zaimoni::JSON& src);
	static std::shared_ptr<wall_model> read_synthetic_id(FILE* src);
	static void write_synthetic_id(const std::shared_ptr<wall_model>& src, FILE* dest);

private:
	static std::vector<std::weak_ptr<wall_model> >& cache();
};

ZAIMONI_SIMULATE_RELOPS_WITH_ADL(wall_model)

// this will have to transcode to FILE*
class map_cell final
{
private:
	// raw pointers lose on tile loading
	std::shared_ptr<floor_model> _floor;	// empty/null is legal and corresponds to "none"
	std::shared_ptr<wall_model> _wall_w;
	std::shared_ptr<wall_model> _wall_n;
	typename zaimoni::bitmap<2>::type _flags;

public:
	map_cell() = default;
	~map_cell() = default;
	map_cell(const map_cell& src) = default;
	map_cell(map_cell&& src) = default;
	map_cell& operator=(const map_cell & src) = default;
	map_cell& operator=(map_cell&& src) = default;

	map_cell(FILE* src)
	: _floor(floor_model::read_synthetic_id(src)),
	  _wall_w(wall_model::read_synthetic_id(src)),
	  _wall_n(wall_model::read_synthetic_id(src)),
	  _flags(zaimoni::read<decltype(_flags)>(src)) {}

	void save(FILE* dest) const {
		floor_model::write_synthetic_id(_floor, dest);
		wall_model::write_synthetic_id(_wall_w, dest);
		wall_model::write_synthetic_id(_wall_n, dest);
		zaimoni::write(_flags, dest);
	}

	friend bool operator==(const map_cell& lhs, const map_cell& rhs);
	friend bool operator<(const map_cell& lhs, const map_cell& rhs);

	void set_floor(const std::string& id) { _floor = floor_model::get(id); }
	void set_n_wall(const std::string& id, bool reversed = false) {
		_wall_n = wall_model::get(id);
		_flags &= ~(1ULL << 0);
		_flags |= ((unsigned long long)reversed << 0);
	}
	void set_w_wall(const std::string& id, bool reversed = false) {
		_wall_w = wall_model::get(id);
		_flags &= ~(1ULL << 1);
		_flags |= ((unsigned long long)reversed << 1);
	}
	bool n_reversed() const { return _flags & (1ULL << 0); }
	bool w_reversed() const { return _flags & (1ULL << 1); }

	std::string floor(iskandria::compass::XCOMlike o) const {
#ifndef NDEBUG
		switch (o)
		{
		case iskandria::compass::N:
		case iskandria::compass::E:
		case iskandria::compass::S:
		case iskandria::compass::W:
			assert(0 && "orientation out of range");
		}
#endif
		if (_floor) return _floor->image_key(o);
		return std::string();
	}
	static constexpr bool n_wall_ok(iskandria::compass::XCOMlike o) {
		return iskandria::compass::NW == o || iskandria::compass::NE == o;
	}
	std::string n_wall(iskandria::compass::XCOMlike o) const {
#ifndef NDEBUG
		switch (o)
		{
		case iskandria::compass::N:
		case iskandria::compass::E:
		case iskandria::compass::S:
		case iskandria::compass::W:
			assert(0 && "orientation out of range");
		}
#endif
		switch (o)
		{
		case iskandria::compass::NW:
			if (_wall_n) return _wall_n->image_key(n_reversed() ? iskandria::compass::S : iskandria::compass::N);
			break;
		case iskandria::compass::NE:
			if (_wall_w) return _wall_w->image_key(w_reversed() ? iskandria::compass::S : iskandria::compass::N);
			break;
		}
		return std::string();
	}
	static constexpr bool w_wall_ok(iskandria::compass::XCOMlike o) {
		return iskandria::compass::NW == o || iskandria::compass::SW == o;
	}
	std::string w_wall(iskandria::compass::XCOMlike o) const {
#ifndef NDEBUG
		switch (o)
		{
		case iskandria::compass::N:
		case iskandria::compass::E:
		case iskandria::compass::S:
		case iskandria::compass::W:
			assert(0 && "orientation out of range");
		}
#endif
		switch (o)
		{
		case iskandria::compass::NW:
			if (_wall_w) return _wall_w->image_key(w_reversed() ? iskandria::compass::E : iskandria::compass::W);
			break;
		case iskandria::compass::SW:
			if (_wall_n) return _wall_n->image_key(n_reversed() ? iskandria::compass::E : iskandria::compass::W);
			break;
		}
		return std::string();
	}
	static constexpr bool want_e_cell(iskandria::compass::XCOMlike o) {
		return iskandria::compass::NE == o || iskandria::compass::SE == o;
	}
	static constexpr bool want_s_cell(iskandria::compass::XCOMlike o) {
		return iskandria::compass::SE == o || iskandria::compass::SW == o;
	}

	static constexpr bool n_wall_ok_e(iskandria::compass::XCOMlike o) { return iskandria::compass::NE == o; }
	std::string n_wall_e() const {
		// (src+E)-W wall, reversed -> N wall
		if (_wall_w) return _wall_w->image_key(w_reversed() ? iskandria::compass::N : iskandria::compass::S);
		return std::string();
	}

	static constexpr bool n_wall_ok_s(iskandria::compass::XCOMlike o) { return iskandria::compass::SE == o; }
	std::string n_wall_s() const {
		// (src+S)-N wall, reversed -> N wall
		if (_wall_n) return _wall_n->image_key(n_reversed() ? iskandria::compass::N : iskandria::compass::S);
		return std::string();
	}

	static constexpr bool w_wall_ok_e(iskandria::compass::XCOMlike o) { return iskandria::compass::SE == o; }
	std::string w_wall_e() const {
		// (src+E)-W wall, reversed -> W wall
		if (_wall_w) return _wall_w->image_key(w_reversed() ? iskandria::compass::W : iskandria::compass::E);
		return std::string();
	}

	static constexpr bool w_wall_ok_s(iskandria::compass::XCOMlike o) { return iskandria::compass::SW == o; }
	std::string w_wall_s() const {
		// (src+S)-N wall, reversed -> W wall
		if (_wall_n) return _wall_n->image_key(n_reversed() ? iskandria::compass::W : iskandria::compass::E);
		return std::string();
	}
};

ZAIMONI_SIMULATE_RELOPS_WITH_ADL(map_cell)

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

namespace zaimoni {

template<>
struct rw_mode<iskandria::grid::map_cell> {
	enum {
		group_write = 3,
		group_read = 3
	};
};

}	// namespace zaimoni

#endif
