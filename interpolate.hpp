#ifndef INTERPOLATE_HPP
#define INTERPOLATE_HPP 1

#include <utility>
#include "Zaimoni.STL/augment.STL/type_traits"

namespace zaimoni {
namespace math {
namespace interpolate_1d {

// this actually can be used to define a linear function by example
template<class DOM, class RANGE>
class linear
{
public:
	typedef std::pair<DOM, RANGE> datum;
private:
	datum _x0;
	datum _x1;
	datum _span;
public:
	linear() = default;
	linear(const datum& x0, const datum& x1) : _x0(x0), _x1(x1), _span(x1.first - x0.first, x1.second - x0.second) {};
	linear(const linear& src) = default;
	linear(linear&& src) = default;
	~linear() = default;
	linear& operator=(const linear& src) = default;
	linear& operator=(linear && src) = default;

	RANGE operator()(const DOM& src) const {
		if (src == _x0.first) return _x0.second;
		if (src == _x1.first) return _x1.second;

		DOM from_x0(src - _x0.first);
		// if the domain is 1-real-dimensional we'd like to use the "nearer endpoint"
		if constexpr (1 == R_coords<DOM>::value) {
			DOM from_x1(_x1.first - src);
			if (zaimoni::norm(from_x1) < zaimoni::norm(from_x0)) return _x1.second - (from_x1 / span.first) * span.second;
		}
		return _x0.second + (from_x0 / span.first) * span.second;
	}

	auto slope() const { return _span.second / _span.first; }
};

// more generally: for n data points (x1,y1)...(xn,yn) on a linear domain
// * identify the pair that are "most distant" in domain; re-order so that (x1,y1) and (xn,yn) are that pair
// * the domain-span is then xn-x1; midpoint (xn+x1)/2 formally, if we subtract (xn-x1)/2 from all points we would land on a closed, symmetric interval
// * on that closed symmetric interval, (yn+y1)/2 and (yn-y1)/2 would correspond to the odd and even parts of the function
#if 0
template<class DOM, class RANGE>
class quadratic
{
public:
	static_assert(1 == R_coords<DOM>::value);
	typedef std::pair<DOM, RANGE> datum;
private:
public:
	quadratic() = default;
	quadratic(const quadratic& src) = default;
	quadratic(quadratic&& src) = default;
	~quadratic() = default;
	quadratic& operator=(const quadratic& src) = default;
	quadratic& operator=(quadratic&& src) = default;
};

template<class DOM, class RANGE>
class cubic
{
public:
	static_assert(1 == R_coords<DOM>::value);
	typedef std::pair<DOM, RANGE> datum;
private:
public:
	cubic() = default;
	cubic(const cubic& src) = default;
	cubic(cubic&& src) = default;
	~cubic() = default;
	cubic& operator=(const cubic& src) = default;
	cubic& operator=(cubic && src) = default;
};

template<class DOM, class RANGE>
class quartic
{
public:
	static_assert(1 == R_coords<DOM>::value);
	typedef std::pair<DOM, RANGE> datum;
private:
public:
	quartic() = default;
	quartic(const quartic& src) = default;
	quartic(quartic&& src) = default;
	~quartic() = default;
	quartic& operator=(const quartic& src) = default;
	quartic& operator=(quartic&& src) = default;
};
#endif

}
}
}

#endif
