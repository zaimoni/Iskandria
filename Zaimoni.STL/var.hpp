#ifndef VAR_HPP
#define VAR_HPP 1

#include "eval.hpp"

// We expect float, double, and long double to work out of the box.
#include "augment.STL/cmath"
#include "numeric_error.hpp"

namespace zaimoni {

// we don't want to use a normal macro here as this code is not really expected to be stable
#pragma start_copy interface_of
	template<class Derived>
	struct _interface_of<Derived, double> : public virtual fp_API
	{
	private:
		_fp_stats<double> _stats;
	public:
		void invalidate_stats() { _stats.invalidate_stats(); }

		bool is_scal_bn_identity() const override {
			force_valid_stats();
			return _stats.is_scal_bn_identity();
		}

		std::pair<intmax_t,intmax_t> scal_bn_safe_range() const override {
			// frexp convention: mantissa is [0.5,1.0) and exponent of 1.0 is 1
			force_valid_stats();
			std::pair<intmax_t, intmax_t> ret(fp_API::max_scal_bn_safe_range());
			_stats.scal_bn_safe_range(ret.first, ret.second);
			return ret;
		}

		intmax_t ideal_scal_bn() const override {
			if (is_scal_bn_identity()) return 0;
			return _stats.ideal_scal_bn();
		}

		fp_API* clone() const override { return new Derived(*static_cast<const Derived*>(this)); }
	private:
		void force_valid_stats() const {
			if (!_stats.valid()) _stats.init_stats(static_cast<const Derived*>(this)->value());
		}
		void _scal_bn(intmax_t scale) override {
			auto& x = static_cast<Derived*>(this)->value();
			x = std::scalbn(x, scale);
		}	// power-of-two
		fp_API* _eval() const override { return 0; }
		bool _is_inf() const override { return std::isinf(static_cast<const Derived*>(this)->value()); }
		bool _is_finite() const override { return std::isfinite(static_cast<const Derived*>(this)->value()); }
	};
#pragma end_copy
#pragma for F in float,long double
#pragma substitute $F for double in interface_of
	template<class Derived>
	struct _interface_of<Derived, float> : public virtual fp_API
	{
	private:
		_fp_stats<float> _stats;
	public:
		void invalidate_stats() { _stats.invalidate_stats(); }

		bool is_scal_bn_identity() const override {
			force_valid_stats();
			return _stats.is_scal_bn_identity();
		}

		std::pair<intmax_t,intmax_t> scal_bn_safe_range() const override {
			// frexp convention: mantissa is [0.5,1.0) and exponent of 1.0 is 1
			force_valid_stats();
			std::pair<intmax_t, intmax_t> ret(fp_API::max_scal_bn_safe_range());
			_stats.scal_bn_safe_range(ret.first, ret.second);
			return ret;
		}

		intmax_t ideal_scal_bn() const override {
			if (is_scal_bn_identity()) return 0;
			return _stats.ideal_scal_bn();
		}

		fp_API* clone() const override { return new Derived(*static_cast<const Derived*>(this)); }
	private:
		void force_valid_stats() const {
			if (!_stats.valid()) _stats.init_stats(static_cast<const Derived*>(this)->value());
		}
		void _scal_bn(intmax_t scale) override {
			auto& x = static_cast<Derived*>(this)->value();
			x = std::scalbn(x, scale);
		}	// power-of-two
		fp_API* _eval() const override { return 0; }
		bool _is_inf() const override { return std::isinf(static_cast<const Derived*>(this)->value()); }
		bool _is_finite() const override { return std::isfinite(static_cast<const Derived*>(this)->value()); }
	};

	template<class Derived>
	struct _interface_of<Derived, long double> : public virtual fp_API
	{
	private:
		_fp_stats<long double> _stats;
	public:
		void invalidate_stats() { _stats.invalidate_stats(); }

		bool is_scal_bn_identity() const override {
			force_valid_stats();
			return _stats.is_scal_bn_identity();
		}

		std::pair<intmax_t,intmax_t> scal_bn_safe_range() const override {
			// frexp convention: mantissa is [0.5,1.0) and exponent of 1.0 is 1
			force_valid_stats();
			std::pair<intmax_t, intmax_t> ret(fp_API::max_scal_bn_safe_range());
			_stats.scal_bn_safe_range(ret.first, ret.second);
			return ret;
		}

		intmax_t ideal_scal_bn() const override {
			if (is_scal_bn_identity()) return 0;
			return _stats.ideal_scal_bn();
		}

		fp_API* clone() const override { return new Derived(*static_cast<const Derived*>(this)); }
	private:
		void force_valid_stats() const {
			if (!_stats.valid()) _stats.init_stats(static_cast<const Derived*>(this)->value());
		}
		void _scal_bn(intmax_t scale) override {
			auto& x = static_cast<Derived*>(this)->value();
			x = std::scalbn(x, scale);
		}	// power-of-two
		fp_API* _eval() const override { return 0; }
		bool _is_inf() const override { return std::isinf(static_cast<const Derived*>(this)->value()); }
		bool _is_finite() const override { return std::isfinite(static_cast<const Derived*>(this)->value()); }
	};
#pragma end_substitute
#pragma done

}	// namespace zaimoni

// close-reopen to delineate support overrides from the main class

namespace zaimoni {

template<class Derived>
class var_CRTP : public type_of_t<Derived>	// \todo eliminate this if it becomes clear it's not useful
{
public:
	template<class T> T* get() { return static_cast<Derived*>(this)->template _get(); }
	template<class T> const T* get() const { return static_cast<Derived*>(this)->template _get(); }
};

template<class T, class U= type_of_t<T> >
class var : public var_CRTP<var<T, U> >, public _interface_of<var<T, U>, T>, public _access<T>
{
private:
	T _x;
public:
	var() = default;
	var(const T& src) : _x(src) {
		const auto err = _constructor_fatal();
		if (err) throw zaimoni::math::numeric_error(err);
	}
	var(T&& src) : _x(std::move(src)) {
		const auto err = _constructor_fatal();
		if (err) throw zaimoni::math::numeric_error(err);
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

namespace bits {

	template<class T, class U> struct _type_of<var<T, U> > {
		static_assert(std::is_base_of_v<zaimoni::math::type, U>);
		typedef U type;
	};

}

}	// namespace zaimoni

// close/re-open to separate legacy implementation from experimental implementation

// above is not Kuroda-grammar friendly.  Try to do what Franci does here
namespace zaimoni {

	namespace detail {

		template<class T> struct var_fp_impl;

		template<>
		struct var_fp_impl<double> {
			using param_type = double;

			static const math::type* domain(const param_type& x)  {
				if (std::isnan(x)) return nullptr;
				if (std::isinf(x)) return &zaimoni::math::get<_type<_type_spec::_R_SHARP_>>();
				return &zaimoni::math::get<_type<_type_spec::_R_>>();
			}
			static constexpr bool self_eval(const param_type& x) { return false; }
			static constexpr int sgn(const param_type& x) { return 0 < x ? 1 : (0 > x ? -1 : 0); }
			static std::string to_s(const param_type& x) { return std::to_string(x); }
			static bool is_scal_bn_identity(const param_type& x) {
				return _fp_stats<param_type>(x).is_scal_bn_identity();
			}
			static std::pair<intmax_t, intmax_t> scal_bn_safe_range(const param_type& x) {
				std::pair<intmax_t, intmax_t> ret(fp_API::max_scal_bn_safe_range());
				_fp_stats<param_type>(x).scal_bn_safe_range(ret.first, ret.second);
				return ret;
			}
			static intmax_t ideal_scal_bn(const param_type& x) {
				return _fp_stats<param_type>(x).ideal_scal_bn();
			}
			static fp_API* clone(const param_type& x) { return nullptr; }
			static void _scal_bn(param_type& x, intmax_t scale) { x = std::scalbn(x, scale); }
			static constexpr fp_API* _eval(const param_type& x) { return nullptr; }
		};

		template<>
		struct var_fp_impl<float> {
			using param_type = float;

			static const math::type* domain(const param_type& x) {
				if (std::isnan(x)) return nullptr;
				if (std::isinf(x)) return &zaimoni::math::get<_type<_type_spec::_R_SHARP_>>();
				return &zaimoni::math::get<_type<_type_spec::_R_>>();
			}
			static constexpr bool self_eval(const param_type& x) { return false; }
			static constexpr int sgn(const param_type& x) { return 0 < x ? 1 : (0 > x ? -1 : 0); }
			static std::string to_s(const param_type& x) { return std::to_string(x); }
			static bool is_scal_bn_identity(const param_type& x) {
				return _fp_stats<param_type>(x).is_scal_bn_identity();
			}
			static std::pair<intmax_t, intmax_t> scal_bn_safe_range(const param_type& x) {
				std::pair<intmax_t, intmax_t> ret(fp_API::max_scal_bn_safe_range());
				_fp_stats<param_type>(x).scal_bn_safe_range(ret.first, ret.second);
				return ret;
			}
			static intmax_t ideal_scal_bn(const param_type& x) {
				return _fp_stats<param_type>(x).ideal_scal_bn();
			}
			static fp_API* clone(const param_type& x) { return nullptr; }
			static void _scal_bn(param_type& x, intmax_t scale) { x = std::scalbn(x, scale); }
			static constexpr fp_API* _eval(const param_type& x) { return nullptr; }
		};

		template<>
		struct var_fp_impl<long double> {
			using param_type = long double;

			static const math::type* domain(const param_type& x) {
				if (std::isnan(x)) return nullptr;
				if (std::isinf(x)) return &zaimoni::math::get<_type<_type_spec::_R_SHARP_>>();
				return &zaimoni::math::get<_type<_type_spec::_R_>>();
			}
			static constexpr bool self_eval(const param_type& x) { return false; }
			static constexpr int sgn(const param_type& x) { return 0 < x ? 1 : (0 > x ? -1 : 0); }
			static std::string to_s(const param_type& x) { return std::to_string(x); }
			static bool is_scal_bn_identity(const param_type& x) {
				return _fp_stats<param_type>(x).is_scal_bn_identity();
			}
			static std::pair<intmax_t, intmax_t> scal_bn_safe_range(const param_type& x) {
				std::pair<intmax_t, intmax_t> ret(fp_API::max_scal_bn_safe_range());
				_fp_stats<param_type>(x).scal_bn_safe_range(ret.first, ret.second);
				return ret;
			}
			static intmax_t ideal_scal_bn(const param_type& x) {
				return _fp_stats<param_type>(x).ideal_scal_bn();
			}
			static fp_API* clone(const param_type& x) { return nullptr; }
			static void _scal_bn(param_type& x, intmax_t scale) { x = std::scalbn(x, scale); }
			static constexpr fp_API* _eval(const param_type& x) { return nullptr; }
		};

	}

	template<class T>
	class var_fp final : public fp_API // "variable, floating-point"
	{
	public:
		T _x;	// we would provide full accessors anyway so may as well be public

		var_fp() = default;
		var_fp(const T& src) noexcept(std::is_nothrow_copy_constructible_v<T>) : _x(src) {}
		var_fp(const var_fp& src) = default;
		var_fp(var_fp&& src) = default;
		virtual ~var_fp() = default;
		var_fp& operator=(const var_fp& src) = default;
		var_fp& operator=(var_fp&& src) = default;
		var_fp& operator=(const T& src) noexcept(std::is_nothrow_copy_assignable_v<T>) {
			_x = src;	// presumably this is ACID
			return *this;
		};

		// placeholders
		const math::type* domain() const override { return detail::var_fp_impl<T>::domain(_x); }
		// implemented
		bool self_eval() override { return detail::var_fp_impl<T>::self_eval(_x); }
		bool is_zero() const override { return zaimoni::is_zero(_x); }
		bool is_one() const override { return zaimoni::is_one(_x); }
		int sgn() const override { return detail::var_fp_impl<T>::sgn(_x); }
		// scalbn: scale by power of 2.  Important operation as it's infinite-precision (when it works)
		bool is_scal_bn_identity() const override { return detail::var_fp_impl<T>::is_scal_bn_identity(_x); }
		std::pair<intmax_t, intmax_t> scal_bn_safe_range() const override { return detail::var_fp_impl<T>::scal_bn_safe_range(_x); }
		intmax_t ideal_scal_bn() const override { return detail::var_fp_impl<T>::ideal_scal_bn(_x); }

		// technical infrastructure
		fp_API* clone() const override {
			if (fp_API* test = detail::var_fp_impl<T>::clone(_x)) return test;
			return new var_fp(_x);
		}
		std::string to_s() const override { return detail::var_fp_impl<T>::to_s(_x); }
		int precedence() const override { return std::numeric_limits<int>::max(); }	// things like numerals generally outrank all operators

	private:
		void _scal_bn(intmax_t scale) override { return detail::var_fp_impl<T>::_scal_bn(_x, scale); }	// power-of-two
		fp_API* _eval() const override { return detail::var_fp_impl<T>::_eval(_x); }	// memory-allocating evaluation
	};

}

#ifdef _INTERVAL_HPP
#include "bits/_interval_var.hpp"
#endif
#ifdef ANGLE_HPP
#include "bits/_angle_var.hpp"
#endif

#endif
