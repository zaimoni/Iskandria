#ifndef INTERVAL_SHIM_HPP
#define INTERVAL_SHIM_HPP

#include "Zaimoni.STL/interval.hpp"

struct interval_shim
{
	typedef ISK_INTERVAL<double> interval;
	static constexpr const interval pi = interval(3.1415926535897932, 3.1415926535897935);

	// angle type wants these
	static constexpr const interval SQRT2 = interval(1.41421356237309492, 1.41421356237309515);
	static constexpr const interval SQRT3 = interval(1.73205080756887719, 1.73205080756887742);
};

#endif
