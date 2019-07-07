#ifndef INTERVAL_SHIM_HPP
#define INTERVAL_SHIM_HPP

// evade boost library hard crash
#ifdef __GNUC__
#ifndef __i386__
#define CONSTANTS_ISK_INTERVAL 1
#endif
#endif

// #define CONSTANTS_ISK_INTERVAL 1

#ifdef CONSTANTS_ISK_INTERVAL
#include "Zaimoni.STL/interval.hpp"
#else
#include <boost/numeric/interval.hpp>
#include "Zaimoni.STL/augment.STL/cmath"
#include "Zaimoni.STL/Logging.h"
#define ISK_INTERVAL boost::numeric::interval
#include "Zaimoni.STL/_interval.hpp"
#endif

struct interval_shim
{
	typedef ISK_INTERVAL<double> interval;
	static const interval pi;	// define is a point estimate
};

#endif
