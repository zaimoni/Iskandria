#ifndef GRIDTILE_HPP
#define GRIDTILE_HPP 1

#include "gridspace.hpp"
#include "XCOMlike.hpp"

namespace iskandria {
namespace grid {

// Cf. XCOM. We have two types of tiles: floor, and wall.
// We want the isometric display mode to use approximately equilateral triangles; e.g. side length 8/height 7 or side length 15/height 13
// We also want the side length to be even.
// The voxel count per unit side should be a power of 2 (e.g. 16)
// We are optimized for 3+1 Newtonian spacetime display

// anchor point for floor model is 0,0,0 x-y-z cartesian std orientation; unit box is (0,0,0),(16,16,16) i.e. legal coordinates 0..15

class floor_model
{
private:
	std::string _name;
	cartesian<3> _voxel_bounds;
public:
	floor_model() = default;
	~floor_model() = default;
	floor_model(const floor_model& src) = default;
	floor_model(floor_model&& src) = default;
	floor_model& operator=(const floor_model& src) = default;
	floor_model& operator=(floor_model && src) = default;
};

class wall_model
{
private:
	std::string _name;
	cartesian<3> _voxel_bounds;
public:
	wall_model() = default;
	~wall_model() = default;
	wall_model(const wall_model& src) = default;
	wall_model(wall_model && src) = default;
	wall_model& operator=(const wall_model & src) = default;
	wall_model& operator=(wall_model && src) = default;
};

// standard orientation is N (this does not line up with trigonometry)
// we generally want to be able to rotate about any coordinate plane, but default to xy plane (first two coordinates)
// we generally want to be able to do any non-interpolating rotation

enum {
	VOXEL_EDGE = 16
};

}	// namespace grid
}	// namespace iskandria
#endif
