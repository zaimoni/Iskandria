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

	template<class T>
	struct types<ISK_INTERVAL<T> >
	{
		typedef ISK_INTERVAL<typename types<T>::norm> norm;
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

		template<std::floating_point F>
		struct would_overflow<ISK_INTERVAL<F> >
		{
			static bool sum(const ISK_INTERVAL<F>& lhs, const ISK_INTERVAL<F>& rhs) {
				return would_overflow<F>::sum(lhs.lower(), rhs.lower()) || would_overflow<F>(lhs.upper(), rhs.upper());
			}
//			static int product(const ISK_INTERVAL<F>& lhs, const ISK_INTERVAL<F>& rhs) { ...; }
			static int square(const ISK_INTERVAL<F>& lhs) {
				if (0 <= lhs.lower()) return would_overflow<F>::square(lhs.upper());
				if (0 >= lhs.upper()) return would_overflow<F>::square(lhs.lower());
				if (-lhs.lower() <= lhs.upper()) return would_overflow<F>::square(lhs.upper());
				return would_overflow<F>::square(lhs.lower());
			}
		};

		template<class T>
		struct numerical<ISK_INTERVAL<T> >
		{
			enum {
				error_tracking = 1
			};
			typedef typename std::remove_cv<T>::type exact_type;			// exact version of this type
			typedef typename std::remove_cv<T>::type exact_arithmetic_type;	// exact version of the individual coordinates of this type

			static auto error(typename const_param<ISK_INTERVAL<T> >::type src) {
				auto err(src.upper());
				err -= src.lower();
				return err.upper();
			}
			static constexpr bool causes_division_by_zero(typename const_param<ISK_INTERVAL<T> >::type src) { return contains_zero(src); };
		};

		template<int code, class T>
		class Interval : public ISK_INTERVAL<T> {
		public:
			constexpr Interval() = default;
			explicit constexpr Interval(const ISK_INTERVAL<T>& src) : ISK_INTERVAL<T>(src) {};
			explicit constexpr Interval(const T& l, const T& u) : ISK_INTERVAL<T>(l,u) {};
			ZAIMONI_DEFAULT_COPY_DESTROY_ASSIGN(Interval);
		};
	}
}	// namespace zaimoni

#ifdef VAR_HPP
#include "bits/_interval_var.hpp"
#endif

#endif
