#ifndef EVAL_HPP
#define EVAL_HPP 1

#include <limits.h>
#include <limits>
#include <utility>
#include <memory>
#include <string>
#include "augment.STL/type_traits"

namespace zaimoni {

	using std::to_string;
	using std::swap;

	// Yet another take on higher-mathematics typing.
	struct _type_spec {
		enum arch_domain {
			_Z_ = 1,	// integers
			_Q_,		// rational numbers
			_R_,		// real numbers
			_C_,		// complex numbers
			_R_SHARP_,	// extended real numbers
			_C_SHARP_	// extended complex numbers
		};
		enum operation {
			none = 0	// 
			// we do want to support vector spaces, matrices, etc.
		};
	};

	template<_type_spec::arch_domain DOM, _type_spec::operation OP = _type_spec::none> struct _type {
		virtual ~_type() = default;
	};

	template<class T>
	struct eval_shared_ptr
	{
		virtual std::shared_ptr<T> destructive_eval() = 0;
	};

	struct fp_API {	// virtual base
		static constexpr std::pair<intmax_t, intmax_t> max_scal_bn_safe_range() { return std::pair<intmax_t, intmax_t>(std::numeric_limits<intmax_t>::min(), std::numeric_limits<intmax_t>::max()); }	// simple static member variable crashes at link-time even if initialized here

		virtual ~fp_API() = default;

		virtual bool self_eval() = 0;
		template<class T>
		typename std::enable_if<std::is_base_of<fp_API, T>::value, bool >::type
		static eval(std::shared_ptr<T>& dest) {
			if (auto efficient = dynamic_cast<eval_shared_ptr<T>*>(dest.get())) {	// should be NULL rather than throwing bad_cast
				if (auto result = efficient->destructive_eval()) {
					dest = result;
					return true;
				}
			}
			if (auto result = dest->_eval()) {
				dest = std::shared_ptr<T>(dynamic_cast<T*>(result));
				return true;
			}
			return false;
		}

		// numerical support -- these have coordinate-wise definitions available
		// we do not propagate NaN so no test here for it
		virtual int allow_infinity() const = 0;	// 0: no; -1: signed; 1 unsigned
		virtual bool is_inf() const = 0;
		virtual bool is_finite() const = 0;
		virtual bool is_zero() const = 0;
		virtual bool is_one() const = 0;
		virtual int sgn() const = 0;
		// scalbn: scale by power of 2.  Important operation as it's infinite-precision (when it works)
		virtual bool is_scal_bn_identity() const = 0;
		virtual std::pair<intmax_t,intmax_t> scal_bn_safe_range() const = 0;	// return value is (lower bound, upper bound); 0 >= lower bound, 0 <= upper bound; bounds are non-strict
		void scal_bn(intmax_t scale) {
			if (0 == scale || is_scal_bn_identity()) return;	// no-op
			const auto legal = scal_bn_safe_range();
			if (0 < scale) {
				if (scale > legal.second) throw std::runtime_error("attempted overflow scal_bn");
			} else {	// if (0 > scale)
				if (scale < legal.first) throw std::runtime_error("attempted underflow scal_bn");
			}

			_scal_bn(scale);
		};	// power-of-two
		virtual intmax_t ideal_scal_bn() const = 0; // what would set our fp exponent to 1
		// technical infrastructure
		virtual fp_API* clone() const = 0;	// result is a value-clone; internal representation may be more efficient than the source
		virtual std::string to_s() const = 0;
		virtual int precedence() const = 0;
	protected:
		template<class T>
		typename std::enable_if<std::is_base_of<fp_API, T>::value, void >::type
		static __scal_bn(std::shared_ptr<T>& dest,intmax_t scale) {
			std::shared_ptr<T> working = dest.unique() ? dest : std::shared_ptr<T>(dynamic_cast<T*>(dest->clone()));
			working->scal_bn(scale);
			dest = working;
		}
	private:
		virtual void _scal_bn(intmax_t scale) = 0;	// power-of-two
		virtual fp_API* _eval() const = 0;	// memory-allocating evaluation
	};

	template<class T>
	T* clone(const T& src) { return src.clone(); }

	template<class T>
	typename std::enable_if<std::is_base_of<fp_API,T>::value, std::shared_ptr<T> >::type
	scalBn(const std::shared_ptr<T>& dest, intmax_t scale) {
		auto working = dest.unique() ? dest : std::shared_ptr<T>(dynamic_cast<T*>(dest->clone()));
		working->scal_bn(scale);
		return working;
	}

	template<class T>
	struct _access : public virtual fp_API {
		virtual T& value() = 0;
		virtual const T& value() const = 0;

		virtual bool self_eval() { return false; };

		virtual bool is_zero() const { return zaimoni::is_zero(value()); };
		virtual bool is_one() const { return zaimoni::is_one(value()); };
		virtual int sgn() const { return zaimoni::sgn(value()); };

		virtual std::string to_s() const { return to_string(value()); }
		virtual int precedence() const { return std::numeric_limits<int>::max(); }	// things like numerals generally outrank all operators
	};

	template<class Derived>
	struct _infinite : public virtual fp_API {
		virtual bool is_inf() const { return static_cast<const Derived*>(this)->_is_inf(); }
		virtual bool is_finite() const { return static_cast<const Derived*>(this)->_is_finite(); };
	};

	// top-levels: _C_SHARP_, _R_SHARP_
	template<_type_spec::operation OP>
	struct _type<_type_spec::_C_SHARP_, OP> : public virtual fp_API {
		enum { API_code = 1 };
		virtual int allow_infinity() const { return 1; }
	};

	template<_type_spec::operation OP>
	struct _type<_type_spec::_R_SHARP_, OP> : public virtual fp_API {
		enum { API_code = 1 };
		virtual int allow_infinity() const { return -1; }
	};

	// subspace relations
	template<_type_spec::operation OP>
	struct _type<_type_spec::_C_, OP> : public _type<_type_spec::_C_SHARP_, OP> {
		enum {API_code = 0};

		virtual ~_type() = default;

		// numerical support -- these have coordinate-wise definitions available
		virtual int allow_infinity() const override { return 0; }
		virtual bool is_inf() const { return false; };
		virtual bool is_finite() const { return true; };
	};

	template<_type_spec::operation OP>
	struct _type<_type_spec::_R_, OP> : public _type<_type_spec::_C_, OP>, public _type<_type_spec::_R_SHARP_, OP> {
		virtual int allow_infinity() const override { return 0; }
	};

	template<_type_spec::operation OP>
	struct _type<_type_spec::_Q_, OP> : public _type<_type_spec::_R_, OP> {};

	template<_type_spec::operation OP>
	struct _type<_type_spec::_Z_, OP> : public _type<_type_spec::_Q_, OP> {};

	template<class T>
	struct _type_of {};	// must override to do anything useful

	template<class Derived, class T, int API_CODE>
	struct _interface_of {};	// must override to do anything useful

	template<class Derived, class T>
	struct _interface_of<Derived, std::shared_ptr<T>, 0>
	{
	};

	template<class Derived, class T>
	struct _interface_of<Derived, std::shared_ptr<T>, 1> : public _infinite<Derived>
	{
	};

	template<>
	struct _type_of<float>
	{
		typedef _type<_type_spec::_R_SHARP_> type;
	};

	template<>
	struct _type_of<double>
	{
		typedef _type<_type_spec::_R_SHARP_> type;
	};

	template<>
	struct _type_of<long double>
	{
		typedef _type<_type_spec::_R_SHARP_> type;
	};


	template<class T>
	struct eval {
		enum {
			has_self = 0,
			has_destructive = 0
		};
		// required API of specializations
#if 0
		typedef ... in_type;
		typedef _type_of<in_type>::type out_type;

		static bool self(in_type& x);
		static out_type* destructive(in_type*& x);
#endif
	};	// override to do anything useful

	template<class T>
	typename std::enable_if<eval<T>::has_self, bool>::type self_eval(T& x) { return eval<T>::self(x); }

	template<class T>
	typename std::enable_if<eval<T>::has_destructive, typename eval<T>::out_type>::type destructive_eval(T*& x) { return eval<T>::destructive(x); }

}	// namespace zaimoni

#endif
