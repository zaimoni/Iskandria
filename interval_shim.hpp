#ifndef INTERVAL_SHIM_HPP
#define INTERVAL_SHIM_HPP

// #define CONSTANTS_ISK_INTERVAL 1

#ifdef CONSTANTS_ISK_INTERVAL
#include "interval.hpp"
#define ISK_INTERVAL zaimoni::math::interval
#else
#include <boost/numeric/interval.hpp>
#include "Zaimoni.STL/augment.STL/cmath"
#include "Zaimoni.STL/Logging.h"
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

// Logging.h support -- outside of namespace to match Logging.h

template<class T>
void INFORM(const ISK_INTERVAL<T>& x)
{
	if (x.lower() == x.upper()) {
		INFORM(x.upper());
		return;
	}
	INC_INFORM("[");
	INC_INFORM(x.lower());
	INC_INFORM(",");
	INC_INFORM(x.upper());
	INFORM("]");
}

template<class T>
void INC_INFORM(const ISK_INTERVAL<T>& x)
{
	if (x.lower() == x.upper()) {
		INC_INFORM(x.upper());
		return;
	}
	INC_INFORM("[");
	INC_INFORM(x.lower());
	INC_INFORM(",");
	INC_INFORM(x.upper());
	INC_INFORM("]");
}

namespace zaimoni {

template<>
struct definitely<typename interval_shim::interval>
{
	typedef typename interval_shim::interval interval;
	static bool equal(typename const_param<interval>::type lhs, typename const_param<interval>::type rhs) { return rhs.upper()==rhs.lower() && lhs == rhs.upper(); }
	static bool unequal(typename const_param<interval>::type lhs, typename const_param<interval>::type rhs) { return lhs.upper()<rhs.lower() || rhs.upper()<lhs.lower(); }
};

template<class T>
constexpr bool isFinite(const ISK_INTERVAL<T>& x)
{
	return isFinite(x.lower()) && isFinite(x.upper());
}

template<class T>
constexpr bool isNaN(const ISK_INTERVAL<T>& x)
{
	return isNaN(x.lower()) || isNaN(x.upper());
}

template<class T>
constexpr ISK_INTERVAL<T> scalBn(const ISK_INTERVAL<T>& x, int scale)
{
	return ISK_INTERVAL<T>(scalBn(x.lower(), scale), scalBn(x.upper(), scale));
}

namespace math {

	using zaimoni::isNaN;
	using zaimoni::scalBn;

}
}	// namespace zaimoni

#endif
