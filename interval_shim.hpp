#ifndef INTERVAL_SHIM_HPP
#define INTERVAL_SHIM_HPP

// #define CONSTANTS_ISK_INTERVAL 1

#ifdef CONSTANTS_ISK_INTERVAL
#include "interval.hpp"
#define ISK_INTERVAL zaimoni::math::interval
#else
#include <boost/numeric/interval.hpp>
#include "Zaimoni.STL/augment.STL/type_traits"
#define ISK_INTERVAL boost::numeric::interval
#endif

struct interval_shim
{
	typedef ISK_INTERVAL<double> interval;
	static const interval pi;	// define is a point estimate
};

template<class T>
struct interval_shim_template
{
	typedef ISK_INTERVAL<T> interval;
};

namespace zaimoni {

template<>
struct definitely<typename interval_shim::interval>
{
	typedef typename interval_shim::interval interval;
	static bool equal(typename const_param<interval>::type lhs, typename const_param<interval>::type rhs) { return rhs.upper()==rhs.lower() && lhs == rhs.upper(); }
	static bool unequal(typename const_param<interval>::type lhs, typename const_param<interval>::type rhs) { return lhs.upper()<rhs.lower() || rhs.upper()<lhs.lower(); }
};

}	// namespace zaimoni

#endif
