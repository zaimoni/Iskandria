#ifndef INTERPOLATE_HPP
#define INTERPOLATE_HPP 1

#include <utility>
#include "Zaimoni.STL/augment.STL/type_traits"

namespace zaimoni {
namespace math {
namespace interpolate_1d {

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
		DOM from_x0(src - _x0.first);
		DOM from_x1(_x1.first - src);
		if (zaimoni::norm(from_x0) <= zaimoni::norm(from_x1)) return _x0.second + (from_x0 / span.first) * span.second;
		return _x1.second - (from_x1 / span.first) * span.second;
	}
};

# if 0
class quadratic
{
};

class cubic
{
};

class quartic
{
};
#endif

}
}
}

#endif
