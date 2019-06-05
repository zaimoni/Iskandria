#ifndef INTERVAL_SHIM_HPP
#define INTERVAL_SHIM_HPP

// #define CONSTANTS_ISK_INTERVAL 1

#ifdef CONSTANTS_ISK_INTERVAL
#include "interval.hpp"
#else
#include <boost/numeric/interval.hpp>
#include "Zaimoni.STL/augment.STL/type_traits"
#endif

struct interval_shim
{
#ifdef CONSTANTS_ISK_INTERVAL
	typedef zaimoni::math::interval<double> interval;
#else
	typedef boost::numeric::interval<double> interval;
#endif

	static const interval pi;	// define is a point estimate
};

namespace zaimoni {

template<>
struct definitely<interval_shim::interval>
{
	static bool equal(typename const_param<interval_shim::interval>::type lhs, typename const_param<interval_shim::interval>::type rhs) { return rhs.upper()==rhs.lower() && lhs == rhs.upper(); }
	static bool unequal(typename const_param<interval_shim::interval>::type lhs, typename const_param<interval_shim::interval>::type rhs) { return lhs.upper()<rhs.lower() || rhs.upper()<lhs.lower(); }
};

}	// namespace zaimoni

#endif
