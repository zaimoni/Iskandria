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

	}
}	// namespace zaimoni

// would prefer to make this triggered by common include of _interval.hpp and var.hpp, but that involves
// moving interval family to Zaimoni.STL and we're not ready for that technically

#include "Zaimoni.STL/var.hpp"

namespace zaimoni {

#pragma start_copy type_of
	template<>
	struct _type_of<ISK_INTERVAL<double> >
	{
		typedef typename _type_of<double>::type type;
	};
#pragma end_copy
#pragma substitute float for double in type_of
	template<>
	struct _type_of<ISK_INTERVAL<float> >
	{
		typedef typename _type_of<float>::type type;
	};
#pragma end_substitute
#pragma substitute long double for double in type_of
	template<>
	struct _type_of<ISK_INTERVAL<long double> >
	{
		typedef typename _type_of<long double>::type type;
	};
#pragma end_substitute

	// we don't want to use a normal macro here as this code is not really expected to be stable
#pragma start_copy interface_of
	template<>
	template<class Derived>
	struct _interface_of<Derived, ISK_INTERVAL<double>, 0> : public virtual fp_API
	{
	private:
		_fp_stats<double> _stats_l;
		_fp_stats<double> _stats_u;
	public:
		void invalidate_stats() {
			_stats_l.invalidate_stats();
			_stats_u.invalidate_stats();
		}

		virtual bool is_scal_bn_identity() const {
			force_valid_stats();
			return _stats_l.is_scal_bn_identity() && _stats_u.is_scal_bn_identity();
		}

		virtual std::pair<intmax_t, intmax_t> scal_bn_safe_range() const {
			// frexp convention: mantissa is [0.5,1.0) and exponent of 1.0 is 1
			force_valid_stats();
			std::pair<intmax_t, intmax_t> ret(fp_API::max_scal_bn_safe_range);
			_stats_l.scal_bn_safe_range(ret.first, ret.second);
			_stats_u.scal_bn_safe_range(ret.first, ret.second);
			return ret;
		}

		virtual intmax_t ideal_scal_bn() const {
			if (is_scal_bn_identity()) return 0;	// inline this if/when micro-optimizing
			const auto rhs = _stats_u.ideal_scal_bn();
			if (_stats_l.is_scal_bn_identity()) return rhs;
			const auto lhs = _stats_l.ideal_scal_bn();
			if (_stats_u.is_scal_bn_identity()) return lhs;
			if (0 < lhs && 0 < rhs) return (lhs < rhs) ? lhs : rhs;
			if (0 > lhs && 0 > rhs) return (lhs < rhs) ? rhs : lhs;
//			if (0 == rhs || 0 == lhs) return 0;
			return 0;
		}

		virtual bool scal_bn(intmax_t scale) {
			if (0 == scale) return true;	// no-op
			if (is_scal_bn_identity()) return true;	// no-op

			const auto legal = scal_bn_safe_range();
			if (0 < scale) {
				if (scale > legal.second) return false;
			}
			else {	// if (0 > scale)
				if (scale < legal.first) return false;
			}
			auto& x = static_cast<Derived*>(this)->value();
			x.assign(std::scalbn(x.lower(), scale),std::scalbn(x.upper(), scale));
			return true;
		}	// power-of-two

		virtual fp_API* clone() const {
			auto& x = static_cast<const Derived*>(this)->value();
			const auto tmp = x.lower();
			if (tmp!=x.upper()) return new Derived(*static_cast<const Derived*>(this));
			// singleton...optimize when cloning
			return new var<double,typename _type_of<Derived>::type>(tmp);
		}
	private:
		void force_valid_stats() const {
			if (!_stats_l.valid()) {
				auto& x = static_cast<const Derived*>(this)->value();
				_stats_l.init_stats(x.lower());
				_stats_u.init_stats(x.upper());
			}
		}
	};

	template<>
	template<class Derived>
	struct _interface_of<Derived, ISK_INTERVAL<double>, 1> : public _interface_of<Derived, ISK_INTERVAL<double>, 0>
	{
		virtual bool is_inf() const {
			auto& x = static_cast<const Derived*>(this)->value();
			return std::isinf(x.lower()) || std::isinf(x.upper());	// \todo may not be correct
		}
		virtual bool is_finite() const {
			auto& x = static_cast<const Derived*>(this)->value();
			return std::isfinite(x.lower()) || std::isfinite(x.upper());
		}
	};
#pragma end_copy
#pragma substitute float for double in interface_of
	template<>
	template<class Derived>
	struct _interface_of<Derived, ISK_INTERVAL<float>, 0> : public virtual fp_API
	{
	private:
		_fp_stats<float> _stats_l;
		_fp_stats<float> _stats_u;
	public:
		void invalidate_stats() {
			_stats_l.invalidate_stats();
			_stats_u.invalidate_stats();
		}

		virtual bool is_scal_bn_identity() const {
			force_valid_stats();
			return _stats_l.is_scal_bn_identity() && _stats_u.is_scal_bn_identity();
		}

		virtual std::pair<intmax_t, intmax_t> scal_bn_safe_range() const {
			// frexp convention: mantissa is [0.5,1.0) and exponent of 1.0 is 1
			force_valid_stats();
			std::pair<intmax_t, intmax_t> ret(fp_API::max_scal_bn_safe_range);
			_stats_l.scal_bn_safe_range(ret.first, ret.second);
			_stats_u.scal_bn_safe_range(ret.first, ret.second);
			return ret;
		}

		virtual intmax_t ideal_scal_bn() const {
			if (is_scal_bn_identity()) return 0;	// inline this if/when micro-optimizing
			const auto rhs = _stats_u.ideal_scal_bn();
			if (_stats_l.is_scal_bn_identity()) return rhs;
			const auto lhs = _stats_l.ideal_scal_bn();
			if (_stats_u.is_scal_bn_identity()) return lhs;
			if (0 < lhs && 0 < rhs) return (lhs < rhs) ? lhs : rhs;
			if (0 > lhs && 0 > rhs) return (lhs < rhs) ? rhs : lhs;
//			if (0 == rhs || 0 == lhs) return 0;
			return 0;
		}

		virtual bool scal_bn(intmax_t scale) {
			if (0 == scale) return true;	// no-op
			if (is_scal_bn_identity()) return true;	// no-op

			const auto legal = scal_bn_safe_range();
			if (0 < scale) {
				if (scale > legal.second) return false;
			}
			else {	// if (0 > scale)
				if (scale < legal.first) return false;
			}
			auto& x = static_cast<Derived*>(this)->value();
			x.assign(std::scalbn(x.lower(), scale),std::scalbn(x.upper(), scale));
			return true;
		}	// power-of-two

		virtual fp_API* clone() const {
			auto& x = static_cast<const Derived*>(this)->value();
			const auto tmp = x.lower();
			if (tmp!=x.upper()) return new Derived(*static_cast<const Derived*>(this));
			// singleton...optimize when cloning
			return new var<float,typename _type_of<Derived>::type>(tmp);
		}
	private:
		void force_valid_stats() const {
			if (!_stats_l.valid()) {
				auto& x = static_cast<const Derived*>(this)->value();
				_stats_l.init_stats(x.lower());
				_stats_u.init_stats(x.upper());
			}
		}
	};

	template<>
	template<class Derived>
	struct _interface_of<Derived, ISK_INTERVAL<float>, 1> : public _interface_of<Derived, ISK_INTERVAL<float>, 0>
	{
		virtual bool is_inf() const {
			auto& x = static_cast<const Derived*>(this)->value();
			return std::isinf(x.lower()) || std::isinf(x.upper());	// \todo may not be correct
		}
		virtual bool is_finite() const {
			auto& x = static_cast<const Derived*>(this)->value();
			return std::isfinite(x.lower()) || std::isfinite(x.upper());
		}
	};
#pragma end_substitute
#pragma substitute long double for double in interface_of
	template<>
	template<class Derived>
	struct _interface_of<Derived, ISK_INTERVAL<long double>, 0> : public virtual fp_API
	{
	private:
		_fp_stats<long double> _stats_l;
		_fp_stats<long double> _stats_u;
	public:
		void invalidate_stats() {
			_stats_l.invalidate_stats();
			_stats_u.invalidate_stats();
		}

		virtual bool is_scal_bn_identity() const {
			force_valid_stats();
			return _stats_l.is_scal_bn_identity() && _stats_u.is_scal_bn_identity();
		}

		virtual std::pair<intmax_t, intmax_t> scal_bn_safe_range() const {
			// frexp convention: mantissa is [0.5,1.0) and exponent of 1.0 is 1
			force_valid_stats();
			std::pair<intmax_t, intmax_t> ret(fp_API::max_scal_bn_safe_range);
			_stats_l.scal_bn_safe_range(ret.first, ret.second);
			_stats_u.scal_bn_safe_range(ret.first, ret.second);
			return ret;
		}

		virtual intmax_t ideal_scal_bn() const {
			if (is_scal_bn_identity()) return 0;	// inline this if/when micro-optimizing
			const auto rhs = _stats_u.ideal_scal_bn();
			if (_stats_l.is_scal_bn_identity()) return rhs;
			const auto lhs = _stats_l.ideal_scal_bn();
			if (_stats_u.is_scal_bn_identity()) return lhs;
			if (0 < lhs && 0 < rhs) return (lhs < rhs) ? lhs : rhs;
			if (0 > lhs && 0 > rhs) return (lhs < rhs) ? rhs : lhs;
//			if (0 == rhs || 0 == lhs) return 0;
			return 0;
		}

		virtual bool scal_bn(intmax_t scale) {
			if (0 == scale) return true;	// no-op
			if (is_scal_bn_identity()) return true;	// no-op

			const auto legal = scal_bn_safe_range();
			if (0 < scale) {
				if (scale > legal.second) return false;
			}
			else {	// if (0 > scale)
				if (scale < legal.first) return false;
			}
			auto& x = static_cast<Derived*>(this)->value();
			x.assign(std::scalbn(x.lower(), scale),std::scalbn(x.upper(), scale));
			return true;
		}	// power-of-two

		virtual fp_API* clone() const {
			auto& x = static_cast<const Derived*>(this)->value();
			const auto tmp = x.lower();
			if (tmp!=x.upper()) return new Derived(*static_cast<const Derived*>(this));
			// singleton...optimize when cloning
			return new var<long double,typename _type_of<Derived>::type>(tmp);
		}
	private:
		void force_valid_stats() const {
			if (!_stats_l.valid()) {
				auto& x = static_cast<const Derived*>(this)->value();
				_stats_l.init_stats(x.lower());
				_stats_u.init_stats(x.upper());
			}
		}
	};

	template<>
	template<class Derived>
	struct _interface_of<Derived, ISK_INTERVAL<long double>, 1> : public _interface_of<Derived, ISK_INTERVAL<long double>, 0>
	{
		virtual bool is_inf() const {
			auto& x = static_cast<const Derived*>(this)->value();
			return std::isinf(x.lower()) || std::isinf(x.upper());	// \todo may not be correct
		}
		virtual bool is_finite() const {
			auto& x = static_cast<const Derived*>(this)->value();
			return std::isfinite(x.lower()) || std::isfinite(x.upper());
		}
	};
#pragma end_substitute


}

#endif
