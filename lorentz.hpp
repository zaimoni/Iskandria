#ifndef LORENTZ_HPP
#define LORENTZ_HPP

#include "constants.hpp"
#include "matrix.hpp"

/*
	West coast Lorentz metric: 1,3 i.e. (+, -, -, -).  E.g., Jakob Schwichtenberg; (t, x, y, z)
	East coast Lorentz metric: 3,1 i.e. (+, +, +, -).  E.g., Misner/Thorne/Wheeler (x, y, z, t)
*/

namespace zaimoni {
namespace math {

	constexpr uintmax_t m_choose_n(uintmax_t m, uintmax_t n) {
		if (m > n) return 0;	// probably should throw but need a high enough C++ version for that
		// basis cases
		if (0 == m || m == n) return 1;
		if (1 == m || m + 1 == n) return n;
		if (n / 2 < m) m = n - m; // reduce
		// can overflow, in principle
		uintmax_t numerator = n;
		uintmax_t denominator = 1;
		uintmax_t index = 1;
		while (index < m) {
			uintmax_t stage_numerator = n - index;
			uintmax_t stage_denominator = index + 1;
			auto test = gcd(stage_numerator, stage_denominator);
			if (1 < test) {
				stage_numerator /= test;
				stage_denominator /= test;
			}
			if (1 < stage_denominator) {
				test = gcd(numerator, stage_denominator);
				if (1 < test) {
					numerator /= test;
					stage_denominator /= test;
				}
			}
			if (1 < denominator) {
				test = gcd(stage_numerator, denominator);
				if (1 < test) {
					stage_numerator /= test;
					denominator /= test;
				}
			}
			if ((uintmax_t)(-1) / numerator >= stage_numerator) {
				numerator *= stage_numerator;
				denominator *= stage_denominator;
			} else if (!std::is_constant_evaluated()) throw std::runtime_error("overflow");
		}
	}

}
}

namespace differential_geometry {
	template<class T, fundamental_constants::units u, bool west_coast = true, size_t spatial = 3, size_t temporal = 1>
	class vector
	{
		static_assert(0 < spatial);
		static_assert(0 < temporal);

		std::array<T, spatial + temporal> _x;
	public:
		vector() = default;
		vector(const vector& src) = default;
		vector(vector&& src) = default;
		~vector() = default;
		vector& operator=(const vector& src) = default;
		vector& operator=(vector && src) = default;

		T& operator[](size_t i) { return _x[i]; }
		const T& operator[](size_t i) const { return _x[i]; }
	};

	template<size_t n, class T, fundamental_constants::units u, bool west_coast = true, size_t spatial = 3, size_t temporal = 1>
	class form
	{
		static_assert(0 < spatial);
		static_assert(0 < temporal);

		std::array<T, zaimoni::math::m_choose_n(n, spatial + temporal)> _x;
	public:
		form() = default;
		form(const form& src) = default;
		form(form&& src) = default;
		~form() = default;
		form& operator=(const form& src) = default;
		form& operator=(form&& src) = default;

		T& operator[](size_t i) { return _x[i]; }
		const T& operator[](size_t i) const { return _x[i]; }
	};
}

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
			static constexpr const auto ooao = zaimoni::math::matrix_diagonal<signed char, spatial + temporal>(differential_geometry_signature<temporal, spatial>());
			return ooao;
		} else {
			static constexpr const auto ooao = zaimoni::math::matrix_diagonal<signed char, spatial + temporal>(differential_geometry_signature<spatial, temporal>());
			return ooao;
		}
	}
}

#endif
