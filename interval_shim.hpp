#ifndef INTERVAL_SHIM_HPP
#define INTERVAL_SHIM_HPP

// #define CONSTANTS_ISK_INTERVAL 1

#ifdef CONSTANTS_ISK_INTERVAL
#include "interval.hpp"
#else
#include <boost/numeric/interval.hpp>
#include "Zaimoni.STL/augment.STL/cmath"
#include "Zaimoni.STL/Logging.h"
#define ISK_INTERVAL boost::numeric::interval
#include "_interval.hpp"
#endif

struct interval_shim
{
	typedef ISK_INTERVAL<double> interval;
	static const interval pi;	// define is a point estimate
};

#endif
