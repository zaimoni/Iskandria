#ifndef INTERVAL_SHIM_HPP
#define INTERVAL_SHIM_HPP

#include "Zaimoni.STL/interval.hpp"

struct interval_shim
{
	typedef ISK_INTERVAL<double> interval;
	static const interval pi;	// define is a point estimate
};

#endif
