#ifndef VAR_HPP
#define VAR_HPP 1

#include "eval.hpp"

// We expect float, double, and long double to work out of the box.
#include <cmath>

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

		void invalidate_stats() { _valid = false; }
		void init_stats(typename const_param<double>::type x) const {
			_mantissa = frexp(x, &_exponent);
			_valid = true;
		}
	};

// we don't want to use a normal macro here as this code is not really expected to be stable
#pragma start_copy interface_of
	template<>
	template<class Derived>
	struct _interface_of<Derived, double, 0> : public virtual fp_API, private _fp_stats<double>
	{
		void invalidate_stats() { _fp_stats<double>::invalidate_stats(); }

		virtual void scal_bn_safe_range(intmax_t& lb, intmax_t& ub) const {
			// frexp convention: mantissa is [0.5,1.0) and exponent of 1.0 is 1
			if (!_valid) init_stats(static_cast<const Derived*>(this)->value());
			lb = std::numeric_limits<double>::min_exponent - _exponent;
			ub = std::numeric_limits<double>::max_exponent - _exponent;
			if (0 < lb) lb = 0;
			if (0 > ub) ub = 0;
		};
		virtual bool scal_bn(intmax_t scale) {
			if (0 == scale) return true;	// no-op
			auto& x = static_cast<Derived*>(this)->value();
			if (!_valid) init_stats(x);
			if (0 < scale) {
				if (scale > std::numeric_limits<double>::max_exponent - _exponent) return false;
			} else {	// if (0 > scale)
				if (scale < std::numeric_limits<double>::min_exponent - _exponent) return false;
			}
			x = std::scalbn(x, scale);
			return true;
		};	// power-of-two

		virtual fp_API* clone() const { return new Derived(*static_cast<const Derived*>(this)); }
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
	struct _interface_of<Derived, float, 0> : public virtual fp_API, private _fp_stats<float>
	{
		void invalidate_stats() { _fp_stats<float>::invalidate_stats(); }

		virtual void scal_bn_safe_range(intmax_t& lb, intmax_t& ub) const {
			// frexp convention: mantissa is [0.5,1.0) and exponent of 1.0 is 1
			if (!_valid) init_stats(static_cast<const Derived*>(this)->value());
			lb = std::numeric_limits<float>::min_exponent - _exponent;
			ub = std::numeric_limits<float>::max_exponent - _exponent;
			if (0 < lb) lb = 0;
			if (0 > ub) ub = 0;
		};
		virtual bool scal_bn(intmax_t scale) {
			if (0 == scale) return true;	// no-op
			auto& x = static_cast<Derived*>(this)->value();
			if (!_valid) init_stats(x);
			if (0 < scale) {
				if (scale > std::numeric_limits<float>::max_exponent - _exponent) return false;
			} else {	// if (0 > scale)
				if (scale < std::numeric_limits<float>::min_exponent - _exponent) return false;
			}
			x = std::scalbn(x, scale);
			return true;
		};	// power-of-two

		virtual fp_API* clone() const { return new Derived(*static_cast<const Derived*>(this)); }
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
	struct _interface_of<Derived, long double, 0> : public virtual fp_API, private _fp_stats<long double>
	{
		void invalidate_stats() { _fp_stats<long double>::invalidate_stats(); }

		virtual void scal_bn_safe_range(intmax_t& lb, intmax_t& ub) const {
			// frexp convention: mantissa is [0.5,1.0) and exponent of 1.0 is 1
			if (!_valid) init_stats(static_cast<const Derived*>(this)->value());
			lb = std::numeric_limits<long double>::min_exponent - _exponent;
			ub = std::numeric_limits<long double>::max_exponent - _exponent;
			if (0 < lb) lb = 0;
			if (0 > ub) ub = 0;
		};
		virtual bool scal_bn(intmax_t scale) {
			if (0 == scale) return true;	// no-op
			auto& x = static_cast<Derived*>(this)->value();
			if (!_valid) init_stats(x);
			if (0 < scale) {
				if (scale > std::numeric_limits<long double>::max_exponent - _exponent) return false;
			} else {	// if (0 > scale)
				if (scale < std::numeric_limits<long double>::min_exponent - _exponent) return false;
			}
			x = std::scalbn(x, scale);
			return true;
		};	// power-of-two

		virtual fp_API* clone() const { return new Derived(*static_cast<const Derived*>(this)); }
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
class var_CRTP : public _type_of<Derived>::type
{
public:
	template<class T> T* get() { return static_cast<Derived*>(this)->template _get(); }
	template<class T> const T* get() const { return static_cast<Derived*>(this)->template _get(); }
};

template<class T, class U= typename _type_of<T>::type>
class var : public var_CRTP<var<T, U> >, public _interface_of<var<T, U>, T, U::API_code>
{
private:
	T _x;
public:
	var() = default;
	var(const T& src) : _x(src) {};
	var(T&& src) : _x(std::move(src)) {};
	var(const var& src) = default;
	var(var&& src) = default;
	~var() = default;
	var& operator=(const T& src) { _x = src; this->invalidate_stats(); return *this; };
	var& operator=(T&& src) { _x = std::move(src); this->invalidate_stats(); return *this; };
	var& operator=(const var& src) = default;
	var& operator=(var&& src) = default;

	T& value() { this->invalidate_stats(); return _x; }
	const T& value() const { return _x; }

	template<class V> typename std::enable_if<!std::is_base_of<V,T>::value, V*>::type _get() { return 0; }
	template<class V> typename std::enable_if<std::is_base_of<V, T>::value, V*>::type _get() { this->invalidate_stats(); return &_x; }
	template<class V> typename std::enable_if<!std::is_base_of<V, T>::value, const V*>::type _get() const { return 0; }
	template<class V> typename std::enable_if<std::is_base_of<V, T>::value, const V*>::type _get() const { return &_x; }
};

template<class T, class U>
struct _type_of<var<T, U> >
{
	typedef U type;
};

}	// namespace zaimoni

#endif
