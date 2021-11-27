#ifndef VAR_HPP
#define VAR_HPP 1

#include "eval.hpp"

// We expect float, double, and long double to work out of the box.
#include "augment.STL/cmath"
#include "numeric_error.hpp"
#include <charconv>

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
			static constexpr int sgn(const param_type& x) { return 0 < x ? 1 : (0 > x ? -1 : 0); }
			static std::string to_s(const param_type& x) {
				char buffer[30];
				auto ret = std::to_chars(std::begin(buffer), std::end(buffer), x);
				*ret.ptr = 0;
				return std::string(buffer);
			}
			static bool is_scal_bn_identity(const param_type& x) {
				return _fp_stats<param_type>(x).is_scal_bn_identity();
			}
			static intmax_t scal_bn_is_safe(const param_type& x, intmax_t scale) {
				const auto span(scal_bn_safe_range(x));
				if (0 < scale) {
					return span.second < scale ? span.second : scale;
				} else /* if (0 > scale) */ {
					return span.first > scale ? span.first : scale;
				}
			}
			static intmax_t ideal_scal_bn(const param_type& x) {
				return _fp_stats<param_type>(x).ideal_scal_bn();
			}
			static constexpr fp_API* clone() { return nullptr; }
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
			static constexpr int sgn(const param_type& x) { return 0 < x ? 1 : (0 > x ? -1 : 0); }
			static std::string to_s(const param_type& x) {
				char buffer[30];
				auto ret = std::to_chars(std::begin(buffer), std::end(buffer), x);
				*ret.ptr = 0;
				return std::string(buffer);
			}
			static bool is_scal_bn_identity(const param_type& x) {
				return _fp_stats<param_type>(x).is_scal_bn_identity();
			}
			static intmax_t scal_bn_is_safe(const param_type& x, intmax_t scale) {
				const auto span(scal_bn_safe_range(x));
				if (0 < scale) {
					return span.second < scale ? span.second : scale;
				} else /* if (0 > scale) */ {
					return span.first > scale ? span.first : scale;
				}
			}
			static intmax_t ideal_scal_bn(const param_type& x) {
				return _fp_stats<param_type>(x).ideal_scal_bn();
			}
			static constexpr fp_API* clone() { return nullptr; }
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
			static constexpr int sgn(const param_type& x) { return 0 < x ? 1 : (0 > x ? -1 : 0); }
			static std::string to_s(const param_type& x) {
				char buffer[30];
				auto ret = std::to_chars(std::begin(buffer), std::end(buffer), x);
				*ret.ptr = 0;
				return std::string(buffer);
			}
			static bool is_scal_bn_identity(const param_type& x) {
				return _fp_stats<param_type>(x).is_scal_bn_identity();
			}
			static intmax_t scal_bn_is_safe(const param_type& x, intmax_t scale) {
				const auto span(scal_bn_safe_range(x));
				if (0 < scale) {
					return span.second < scale ? span.second : scale;
				} else /* if (0 > scale) */ {
					return span.first > scale ? span.first : scale;
				}
			}
			static intmax_t ideal_scal_bn(const param_type& x) {
				return _fp_stats<param_type>(x).ideal_scal_bn();
			}
			static fp_API* clone(const param_type& x) { return nullptr; }
			static constexpr fp_API* clone() { return nullptr; }
			static void _scal_bn(param_type& x, intmax_t scale) { x = std::scalbn(x, scale); }
			static constexpr fp_API* _eval(const param_type& x) { return nullptr; }
		};


		template<>
		struct var_fp_impl<signed long long> {
			using param_type = signed long long;

			static const math::type* domain(const param_type& x) {
				return &zaimoni::math::get<_type<_type_spec::_Z_>>();
			}
			static constexpr int sgn(const param_type& x) { return 0 < x ? 1 : (0 > x ? -1 : 0); }
			static std::string to_s(const param_type& x) { return std::to_string(x); }
			static bool is_scal_bn_identity(const param_type& x) {
				return _fp_stats<param_type>(x).is_scal_bn_identity();
			}
			static intmax_t scal_bn_is_safe(const param_type& x, intmax_t scale) {
				const auto span(scal_bn_safe_range(x));
				if (0 < scale) {
					return span.second < scale ? span.second : scale;
				} else /* if (0 > scale) */ {
					return span.first > scale ? span.first : scale;
				}
			}
			static intmax_t ideal_scal_bn(const param_type& x) {
				return _fp_stats<param_type>(x).ideal_scal_bn();
			}
			static constexpr fp_API* clone() { return nullptr; }
			static void _scal_bn(param_type& x, intmax_t scale) {
				if (0 > scale) {
					x >>= -scale;
				} else /* if (0 <= scale) */ {
					x <<= scale;
				}
			}
			static constexpr fp_API* _eval(const param_type& x) { return nullptr; }
		};

		template<>
		struct var_fp_impl<unsigned long long> {
			using param_type = unsigned long long;

			static const math::type* domain(const param_type& x) {
				return &zaimoni::math::get<_type<_type_spec::_Z_>>();
			}
			static constexpr int sgn(const param_type& x) { return 0 < x ? 1 : 0; }
			static std::string to_s(const param_type& x) { return std::to_string(x); }
			static bool is_scal_bn_identity(const param_type& x) {
				return _fp_stats<param_type>(x).is_scal_bn_identity();
			}
			static intmax_t scal_bn_is_safe(const param_type& x, intmax_t scale) {
				const auto span(scal_bn_safe_range(x));
				if (0 < scale) {
					return span.second < scale ? span.second : scale;
				} else /* if (0 > scale) */ {
					return span.first > scale ? span.first : scale;
				}
			}
			static intmax_t ideal_scal_bn(const param_type& x) {
				return _fp_stats<param_type>(x).ideal_scal_bn();
			}
			static constexpr fp_API* clone() { return nullptr; }
			static void _scal_bn(param_type& x, intmax_t scale) {
				if (0 > scale) {
					x >>= -scale;
				} else /* if (0 <= scale) */ {
					x <<= scale;
				}
			}
			static constexpr fp_API* _eval(const param_type& x) { return nullptr; }
		};
	}

	template<class T>
	class var_fp final : public fp_API // "variable, floating-point" (actually value as we aren't tracking display name)
	{
		static_assert(!std::is_base_of_v<fp_API, T>);
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

		const math::type* domain() const override { return detail::var_fp_impl<T>::domain(_x); }
		constexpr bool self_eval() override { return false; }
		bool is_zero() const override { return zaimoni::is_zero(_x); }
		bool is_one() const override { return zaimoni::is_one(_x); }
		int sgn() const override { return detail::var_fp_impl<T>::sgn(_x); }
		// scalbn: scale by power of 2.  Important operation as it's infinite-precision (when it works)
		bool is_scal_bn_identity() const override { return detail::var_fp_impl<T>::is_scal_bn_identity(_x); }
		intmax_t scal_bn_is_safe(intmax_t scale) const override { return detail::var_fp_impl<T>::scal_bn_is_safe(_x, scale); }
		intmax_t ideal_scal_bn() const override { return detail::var_fp_impl<T>::ideal_scal_bn(_x); }

		// technical infrastructure
		fp_API* clone() const override {
			if constexpr (requires { detail::var_fp_impl<T>::clone(_x); }) {
				if (fp_API* test = detail::var_fp_impl<T>::clone(_x)) return test;
			}
			return new var_fp(_x);
		}
		auto typed_clone() const requires requires() { detail::var_fp_impl<T>::clone(); } { return new var_fp(_x); }

		std::string to_s() const override { return detail::var_fp_impl<T>::to_s(_x); }
		int precedence() const override { return std::numeric_limits<int>::max(); }	// things like numerals generally outrank all operators

	private:
		void _scal_bn(intmax_t scale) override { return detail::var_fp_impl<T>::_scal_bn(_x, scale); }	// power-of-two
		fp_API* _eval() const override { return detail::var_fp_impl<T>::_eval(_x); }	// memory-allocating evaluation
		std::partial_ordering _value_compare(const fp_API* rhs) const override {
			if (const auto mine = dynamic_cast<decltype(this)>(rhs)) {
				// \todo: use requires clauses to control code paths
				if constexpr (requires { _x <=> mine->_x; }) return _x <=> mine->_x;
				else if constexpr (requires { _x == mine->_x; }) {
					if (_x == mine->_x) return std::partial_ordering::equivalent;
				}
			}
			return std::partial_ordering::unordered;
		}
	};

}

#ifdef _INTERVAL_HPP
#include "bits/_interval_var.hpp"
#endif
#ifdef ANGLE_HPP
#include "bits/_angle_var.hpp"
#endif

#endif
