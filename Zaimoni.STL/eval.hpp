#ifndef EVAL_HPP
#define EVAL_HPP 1

#include <limits.h>
#include <limits>
#include <utility>
#include <string>
#include <compare>
#include <stdexcept>
#include "augment.STL/type_traits"
#include "zero.hpp"
#include "COW.hpp"

namespace zaimoni {

	using std::to_string;
	using std::swap;

	// Yet another take on higher-mathematics typing.
	// Cf. https://arxiv.org/abs/math/0105155v4 for what makes octonions useful to direct-model
	struct _type_spec {
		// hierarchy of "1-dimensional" infinite-precision types
		enum arch_domain {
			_Z_ = 1,	// integers (usual set-theoretic anchor, can build all others from this)
			_Q_,		// rational numbers
			_R_,		// real numbers (alternate anchor, can build all others from this)
			_C_,		// complex numbers
			_H_,		// quaternions
			_O_,		// octonions
			_R_SHARP_,	// extended real numbers
			_C_SHARP_,	// extended complex numbers
			_H_SHARP_,	// extended quaternions
			_O_SHARP_,	// extended octonions
			_S1_		// unit circle
		};
		enum canonical_functions { // also precedence values
			Addition = 1,
			Multiplication
		};
	};
	// while we want to support vector spaces, matrices, etc., that looks it like it requires more general methods than an operation enum
	// e.g., consider a 2-dimensional vector space of operations on the surface of a 3-dimesnional sphere
	namespace math {
		struct type {
			virtual ~type() = default;
			virtual int allow_infinity() const = 0;	// 0: no; -1: signed; 1 unsigned
			virtual bool is_totally_ordered() const = 0;
			std::partial_ordering subclass(const type& rhs) const { return rhs._superclass(this); }
			// evaluate type of canonical operations (generally binary functions)
			virtual const type* self(_type_spec::canonical_functions op) const = 0;
			type* self(_type_spec::canonical_functions op) { return const_cast<type*>(const_cast<const type*>(this)->self(op)); }
			// Abstract algebra modules must override these to recognize what can multiply them, that isn't they themselves.  This default just checks for
			// subobjects.
			virtual const type* left(_type_spec::canonical_functions op, const type& rhs) const {
				auto staging = self(op);
				if (!staging) return nullptr;
				return 0 >= rhs.subclass(*staging) ? this : nullptr;
			}
			type* left(_type_spec::canonical_functions op, const type& rhs) { return const_cast<type*>(const_cast<const type*>(this)->left(op, rhs)); }
			virtual const type* right(_type_spec::canonical_functions op, const type& lhs) const {
				auto staging = self(op);
				if (!staging) return nullptr;
				return 0 >= lhs.subclass(*staging) ? this : nullptr;
			}
			type* right(_type_spec::canonical_functions op, const type& lhs) { return const_cast<type*>(const_cast<const type*>(this)->right(op, lhs)); }
			// most mathematical objects are closed under inverting the operations they support
			virtual const type* inverse(_type_spec::canonical_functions op) const { return self(op); }
			type* inverse(_type_spec::canonical_functions op) { return const_cast<type*>(const_cast<const type*>(this)->inverse(op)); }

			static const type* defined(const type& lhs, _type_spec::canonical_functions op, const type& rhs) {
				if (auto test = lhs.left(op, rhs)) return test;
				if (auto test = rhs.right(op, lhs)) return test;
				return nullptr;
			}
			static type* defined(type& lhs, _type_spec::canonical_functions op, type& rhs) {
				if (auto test = lhs.left(op, rhs)) return test;
				if (auto test = rhs.right(op, lhs)) return test;
				return nullptr;
			}

		private:
			virtual std::partial_ordering _superclass(const type* rhs) const {
				const bool nonstrict_subclass = rhs->_nonstrictSuperclass(this);
				if (_nonstrictSuperclass(rhs)) return nonstrict_subclass ? std::partial_ordering::equivalent : std::partial_ordering::less;
				return nonstrict_subclass ? std::partial_ordering::greater : std::partial_ordering::unordered;
			}
			virtual bool _nonstrictSuperclass(const type* rhs) const = 0;
		};	// tag so we can do template validation

		// allow non-const in case we need something like an attribute-value type for real-time theorems 2020-09-18 zaimoni
		template<class T>
		std::enable_if_t<std::is_base_of_v<type, T>, T&> get() {
			static T ooao;
			return ooao;
		}
	}

	template<_type_spec::arch_domain DOM> struct _type {
		static_assert(unconditional_v<bool, false, DOM>, "must specialize this");
	};

	// C#-style interfaces
	template<class T>
	struct eval_to_ptr
	{
		using eval_type = COW<T>;

		virtual ~eval_to_ptr() = default;
		virtual eval_type destructive_eval() = 0; // exact
		virtual bool algebraic_self_eval() = 0; // also exact
		virtual bool inexact_self_eval() = 0;
	};

	template<class T>
	struct API_sum
	{
		virtual ~API_sum() = default;
		virtual int rearrange_sum(eval_to_ptr<T>::eval_type& rhs) = 0;
		virtual T* eval_sum(const typename eval_to_ptr<T>::eval_type& rhs) const = 0;
		virtual int score_sum(const typename eval_to_ptr<T>::eval_type& rhs) const = 0;
	};

	struct API_addinv
	{
		virtual ~API_addinv() = default;
		virtual void self_negate() = 0;
	};

	template<class T>
	struct API_product
	{
		virtual ~API_product() = default;
		virtual int rearrange_product(eval_to_ptr<T>::eval_type& rhs) = 0;
		virtual T* eval_product(const typename eval_to_ptr<T>::eval_type& rhs) const = 0;
		virtual std::optional<std::pair<int, int> > product_op_count(const typename eval_to_ptr<T>::eval_type& rhs) const = 0;
	};

	// this is right-division, if there is a distinction; it supports a/b := a b^-1 notation
	template<class T>
	struct API_productinv
	{
		virtual ~API_productinv() = default;
		virtual int rearrange_divides(eval_to_ptr<T>::eval_type& lhs) = 0;
		virtual int rearrange_dividedby(eval_to_ptr<T>::eval_type& rhs) = 0;
		virtual T* eval_divides(const typename eval_to_ptr<T>::eval_type& lhs) const = 0;
		virtual T* eval_dividedby(const typename eval_to_ptr<T>::eval_type& rhs) const = 0;
	};

	struct fp_API {	// virtual base
		static constexpr std::pair<intmax_t, intmax_t> max_scal_bn_safe_range() { return std::pair<intmax_t, intmax_t>(std::numeric_limits<intmax_t>::min(), std::numeric_limits<intmax_t>::max()); }	// simple static member variable crashes at link-time even if initialized here

		virtual ~fp_API() = default;

		/// <summary>
		/// Run-time mathematical type system.  Reference return interferes with n-ary operation domain estimation
		/// </summary>
		/// <returns>non-null, or throws std::logic_error</returns>
		virtual const math::type* domain() const = 0; // for Kuroda grammar approach

		virtual bool self_eval() = 0;

		static bool algebraic_reduce(eval_to_ptr<fp_API>::eval_type& dest) {
			if (auto efficient = dest.get_rw<eval_to_ptr<fp_API> >()) {
				if (!efficient->first) efficient->first = dynamic_cast<eval_to_ptr<fp_API>*>(dest.get());
				if (efficient->first) {
					if (auto result = efficient->first->destructive_eval()) {
						dest = std::move(result);
						return true;
					}
					if (efficient->first->algebraic_self_eval()) return true;
				}
			}
			return false;
		}

		static bool inexact_reduce(eval_to_ptr<fp_API>::eval_type& dest) {
			if (auto efficient = dest.get_rw<eval_to_ptr<fp_API> >()) {
				if (!efficient->first) efficient->first = dynamic_cast<eval_to_ptr<fp_API>*>(dest.get());
				if (efficient->first) {
					if (efficient->first->inexact_self_eval()) return true;
				}
			} else if (auto efficient = dest.get_rw<fp_API>()) {
				if (!efficient->first) efficient->first = dynamic_cast<fp_API*>(dest.get());
				if (efficient->first) {
					if (efficient->first->self_eval()) return true;
				}
			}
			if (auto result = dest->_eval()) {
				dest = std::unique_ptr<fp_API>(result);
				return true;
			}
			return false;
		}

		static bool eval(eval_to_ptr<fp_API>::eval_type& dest) {
			// \todo? micro-optimize by inlining
			if (algebraic_reduce(dest)) return true;
			if (inexact_reduce(dest)) return true;
			return false;
		}

		// numerical support -- these have coordinate-wise definitions available
		// we do not propagate NaN so no test here for it
		virtual bool is_inf() const {
			if (0 == domain()->allow_infinity()) return false;
			if (const auto test = _is_finite()) return !*test;
			return false;
		}
		virtual bool is_finite() const {
			if (0 == domain()->allow_infinity()) return true;
			if (const auto test = _is_finite()) return *test;
			return false;
		}
		virtual std::optional<bool> is_finite_kripke() const {
			if (0 == domain()->allow_infinity()) return true;
			return _is_finite();
		}
		virtual bool is_zero() const = 0;
		virtual bool is_one() const = 0;
		virtual int sgn() const = 0;
		// scalbn: scale by power of 2.  Important operation as it's infinite-precision (when it works)
		virtual bool is_scal_bn_identity() const = 0;
		virtual intmax_t scal_bn_is_safe(intmax_t scale) const = 0; // return value might be less extreme than requested
		void scal_bn(intmax_t scale) {
			if (0 == scale || is_scal_bn_identity()) return;	// no-op
			if (const auto try_this = scal_bn_is_safe(scale); try_this != scale) throw std::logic_error("attempted unsafe scal_bn");
			_scal_bn(scale);
		};	// power-of-two
		virtual intmax_t ideal_scal_bn() const = 0; // what would set our fp exponent to 1
		// technical infrastructure
		virtual fp_API* clone() const = 0;	// result is a value-clone; internal representation may be more efficient than the source
		virtual std::string to_s() const = 0;
		virtual int precedence() const = 0;
		virtual int precedence_to_s() const { return precedence(); }

		std::partial_ordering value_compare(const fp_API* rhs) const {
			if (!rhs) return std::partial_ordering::unordered; // might need this to "work"
			if (this == rhs) return std::partial_ordering::equivalent; // self-compare
			return _value_compare(rhs); // delegate
		}
		friend std::partial_ordering operator<=>(const fp_API& lhs, const fp_API& rhs) { return lhs.value_compare(&rhs); }

	protected:
		bool is_scal_bn_identity_default() const { return is_zero() || is_inf(); }

	private:
		virtual void _scal_bn(intmax_t scale) = 0;	// power-of-two
		virtual fp_API* _eval() const = 0;	// memory-allocating evaluation
		virtual std::optional<bool> _is_finite() const { throw std::logic_error("must define _is_finite"); }
		virtual std::partial_ordering _value_compare(const fp_API* rhs) const { return std::partial_ordering::unordered; } // stub \todo make abstract
	};

	// static converter
	namespace ptr
	{
		template<class T> T* writeable(eval_to_ptr<fp_API>::eval_type& src) requires requires(const T* x) { x->typed_clone(); } {
			if (auto r = src.get_rw<T>()) {
				if (!r->first) src = std::unique_ptr<fp_API>(r->first = r->second->typed_clone());
				return r->first;
			}
			return nullptr;
		}

		template<class T> T* writeable(eval_to_ptr<fp_API>::eval_type& src) {
			if (auto r = src.get_rw<T>()) {
				if (!r->first) {
					src = std::unique_ptr<fp_API>(src.get_c()->clone());
					if (!(r = src.get_rw<T>())) return nullptr;
				}
				return r->first;
			}
			return nullptr;
		}
	}	// namespace ptr

	// top-levels: _C_SHARP_, _R_SHARP_, _S1_, _H_SHARP_, _O_SHARP_
	template<>
	struct _type<_type_spec::_O_SHARP_> : public virtual math::type {
		enum { _allow_infinity = 1 };

		int allow_infinity() const override { return _allow_infinity; }
		bool is_totally_ordered() const override { return false; }
		const type* self(_type_spec::canonical_functions op) const override {
			switch (op) {
			case _type_spec::Addition:
			case _type_spec::Multiplication: return this;
			default: throw std::logic_error("unhandled operation");
			}
		}
	private:
		bool _nonstrictSuperclass(const type* rhs) const override { return nullptr != dynamic_cast<decltype(this)>(rhs); }
	};
	static_assert(1 == _type<_type_spec::_O_SHARP_>::_allow_infinity);

	template<>
	struct _type<_type_spec::_H_SHARP_> : public virtual math::type {
		enum { _allow_infinity = 1 };

		int allow_infinity() const override { return _allow_infinity; }
		bool is_totally_ordered() const override { return false; }
		const type* self(_type_spec::canonical_functions op) const override {
			switch (op) {
			case _type_spec::Addition:
			case _type_spec::Multiplication: return this;
			default: throw std::logic_error("unhandled operation");
			}
		}
	private:
		bool _nonstrictSuperclass(const type* rhs) const override { return nullptr != dynamic_cast<decltype(this)>(rhs); }
	};
	static_assert(1 == _type<_type_spec::_H_SHARP_>::_allow_infinity);

	template<>
	struct _type<_type_spec::_C_SHARP_> : public virtual math::type {
		enum { _allow_infinity = 1 };

		int allow_infinity() const override { return _allow_infinity; }
		bool is_totally_ordered() const override { return false; }
		const type* self(_type_spec::canonical_functions op) const override {
			switch (op) {
				case _type_spec::Addition :
				case _type_spec::Multiplication : return this;
				default: throw std::logic_error("unhandled operation");
			}
		}
	private:
		bool _nonstrictSuperclass(const type* rhs) const override { return nullptr != dynamic_cast<decltype(this)>(rhs); }
	};
	static_assert(1 == _type<_type_spec::_C_SHARP_>::_allow_infinity);

	template<>
	struct _type<_type_spec::_R_SHARP_> : public virtual math::type {
		enum { _allow_infinity = -1 };

		int allow_infinity() const override { return _allow_infinity; }
		bool is_totally_ordered() const override { return true; }
		const type* self(_type_spec::canonical_functions op) const override {
			switch (op) {
			case _type_spec::Addition:
			case _type_spec::Multiplication: return this;
			default: throw std::logic_error("unhandled operation");
			}
		}
	private:
		bool _nonstrictSuperclass(const type* rhs) const override { return nullptr != dynamic_cast<decltype(this)>(rhs); }
	};
	static_assert(-1 == _type<_type_spec::_R_SHARP_>::_allow_infinity);

	// unit circle.  Permutation groups and n-dimensional surfaces of n+1-dimensional spheres would be different type hierarchies
	template<>
	struct _type<_type_spec::_S1_> final : public virtual math::type {
		enum { _allow_infinity = 0 };

		int allow_infinity() const override { return _allow_infinity; }
		bool is_totally_ordered() const override { return false; }
		const type* self(_type_spec::canonical_functions op) const override {
			switch (op) {
			case _type_spec::Addition: return this;
			case _type_spec::Multiplication: return nullptr;
			default: throw std::logic_error("unhandled operation");
			}
		}
		const type* left(_type_spec::canonical_functions op, const type& rhs) const override {
			switch (op) {
			case _type_spec::Addition: return 0 >= rhs.subclass(math::get<_type<_type_spec::_S1_>>()) ? this : nullptr;
			case _type_spec::Multiplication: return 0 >= rhs.subclass(math::get<_type<_type_spec::_R_SHARP_>>()) ? this : nullptr;
			default: throw std::logic_error("unhandled operation");
			}
		}
		const type* right(_type_spec::canonical_functions op, const type& lhs) const override {
			switch (op) {
			case _type_spec::Addition: return 0 >= lhs.subclass(math::get<_type<_type_spec::_S1_>>()) ? this : nullptr;
			case _type_spec::Multiplication: return 0 >= lhs.subclass(math::get<_type<_type_spec::_R_SHARP_>>()) ? this : nullptr;
			default: throw std::logic_error("unhandled operation");
			}
		}

	private:
		std::partial_ordering _superclass(const type* rhs) const override { return _nonstrictSuperclass(this) ? std::partial_ordering::equivalent : std::partial_ordering::unordered; }
		bool _nonstrictSuperclass(const type* rhs) const override { return nullptr != dynamic_cast<decltype(this)>(rhs); }
	};
	static_assert(0 == _type<_type_spec::_S1_>::_allow_infinity);

	// subspace relations
	template<>
	struct _type<_type_spec::_O_> : public _type<_type_spec::_O_SHARP_> {
		enum { _allow_infinity = 0 };

		virtual ~_type() = default;

		// numerical support -- these have coordinate-wise definitions available
		int allow_infinity() const override { return _allow_infinity; }
		const type* self(_type_spec::canonical_functions op) const override {
			switch (op) {
			case _type_spec::Addition:
			case _type_spec::Multiplication: return this;
			default: throw std::logic_error("unhandled operation");
			}
		}
	private:
		bool _nonstrictSuperclass(const type* rhs) const override { return nullptr != dynamic_cast<decltype(this)>(rhs); }
	};
	static_assert(0 == _type<_type_spec::_O_>::_allow_infinity);

	template<>
	struct _type<_type_spec::_H_> : public _type<_type_spec::_O_>, public _type<_type_spec::_H_SHARP_> {
		enum { _allow_infinity = 0 };

		virtual ~_type() = default;

		// numerical support -- these have coordinate-wise definitions available
		int allow_infinity() const override { return _allow_infinity; }
		bool is_totally_ordered() const override { return false; }
		const type* self(_type_spec::canonical_functions op) const override {
			switch (op) {
			case _type_spec::Addition:
			case _type_spec::Multiplication: return this;
			default: throw std::logic_error("unhandled operation");
			}
		}
	private:
		bool _nonstrictSuperclass(const type* rhs) const override { return nullptr != dynamic_cast<decltype(this)>(rhs); }
	};
	static_assert(0 == _type<_type_spec::_H_>::_allow_infinity);

	template<>
	struct _type<_type_spec::_C_> : public _type<_type_spec::_H_>, public _type<_type_spec::_C_SHARP_> {
		enum { _allow_infinity = 0 };

		virtual ~_type() = default;

		// numerical support -- these have coordinate-wise definitions available
		int allow_infinity() const override { return _allow_infinity; }
		bool is_totally_ordered() const override { return false; }
		const type* self(_type_spec::canonical_functions op) const override {
			switch (op) {
			case _type_spec::Addition:
			case _type_spec::Multiplication: return this;
			default: throw std::logic_error("unhandled operation");
			}
		}
	private:
		bool _nonstrictSuperclass(const type* rhs) const override { return nullptr != dynamic_cast<decltype(this)>(rhs); }
	};
	static_assert(0 == _type<_type_spec::_C_>::_allow_infinity);

	template<>
	struct _type<_type_spec::_R_> : public _type<_type_spec::_C_>, public _type<_type_spec::_R_SHARP_> {
		enum { _allow_infinity = 0 };

		int allow_infinity() const override { return _allow_infinity; }
		bool is_totally_ordered() const override { return true; }
		const type* self(_type_spec::canonical_functions op) const override {
			switch (op) {
			case _type_spec::Addition:
			case _type_spec::Multiplication: return this;
			default: throw std::logic_error("unhandled operation");
			}
		}
	private:
		bool _nonstrictSuperclass(const type* rhs) const override { return nullptr != dynamic_cast<decltype(this)>(rhs); }
	};
	static_assert(0 == _type<_type_spec::_R_>::_allow_infinity);

	template<>
	struct _type<_type_spec::_Q_> : public _type<_type_spec::_R_> {
		const type* self(_type_spec::canonical_functions op) const override {
			switch (op) {
			case _type_spec::Addition:
			case _type_spec::Multiplication: return this;
			default: throw std::logic_error("unhandled operation");
			}
		}
	private:
		bool _nonstrictSuperclass(const type* rhs) const override { return nullptr != dynamic_cast<decltype(this)>(rhs); }
	};
	static_assert(0 == _type<_type_spec::_Q_>::_allow_infinity);

	template<>
	struct _type<_type_spec::_Z_> final : public _type<_type_spec::_Q_> {
		const type* self(_type_spec::canonical_functions op) const override {
			switch (op) {
			case _type_spec::Addition:
			case _type_spec::Multiplication: return this;
			default: throw std::logic_error("unhandled operation");
			}
		}
		const type* inverse(_type_spec::canonical_functions op) const override {
			switch (op) {
			case _type_spec::Addition: return this;
			case _type_spec::Multiplication: return &math::get<_type<_type_spec::_Q_>>(); // closure of _Z_ under * is _Q_
			default: throw std::logic_error("unhandled operation");
			}
		}
	private:
		bool _nonstrictSuperclass(const type* rhs) const override { return nullptr != dynamic_cast<decltype(this)>(rhs); }
	};
	static_assert(0 == _type<_type_spec::_Z_>::_allow_infinity);

}	// namespace zaimoni

#endif
