#ifndef _INTERVAL_HPP
#define _INTERVAL_HPP 1

#ifndef ISK_INTERVAL
#error improper usage of _interval.hpp: define ISK_INTERVAL first
#endif
// Logging.h support -- outside of namespace to match Logging.h

#include <string>
#include <sstream>
#include <iomanip>

namespace zaimoni {

	using std::to_string;

	template<class T>
	std::string to_string(const ISK_INTERVAL<T>& x)
	{
		std::stringstream dest;
		dest << std::setiosflags(std::ios::scientific) << std::setprecision(std::numeric_limits<T>::max_digits10);
		if (x.lower() == x.upper()) dest << x.upper();
		else dest << "[" << x.lower() << ',' << x.upper() << ']';
		return dest.str();
	}

}

template<class T>
void INFORM(const ISK_INTERVAL<T>& x) { INFORM(zaimoni::to_string(x).c_str()); }

template<class T>
void INC_INFORM(const ISK_INTERVAL<T>& x) { INC_INFORM(zaimoni::to_string(x).c_str()); }

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
	struct type_traits_arithmetic_aux<ISK_INTERVAL<T> > {
		static constexpr bool is_zero(typename const_param<ISK_INTERVAL<T> >::type x) { return x == T(0); }	// could use kronecker delta for this
		static constexpr bool contains_zero(typename const_param<ISK_INTERVAL<T> >::type x) { return 0 >= x.lower() && 0 <= x.upper(); }
		static constexpr bool is_positive(typename const_param<ISK_INTERVAL<T> >::type x) { return 0 < x.lower(); }
		static constexpr bool is_negative(typename const_param<ISK_INTERVAL<T> >::type x) { return 0 > x.upper(); }
		static constexpr bool is_one(typename const_param<ISK_INTERVAL<T> >::type x) { return x == T(1); }
	};

	template<class T>
	constexpr bool isINF(const ISK_INTERVAL<T>& x)
	{
		return isINF(x.lower()) && isINF(x.upper()) && signBit(x.lower()) == signBit(x.upper());	// we do want numeric intervals of numeric intervals to be a compiler error
	}

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

	template<class T>
	constexpr bool is_zero(const ISK_INTERVAL<T>& x)
	{
		return is_zero(x.lower()) && is_zero(x.upper());
	}

	template<class T>
	constexpr bool contains_zero(const ISK_INTERVAL<T>& x)
	{
		return !is_positive(x.lower()) && !is_negative(x.upper());
	}

	template<class T>
	constexpr bool is_positive(const ISK_INTERVAL<T>& x)
	{
		return is_positive(x.lower());
	}

	template<class T>
	constexpr bool is_negative(const ISK_INTERVAL<T>& x)
	{
		return is_negative(x.upper());
	}

	template<class T>
	constexpr bool is_one(const ISK_INTERVAL<T>& x)	// could use kronecker delta for this
	{
		return is_one(x.lower()) && is_one(x.upper());
	}

	namespace math {

		using zaimoni::isNaN;
		using zaimoni::scalBn;

		template<int code, class T>
		class Interval : public ISK_INTERVAL<T> {
		public:
			Interval() = default;
			explicit Interval(const ISK_INTERVAL<T>& src) : ISK_INTERVAL<T>(src) {};
			ZAIMONI_DEFAULT_COPY_DESTROY_ASSIGN(Interval);
		};
	}
}	// namespace zaimoni

#ifdef VAR_HPP
#include "bits/_interval_var.hpp"
#endif

#endif
