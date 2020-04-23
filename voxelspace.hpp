#ifndef VOXELSPACE_HPP
#define VOXELSPACE_HPP

// closely related to display_manager.hpp/cpp; splitting out to avoid increasing dependencies in display_manager.hpp
#include "matrix.hpp"

namespace isk {

	// canonical camera view: positive x down-right, positive y up-right, positive z up
	// dest format: pixel offset, z-index offset
	bool screen_delta(zaimoni::math::vector<int, 3> x0, zaimoni::math::vector<int, 3> x1, std::pair<zaimoni::math::vector<int, 2>, int>& dest);

}

#endif
