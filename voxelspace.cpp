#include "voxelspace.hpp"
#include "display_manager.hpp"
#include "Zaimoni.STL/flat_alg2.hpp"

namespace isk {

bool screen_delta(zaimoni::math::vector<int, 3> x0, zaimoni::math::vector<int, 3> x1, std::pair<zaimoni::math::vector<int, 2>, int>& dest)
{
	std::pair<zaimoni::math::vector<int, 2>, int> working(zaimoni::math::vector<int, 2>(), 0);

	// pure z-index adjustments
	// +,-,+: +3 z-index
	if (x0[0] < x1[0] && x0[1] > x1[1] && x0[2] < x1[2]) {
		if (const auto max_delta = zaimoni::pos_diff(std::numeric_limits<int>::max(), working.second) / 3; 0 < max_delta) {
			const auto delta = zaimoni::min(zaimoni::pos_diff(x1[0], x0[0]), zaimoni::pos_diff(x0[1], x1[1]), zaimoni::pos_diff(x1[2], x0[2]), max_delta);
			working.second += 3 * delta;
			x1[0] -= delta;
			x1[1] += delta;
			x1[2] -= delta;
		}
	}
	// +,-,+: -3 z-index
	else if (x0[0] > x1[0] && x0[1] < x1[1] && x0[2] > x1[2]) {
		if (const auto max_delta = zaimoni::pos_diff(working.second, std::numeric_limits<int>::min()) / 3; 0 < max_delta) {
			const auto delta = zaimoni::min(zaimoni::pos_diff(x0[0], x1[0]), zaimoni::pos_diff(x1[1], x0[1]), zaimoni::pos_diff(x0[2], x1[2]), max_delta);
			working.second -= 3 * delta;
			x1[0] += delta;
			x1[1] -= delta;
			x1[2] += delta;
		}
	}

	if (x0 == x1) {
		dest = working;
		return true;
	}

	return false;
}

}	// namespace isk