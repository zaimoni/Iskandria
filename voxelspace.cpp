#include "voxelspace.hpp"
#include "display_manager.hpp"
#include "Zaimoni.STL/flat_alg2.hpp"

namespace isk {

std::optional<std::pair<zaimoni::math::vector<int, 2>, int> >  screen_delta(zaimoni::math::vector<int, 3> x0, zaimoni::math::vector<int, 3> x1)
{
	std::pair<zaimoni::math::vector<int, 2>, int> working(zaimoni::math::vector<int, 2>(), 0);
	auto x_delta = x0[0] < x1[0] ? std::pair(true, zaimoni::pos_diff(x1[0], x0[0])) : std::pair(false, zaimoni::pos_diff(x0[0], x1[0]));
	auto y_delta = x0[1] < x1[1] ? std::pair(true, zaimoni::pos_diff(x1[1], x0[1])) : std::pair(false, zaimoni::pos_diff(x0[1], x1[1]));
	auto z_delta = x0[2] < x1[2] ? std::pair(true, zaimoni::pos_diff(x1[2], x0[2])) : std::pair(false, zaimoni::pos_diff(x0[2], x1[2]));
	while (x_delta.second || y_delta.second || z_delta.second) {
		if (z_delta.second) {
			if (z_delta.first) {
				const auto delta = zaimoni::max(zaimoni::pos_diff(std::numeric_limits<int>::max(), working.second),
					zaimoni::pos_diff(working.first[1], std::numeric_limits<int>::min()) / (DisplayManager::ISOMETRIC_TRIANGLE_HEIGHT + 1),
					z_delta.second);
				if (0 < delta) {
					working.first[1] -= (DisplayManager::ISOMETRIC_TRIANGLE_HEIGHT + 1) * delta;
					working.second += delta;
					z_delta.second -= delta;
					continue;
				}
			} else {
				const auto delta = zaimoni::max(zaimoni::pos_diff(working.second, std::numeric_limits<int>::min()),
					zaimoni::pos_diff(std::numeric_limits<int>::max(), working.first[1]) / (DisplayManager::ISOMETRIC_TRIANGLE_HEIGHT + 1),
					z_delta.second);
				if (0 < delta) {
					working.first[1] += (DisplayManager::ISOMETRIC_TRIANGLE_HEIGHT + 1) * delta;
					working.second -= delta;
					z_delta.second -= delta;
					continue;
				}
			}
		}
		if (x_delta.second) {
			if (x_delta.first) {
				const auto delta = zaimoni::max(zaimoni::pos_diff(std::numeric_limits<int>::max(), working.second),
					zaimoni::pos_diff(std::numeric_limits<int>::max(), working.first[0]) / DisplayManager::ISOMETRIC_TRIANGLE_WIDTH,
					zaimoni::pos_diff(std::numeric_limits<int>::max(), working.first[1]) / ((DisplayManager::ISOMETRIC_TRIANGLE_HEIGHT + 1) / 2),
					x_delta.second);
				if (0 < delta) {
					working.first[0] += DisplayManager::ISOMETRIC_TRIANGLE_WIDTH * delta;
					working.first[1] += ((DisplayManager::ISOMETRIC_TRIANGLE_HEIGHT + 1) / 2) * delta;
					working.second += delta;
					x_delta.second -= delta;
					continue;
				}
			} else {
				const auto delta = zaimoni::max(zaimoni::pos_diff(working.second, std::numeric_limits<int>::min()),
					zaimoni::pos_diff(working.first[0], std::numeric_limits<int>::min()) / DisplayManager::ISOMETRIC_TRIANGLE_WIDTH,
					zaimoni::pos_diff(working.first[1], std::numeric_limits<int>::min()) / ((DisplayManager::ISOMETRIC_TRIANGLE_HEIGHT + 1) / 2),
					x_delta.second);
				if (0 < delta) {
					working.first[0] -= DisplayManager::ISOMETRIC_TRIANGLE_WIDTH * delta;
					working.first[1] -= ((DisplayManager::ISOMETRIC_TRIANGLE_HEIGHT + 1) / 2) * delta;
					working.second -= delta;
					x_delta.second -= delta;
					continue;
				}
			}
		}
		if (y_delta.second) {
			if (y_delta.first) {
				const auto delta = zaimoni::max(zaimoni::pos_diff(working.second, std::numeric_limits<int>::min()),
					zaimoni::pos_diff(std::numeric_limits<int>::max(), working.first[0]) / DisplayManager::ISOMETRIC_TRIANGLE_WIDTH,
					zaimoni::pos_diff(working.first[1], std::numeric_limits<int>::min()) / ((DisplayManager::ISOMETRIC_TRIANGLE_HEIGHT + 1) / 2),
					x_delta.second);
				if (0 < delta) {
					working.first[0] += DisplayManager::ISOMETRIC_TRIANGLE_WIDTH * delta;
					working.first[1] -= ((DisplayManager::ISOMETRIC_TRIANGLE_HEIGHT + 1) / 2) * delta;
					working.second -= delta;
					y_delta.second -= delta;
					continue;
				}
			} else {
				const auto delta = zaimoni::max(zaimoni::pos_diff(std::numeric_limits<int>::max(), working.second),
					zaimoni::pos_diff(working.first[0], std::numeric_limits<int>::min()) / DisplayManager::ISOMETRIC_TRIANGLE_WIDTH,
					zaimoni::pos_diff(std::numeric_limits<int>::max(), working.first[1]) / ((DisplayManager::ISOMETRIC_TRIANGLE_HEIGHT + 1) / 2),
					x_delta.second);
				if (0 < delta) {
					working.first[0] -= DisplayManager::ISOMETRIC_TRIANGLE_WIDTH * delta;
					working.first[1] += ((DisplayManager::ISOMETRIC_TRIANGLE_HEIGHT + 1) / 2) * delta;
					working.second += delta;
					y_delta.second -= delta;
					continue;
				}
			}
		}
		return std::nullopt;
	}
	return working;
}

}	// namespace isk