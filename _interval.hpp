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
			auto& x = static_cast<const Derived*>(this)->value();
			auto tmp = x.lower();
			return (0 == tmp || std::isinf(tmp)) && (0 == (tmp = x.upper()) || std::isinf(tmp));
		}

		virtual void scal_bn_safe_range(intmax_t& lb, intmax_t& ub) const {
			// frexp convention: mantissa is [0.5,1.0) and exponent of 1.0 is 1
			auto& x = static_cast<const Derived*>(this)->value();
			if (!_stats_l.valid()) {
				_stats_l.init_stats(x.lower());
				_stats_u.init_stats(x.upper());
			}
			lb = std::numeric_limits<intmax_t>::min();
			ub = std::numeric_limits<intmax_t>::max();
			_stats_l.scal_bn_safe_range(lb, ub);
			_stats_u.scal_bn_safe_range(lb, ub);
		}

		virtual bool scal_bn(intmax_t scale) {
			if (0 == scale) return true;	// no-op
			if (is_scal_bn_identity()) return true;	// no-op

			auto _lb = std::numeric_limits<intmax_t>::min();
			auto _ub = std::numeric_limits<intmax_t>::max();
			const bool stats_were_valid = _stats_l.valid();
			if (stats_were_valid) {
				_stats_l.scal_bn_safe_range(_lb, _ub);
				_stats_u.scal_bn_safe_range(_lb, _ub);
			}

			auto& x = static_cast<Derived*>(this)->value();
			if (!stats_were_valid) {
				_stats_l.init_stats(x.lower());
				_stats_u.init_stats(x.upper());
				_stats_l.scal_bn_safe_range(_lb, _ub);
				_stats_u.scal_bn_safe_range(_lb, _ub);
			}
			if (0 < scale) {
				if (scale > _ub) return false;
			}
			else {	// if (0 > scale)
				if (scale < _lb) return false;
			}
			x.assign(std::scalbn(x.lower(), scale),std::scalbn(x.upper(), scale));
			if (!stats_were_valid) invalidate_stats();
			return true;
		}	// power-of-two

		virtual fp_API* clone() const {
			auto& x = static_cast<const Derived*>(this)->value();
			const auto tmp = x.lower();
			if (tmp!=x.upper()) return new Derived(*static_cast<const Derived*>(this));
			// singleton...optimize when cloning
			return new var<double,typename _type_of<Derived>::type>(tmp);
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
			auto& x = static_cast<const Derived*>(this)->value();
			auto tmp = x.lower();
			return (0 == tmp || std::isinf(tmp)) && (0 == (tmp = x.upper()) || std::isinf(tmp));
		}

		virtual void scal_bn_safe_range(intmax_t& lb, intmax_t& ub) const {
			// frexp convention: mantissa is [0.5,1.0) and exponent of 1.0 is 1
			auto& x = static_cast<const Derived*>(this)->value();
			if (!_stats_l.valid()) {
				_stats_l.init_stats(x.lower());
				_stats_u.init_stats(x.upper());
			}
			lb = std::numeric_limits<intmax_t>::min();
			ub = std::numeric_limits<intmax_t>::max();
			_stats_l.scal_bn_safe_range(lb, ub);
			_stats_u.scal_bn_safe_range(lb, ub);
		}

		virtual bool scal_bn(intmax_t scale) {
			if (0 == scale) return true;	// no-op
			if (is_scal_bn_identity()) return true;	// no-op

			auto _lb = std::numeric_limits<intmax_t>::min();
			auto _ub = std::numeric_limits<intmax_t>::max();
			const bool stats_were_valid = _stats_l.valid();
			if (stats_were_valid) {
				_stats_l.scal_bn_safe_range(_lb, _ub);
				_stats_u.scal_bn_safe_range(_lb, _ub);
			}

			auto& x = static_cast<Derived*>(this)->value();
			if (!stats_were_valid) {
				_stats_l.init_stats(x.lower());
				_stats_u.init_stats(x.upper());
				_stats_l.scal_bn_safe_range(_lb, _ub);
				_stats_u.scal_bn_safe_range(_lb, _ub);
			}
			if (0 < scale) {
				if (scale > _ub) return false;
			}
			else {	// if (0 > scale)
				if (scale < _lb) return false;
			}
			x.assign(std::scalbn(x.lower(), scale),std::scalbn(x.upper(), scale));
			if (!stats_were_valid) invalidate_stats();
			return true;
		}	// power-of-two

		virtual fp_API* clone() const {
			auto& x = static_cast<const Derived*>(this)->value();
			const auto tmp = x.lower();
			if (tmp!=x.upper()) return new Derived(*static_cast<const Derived*>(this));
			// singleton...optimize when cloning
			return new var<float,typename _type_of<Derived>::type>(tmp);
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
			auto& x = static_cast<const Derived*>(this)->value();
			auto tmp = x.lower();
			return (0 == tmp || std::isinf(tmp)) && (0 == (tmp = x.upper()) || std::isinf(tmp));
		}

		virtual void scal_bn_safe_range(intmax_t& lb, intmax_t& ub) const {
			// frexp convention: mantissa is [0.5,1.0) and exponent of 1.0 is 1
			auto& x = static_cast<const Derived*>(this)->value();
			if (!_stats_l.valid()) {
				_stats_l.init_stats(x.lower());
				_stats_u.init_stats(x.upper());
			}
			lb = std::numeric_limits<intmax_t>::min();
			ub = std::numeric_limits<intmax_t>::max();
			_stats_l.scal_bn_safe_range(lb, ub);
			_stats_u.scal_bn_safe_range(lb, ub);
		}

		virtual bool scal_bn(intmax_t scale) {
			if (0 == scale) return true;	// no-op
			if (is_scal_bn_identity()) return true;	// no-op

			auto _lb = std::numeric_limits<intmax_t>::min();
			auto _ub = std::numeric_limits<intmax_t>::max();
			const bool stats_were_valid = _stats_l.valid();
			if (stats_were_valid) {
				_stats_l.scal_bn_safe_range(_lb, _ub);
				_stats_u.scal_bn_safe_range(_lb, _ub);
			}

			auto& x = static_cast<Derived*>(this)->value();
			if (!stats_were_valid) {
				_stats_l.init_stats(x.lower());
				_stats_u.init_stats(x.upper());
				_stats_l.scal_bn_safe_range(_lb, _ub);
				_stats_u.scal_bn_safe_range(_lb, _ub);
			}
			if (0 < scale) {
				if (scale > _ub) return false;
			}
			else {	// if (0 > scale)
				if (scale < _lb) return false;
			}
			x.assign(std::scalbn(x.lower(), scale),std::scalbn(x.upper(), scale));
			if (!stats_were_valid) invalidate_stats();
			return true;
		}	// power-of-two

		virtual fp_API* clone() const {
			auto& x = static_cast<const Derived*>(this)->value();
			const auto tmp = x.lower();
			if (tmp!=x.upper()) return new Derived(*static_cast<const Derived*>(this));
			// singleton...optimize when cloning
			return new var<long double,typename _type_of<Derived>::type>(tmp);
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
