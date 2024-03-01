#ifndef ZAIMONI_STL_ZERO_HPP
#define ZAIMONI_STL_ZERO_HPP 1

namespace zaimoni {

// https://devblogs.microsoft.com/oldnewthing/20200413-00/?p=103669 [Raymond Chen/"The Old New Thing"]
template<typename T, typename...>
using unconditional_t = T;

template<typename T, T v, typename...>
inline constexpr T unconditional_v = v;

// https://artificial-mind.net/blog/2020/11/14/cpp17-consteval
template <auto V>
static constexpr const auto force_consteval = V;

}	// namespace zaimoni

#endif
