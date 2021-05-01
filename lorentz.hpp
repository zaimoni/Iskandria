#ifndef LORENTZ_HPP
#define LORENTZ_HPP

#include "constants.hpp"
#include "matrix.hpp"

/*
	West coast Lorentz metric: 1,3 i.e. (+, -, -, -).  E.g., Jakob Schwichtenberg; (t, x, y, z)
	East coast Lorentz metric: 3,1 i.e. (+, +, +, -).  E.g., Misner/Thorne/Wheeler (x, y, z, t)
*/

namespace metric {
	// Assume c=1
	template<auto plus, auto minus>
	constexpr auto differential_geometry_signature()
	{
		static_assert(0 < plus);
		static_assert(0 < minus);
		std::array<signed char, plus + minus> ret;
		std::fill_n(ret.data(), plus, 1);
		std::fill_n(ret.data() + plus, minus, -1);
		return ret;
	}

	template<bool west_coast = true, size_t spatial = 3, size_t temporal = 1>
	auto& Lorentz()
	{
		if constexpr (west_coast) {
			static constexpr const auto ooao = zaimoni::math::matrix_diagonal<signed char, 4>(differential_geometry_signature<temporal, spatial>());
			return ooao;
		} else {
			static constexpr const auto ooao = zaimoni::math::matrix_diagonal<signed char, 4>(differential_geometry_signature<spatial, temporal>());
			return ooao;
		}
	}
}

#endif
