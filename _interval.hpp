#ifndef _INTERVAL_HPP
#define _INTERVAL_HPP

#ifndef ISK_INTERVAL
#error improper usage of _interval.hpp: define ISK_INTERVAL first
#endif
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

// support functions with masters in Augment.STL/type_traits or cmath
namespace zaimoni {

	template<>
	struct definitely<ISK_INTERVAL<double>>
	{
		typedef ISK_INTERVAL<double> interval;
		static bool equal(typename const_param<interval>::type lhs, typename const_param<interval>::type rhs) { return rhs.upper() == rhs.lower() && lhs == rhs.upper(); }
		static bool unequal(typename const_param<interval>::type lhs, typename const_param<interval>::type rhs) { return lhs.upper() < rhs.lower() || rhs.upper() < lhs.lower(); }
	};

	template<>
	struct definitely<ISK_INTERVAL<long double>>
	{
		typedef ISK_INTERVAL<long double> interval;
		static bool equal(typename const_param<interval>::type lhs, typename const_param<interval>::type rhs) { return rhs.upper() == rhs.lower() && lhs == rhs.upper(); }
		static bool unequal(typename const_param<interval>::type lhs, typename const_param<interval>::type rhs) { return lhs.upper() < rhs.lower() || rhs.upper() < lhs.lower(); }
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

// would prefer to make this triggered by common include of _interval.hpp and var.hpp, but that involves
// moving interval family to Zaimoni.STL and we're not ready for that technically

#include "Zaimoni.STL/var.hpp"

namespace zaimoni {
}

#endif