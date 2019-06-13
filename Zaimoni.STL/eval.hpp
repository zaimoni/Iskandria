#ifndef EVAL_HPP
#define EVAL_HPP 1

#include <limits.h>
#include <type_traits>

namespace zaimoni {

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

	template<_type_spec::arch_domain DOMAIN, _type_spec::operation OP = _type_spec::none> struct _type {
		virtual ~_type() = default;
	};

	struct fp_API {	// virtual base
		virtual ~fp_API() = default;
		// numerical support -- these have coordinate-wise definitions available
		// we do not propagate NaN so no test here for it
		virtual bool is_inf() const = 0;
		virtual bool is_finite() const = 0;
		// scalbn: scale by power of 2.  Important operation as it's infinite-precision (when it works)
		virtual bool is_scal_bn_identity() const = 0;
		virtual void scal_bn_safe_range(intmax_t& lb, intmax_t& ub) const = 0;
		virtual bool scal_bn(intmax_t scale) = 0;	// power-of-two
		// technical infrastructure
		virtual fp_API* clone() const = 0;	// result is a value-clone; internal representation may be more efficient than the source
	};

	template<class T>
	T* clone(const T& src) { return dynamic_cast<T*>(src.clone()); }

	// top-levels: _C_SHARP_, _R_SHARP_
	template<>
	template<_type_spec::operation OP>
	struct _type<_type_spec::_C_SHARP_, OP> : public virtual fp_API {
		enum { API_code = 1 };
	};

	template<>
	template<_type_spec::operation OP>
	struct _type<_type_spec::_R_SHARP_, OP> : public virtual fp_API {
		enum { API_code = 1 };
	};

	// subspace relations
	template<>
	template<_type_spec::operation OP>
	struct _type<_type_spec::_C_, OP> : public _type<_type_spec::_C_SHARP_, OP> {
		enum {API_code = 0};

		virtual ~_type() = default;

		// numerical support -- these have coordinate-wise definitions available
		virtual bool is_inf() const { return false; };
		virtual bool is_finite() const { return true; };
	};

	template<>
	template<_type_spec::operation OP>
	struct _type<_type_spec::_R_, OP> : public _type<_type_spec::_C_, OP>, public _type<_type_spec::_R_SHARP_, OP> {};

	template<>
	template<_type_spec::operation OP>
	struct _type<_type_spec::_Q_, OP> : public _type<_type_spec::_R_, OP> {};

	template<>
	template<_type_spec::operation OP>
	struct _type<_type_spec::_Z_, OP> : public _type<_type_spec::_Q_, OP> {};

	template<class T>
	struct _type_of {};	// must override to do anything useful

	template<class Derived, class T, int API_CODE>
	struct _interface_of {};	// must override to do anything useful

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
