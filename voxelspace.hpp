#ifndef VOXELSPACE_HPP
#define VOXELSPACE_HPP

// closely related to display_manager.hpp/cpp; splitting out to avoid increasing dependencies in display_manager.hpp
#include "matrix.hpp"
#include <optional>

namespace isk {

	// canonical camera view: positive x down-right, positive y up-right, positive z up
	// dest format: pixel offset, z-index offset
	std::optional<std::pair<zaimoni::math::vector<int, 2>, int> > screen_delta(zaimoni::math::vector<ptrdiff_t, 3> x0, zaimoni::math::vector<ptrdiff_t, 3> x1);

}

#endif
