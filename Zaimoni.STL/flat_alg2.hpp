// flat_alg2.hpp

#ifndef FLAT_ALG2_HPP
#define FLAT_ALG2_HPP 1

namespace zaimoni {

// Perl's spaceship operator
template<class T,class U>
constexpr int cmp(T& lhs, U& rhs)
{
	if (lhs==rhs) return 0;
	else if (lhs < rhs) return -1;
	else return 1;
}

template<class T1, class T2, class...Args>
constexpr auto min(const T1& x1, const T2& x2, Args...xn) {
	if constexpr (0 < sizeof...(xn)) {
		return min(x1 < x2 ? x1 : x2, xn...);
	} else {
		return x1 < x2 ? x1 : x2;
	}
}

template<class T1, class T2, class...Args>
constexpr auto max(const T1& x1, const T2& x2, Args...xn) {
	if constexpr (0 < sizeof...(xn)) {
		return max(x1 < x2 ? x2 : x1, xn...);
	} else {
		return x1 < x2 ? x2 : x1;
	}
}

}	// namespace zaimoni

#endif
