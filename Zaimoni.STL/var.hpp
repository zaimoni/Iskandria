#ifndef VAR_HPP
#define VAR_HPP 1

#include "eval.hpp"

// We expect float, double, and long double to work out of the box.
#include <cmath>
#include <stdexcept>

namespace zaimoni {

	template<class T>
	class _fp_stats	// odd name avoids conflicting with an earlier prototype in Iskandria's overprecise.hpp
	{
	ZAIMONI_STATIC_ASSERT(std::is_floating_point<T>::value);
	protected:
		mutable T _mantissa;
		mutable int _exponent;
		mutable bool _valid;
	public:
		_fp_stats() : _valid(false) {};
		_fp_stats(typename const_param<T>::type src) : _valid(true) {
			_mantissa = frexp(src, &_exponent);
		};

		void invalidate_stats() { _valid = false; }
		bool valid() const { return _valid; }
		void init_stats(typename const_param<double>::type x) const {
			_mantissa = frexp(x, &_exponent);
			_valid = true;
		}

		// precondition: valid
		// precondition: incoming lb/ub are initialized appropriately
		void scal_bn_safe_range(intmax_t& lb, intmax_t& ub) const {
			if (0 == _mantissa) return;
			auto _lb = std::numeric_limits<T>::min_exponent - _exponent;
			auto _ub = std::numeric_limits<T>::max_exponent - _exponent;
			if (0 < _lb) _lb = 0;
			if (0 > _ub) _ub = 0;
			if (lb < _lb) lb = _lb;
			if (ub > _ub) ub = _ub;
		}

		// precondition: valid
		bool is_scal_bn_identity() const { return std::numeric_limits<T>::max_exponent < _exponent; }	// 0, infinity, or NaN

		// precondition: valid
		intmax_t ideal_scal_bn() const { return is_scal_bn_identity() ? 0 : _exponent - 1; }
	};

// we don't want to use a normal macro here as this code is not really expected to be stable
#pragma start_copy interface_of
	template<>
	template<class Derived>
	struct _interface_of<Derived, double, 0> : public virtual fp_API
	{
	private:
		_fp_stats<double> _stats;
	public:
		void invalidate_stats() { _stats.invalidate_stats(); }

		virtual bool is_scal_bn_identity() const {
			force_valid_stats();
			return _stats.is_scal_bn_identity();
		}

		virtual std::pair<intmax_t,intmax_t> scal_bn_safe_range() const {
			// frexp convention: mantissa is [0.5,1.0) and exponent of 1.0 is 1
			force_valid_stats();
			std::pair<intmax_t, intmax_t> ret(fp_API::max_scal_bn_safe_range);
			_stats.scal_bn_safe_range(ret.first, ret.second);
			return ret;
		}

		virtual intmax_t ideal_scal_bn() const {
			if (is_scal_bn_identity()) return 0;
			return _stats.ideal_scal_bn();
		}

		virtual fp_API* clone() const { return new Derived(*static_cast<const Derived*>(this)); }
	private:
		void force_valid_stats() const {
			if (!_stats.valid()) _stats.init_stats(static_cast<const Derived*>(this)->value());
		}
		virtual bool _scal_bn(intmax_t scale) {
			auto& x = static_cast<Derived*>(this)->value();
			x = std::scalbn(x, scale);
			return true;
		}	// power-of-two
	};

	template<>
	template<class Derived>
	struct _interface_of<Derived, double, 1> : public _interface_of<Derived, double, 0>
	{
		virtual bool is_inf() const { return std::isinf(static_cast<const Derived*>(this)->value()); };
		virtual bool is_finite() const { return std::isfinite(static_cast<const Derived*>(this)->value()); };
	};
#pragma end_copy
#pragma substitute float for double in interface_of
	template<>
	template<class Derived>
	struct _interface_of<Derived, float, 0> : public virtual fp_API
	{
	private:
		_fp_stats<float> _stats;
	public:
		void invalidate_stats() { _stats.invalidate_stats(); }

		virtual bool is_scal_bn_identity() const {
			force_valid_stats();
			return _stats.is_scal_bn_identity();
		}

		virtual std::pair<intmax_t,intmax_t> scal_bn_safe_range() const {
			// frexp convention: mantissa is [0.5,1.0) and exponent of 1.0 is 1
			force_valid_stats();
			std::pair<intmax_t, intmax_t> ret(fp_API::max_scal_bn_safe_range);
			_stats.scal_bn_safe_range(ret.first, ret.second);
			return ret;
		}

		virtual intmax_t ideal_scal_bn() const {
			if (is_scal_bn_identity()) return 0;
			return _stats.ideal_scal_bn();
		}

		virtual fp_API* clone() const { return new Derived(*static_cast<const Derived*>(this)); }
	private:
		void force_valid_stats() const {
			if (!_stats.valid()) _stats.init_stats(static_cast<const Derived*>(this)->value());
		}
		virtual bool _scal_bn(intmax_t scale) {
			auto& x = static_cast<Derived*>(this)->value();
			x = std::scalbn(x, scale);
			return true;
		}	// power-of-two
	};

	template<>
	template<class Derived>
	struct _interface_of<Derived, float, 1> : public _interface_of<Derived, float, 0>
	{
		virtual bool is_inf() const { return std::isinf(static_cast<const Derived*>(this)->value()); };
		virtual bool is_finite() const { return std::isfinite(static_cast<const Derived*>(this)->value()); };
	};
#pragma end_substitute
#pragma substitute long double for double in interface_of
	template<>
	template<class Derived>
	struct _interface_of<Derived, long double, 0> : public virtual fp_API
	{
	private:
		_fp_stats<long double> _stats;
	public:
		void invalidate_stats() { _stats.invalidate_stats(); }

		virtual bool is_scal_bn_identity() const {
			force_valid_stats();
			return _stats.is_scal_bn_identity();
		}

		virtual std::pair<intmax_t,intmax_t> scal_bn_safe_range() const {
			// frexp convention: mantissa is [0.5,1.0) and exponent of 1.0 is 1
			force_valid_stats();
			std::pair<intmax_t, intmax_t> ret(fp_API::max_scal_bn_safe_range);
			_stats.scal_bn_safe_range(ret.first, ret.second);
			return ret;
		}

		virtual intmax_t ideal_scal_bn() const {
			if (is_scal_bn_identity()) return 0;
			return _stats.ideal_scal_bn();
		}

		virtual fp_API* clone() const { return new Derived(*static_cast<const Derived*>(this)); }
	private:
		void force_valid_stats() const {
			if (!_stats.valid()) _stats.init_stats(static_cast<const Derived*>(this)->value());
		}
		virtual bool _scal_bn(intmax_t scale) {
			auto& x = static_cast<Derived*>(this)->value();
			x = std::scalbn(x, scale);
			return true;
		}	// power-of-two
	};

	template<>
	template<class Derived>
	struct _interface_of<Derived, long double, 1> : public _interface_of<Derived, long double, 0>
	{
		virtual bool is_inf() const { return std::isinf(static_cast<const Derived*>(this)->value()); };
		virtual bool is_finite() const { return std::isfinite(static_cast<const Derived*>(this)->value()); };
	};
#pragma end_substitute

}	// namespace zaimoni

// close-reopen to delineate support overrides from the main class

namespace zaimoni {

template<class Derived>
class var_CRTP : public _type_of<Derived>::type	// \todo eliminate this if it becomes clear it's not useful
{
public:
	template<class T> T* get() { return static_cast<Derived*>(this)->template _get(); }
	template<class T> const T* get() const { return static_cast<Derived*>(this)->template _get(); }
};

template<class T, class U= typename _type_of<T>::type>
class var : public var_CRTP<var<T, U> >, public _interface_of<var<T, U>, T, U::API_code>, public _access<T>
{
private:
	T _x;
public:
	var() = default;
	var(const T& src) : _x(src) {
		const auto err = _constructor_fatal();
		if (err) throw std::runtime_error(err);
	}
	var(T&& src) : _x(std::move(src)) {
		const auto err = _constructor_fatal();
		if (err) throw std::runtime_error(err);
	}
	var(const var& src) = default;
	var(var&& src) = default;
	~var() = default;
	var& operator=(const T& src) { _x = src; this->invalidate_stats(); return *this; };
	var& operator=(T&& src) { _x = std::move(src); this->invalidate_stats(); return *this; };
	var& operator=(const var& src) = default;
	var& operator=(var&& src) = default;

	virtual T& value() { this->invalidate_stats(); return _x; }
	virtual const T& value() const { return _x; }

	template<class V> typename std::enable_if<!std::is_base_of<V,T>::value, V*>::type _get() { return 0; }
	template<class V> typename std::enable_if<std::is_base_of<V, T>::value, V*>::type _get() { this->invalidate_stats(); return &_x; }
	template<class V> typename std::enable_if<!std::is_base_of<V, T>::value, const V*>::type _get() const { return 0; }
	template<class V> typename std::enable_if<std::is_base_of<V, T>::value, const V*>::type _get() const { return &_x; }

private:
	const char* _constructor_fatal() const {
		if (!this->allow_infinity() && isINF(_x)) return "infinity in finite variable";
		if (isNaN(_x)) return "NaN in variable";
		return 0;
	}
};

template<class T, class U>
struct _type_of<var<T, U> >
{
	typedef U type;
};

}	// namespace zaimoni

#endif
