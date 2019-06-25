// order matters: interval must be before var.hpp and eval.hpp to prevent compile errors
#include "interval_shim.hpp"
#include "arithmetic.hpp"
#include "Zaimoni.STL/var.hpp"

namespace zaimoni {
namespace math {

	template<class T>
	typename std::enable_if<std::is_base_of<fp_API, T>::value, int>::type rearrange_sum(std::shared_ptr<T>& lhs, std::shared_ptr<T>& rhs) { return 0; }

	template<class T>
	typename std::enable_if<std::is_base_of<fp_API, T>::value, int>::type rearrange_product(std::shared_ptr<T>& lhs, std::shared_ptr<T>& rhs) { return 0; }

	template<class T>
	typename std::enable_if<std::is_base_of<fp_API, T>::value, T*>::type eval_quotient(const std::shared_ptr<T>& n, const std::shared_ptr<T>& d) { return 0; }

#if 0
	template int rearrange_sum< _type<_type_spec::_R_SHARP_, _type_spec::none> >(std::shared_ptr<_type<_type_spec::_R_SHARP_, _type_spec::none> >& lhs, std::shared_ptr<_type<_type_spec::_R_SHARP_, _type_spec::none> >& rhs);
#else
	// rearrange_sum support
	template<class T>
	typename std::enable_if<std::is_floating_point<T>::value, std::pair<int, uintmax_t>>::type mantissa_bits(T mantissa)	// cmath target
	{
		std::pair<int, uintmax_t> ret(0,1);

		while (0.0 < mantissa) {
			ret.first++;
			ret.second <<= 1;
			if (0.5 <= mantissa) ret.second += 1;
			mantissa = scalbn(mantissa, 1);
			mantissa -= 1.0;
		}
		return ret;
	}

	template<class T, class F>
	typename std::enable_if<std::is_base_of<fp_API, T>::value&& std::is_floating_point<F>::value, int>::type rearrange_sum(F& lhs, F& rhs)
	{
		int ret = 0;

		int _exponents[2];	// 0: lhs; 1:rhs
		F l_mantissa = frexp(lhs, &_exponents[0]);
		F r_mantissa = frexp(rhs, &_exponents[1]);
		if (std::numeric_limits<F>::max_exponent < _exponents[0]) return 0;	// we don't handle infinity or NaN here
		if (std::numeric_limits<F>::max_exponent < _exponents[1]) return 0;

		if (_exponents[0] > _exponents[1]) {	// doesn't work for different types
			swap(_exponents[0], _exponents[1]);
			swap(l_mantissa, r_mantissa);
			swap(lhs, rhs);
		}

		bool l_negative = std::signbit(lhs);
		bool r_negative = std::signbit(rhs);
		bool same_sign = (l_negative == r_negative);
		if (_exponents[0] == _exponents[1]) {
			if (!same_sign) {	// proceed (subtractive cancellation ok at this point)
				F tmp = lhs + rhs;
				if (0 == tmp) {
					// mutual annihilation
					lhs = 0;
					rhs = 0;
					return -2;
				}
				if (std::signbit(tmp)) {
					if (l_negative) {
						lhs = tmp;
						rhs = 0;
						return 1;
					} else {
						lhs = 0;
						rhs = tmp;
						return -1;
					}
				} else {
					if (!l_negative) {
						lhs = tmp;
						rhs = 0;
						return 1;
					} else {
						lhs = 0;
						rhs = tmp;
						return -1;
					}
				}
			} else if (std::numeric_limits<F>::min_exponent > _exponents[0]) {
				// denormal, same sign:
				F anchor = std::scalbn(l_negative ? F(-1) : F(1), std::numeric_limits<F>::min_exponent);
				F reference = anchor-lhs;
				if (l_negative) {
					if (reference <= rhs) {	// still denormal, proceed
						rhs += lhs;
						lhs = 0;
						return -1;
					} else {
						// negate the anchor, then use it.
						reference = std::scalbn(F(1), std::numeric_limits<F>::min_exponent);
						reference += rhs;
						lhs += reference;
						rhs = anchor;
						return 2;
					}
				} else {
					if (reference >= rhs) {	// still denormal, proceed
						rhs += lhs;
						lhs = 0;
						return -1;
					} else {
						// negate the anchor, then use it.
						reference = std::scalbn(F(-1), std::numeric_limits<F>::min_exponent);
						reference += rhs;
						lhs += reference;
						rhs = anchor;
						return 2;
					}
				}
			} else if (std::numeric_limits<F>::max_exponent == _exponents[0]) return 0; // overflow imminent
			else {	// sum may be overprecise
				F bias = l_negative ? F(-0.5) : F(0.5);
				F anchor = (l_mantissa - bias) + (r_mantissa - bias);
				rhs = std::scalbn(bias, _exponents[0] + 1);
				lhs = std::scalbn(anchor, _exponents[0]);
				ret = 2;
				l_mantissa = frexp(lhs, &_exponents[0]);
				r_mantissa = frexp(rhs, &_exponents[1]);
			}
		}

restart:
		bool l_to_r = std::numeric_limits<F>::min_exponent <= _exponents[0] && _exponents[0] <= _exponents[1] && _exponents[1] - std::numeric_limits<F>::digits <= _exponents[0];
		if (!l_to_r) return ret;	// done
		F delta = std::scalbn(l_negative ? F(-0.5) : F(0.5), _exponents[0]);

		if (same_sign) {
			F ceiling = std::scalbn(l_negative ? F(-1) : F(1), _exponents[1]);
			ceiling -= delta;
			if (ceiling < rhs) {
				auto test = mantissa_bits(r_mantissa);	// \todo micro-optimize, we don't need the uintmax_t return half here
				if (std::numeric_limits<F>::digits < test.second) return ret;	// would lose the lowest bit
			}
		}

		// controlled subtractive cancellation.
		if (delta_cancel(rhs,lhs,delta)) return -1;
		l_mantissa = frexp(lhs, &_exponents[0]);
		r_mantissa = frexp(rhs, &_exponents[1]);
		ret = 2;
		goto restart;
	}

	template<class T, class F, class F2>
	typename std::enable_if<std::is_base_of<fp_API, T>::value&& std::is_floating_point<F>::value&& std::is_floating_point<F2>::value && (std::numeric_limits<F>::digits<std::numeric_limits<F2>::digits), int>::type rearrange_sum(F& lhs, F2& rhs)
	{
		int ret = 0;

		int _exponents[2];	// 0: lhs; 1:rhs
		F l_mantissa = frexp(lhs, &_exponents[0]);
		F2 r_mantissa = frexp(rhs, &_exponents[1]);
		if (std::numeric_limits<F>::max_exponent < _exponents[0]) return 0;	// we don't handle infinity or NaN here
		if (std::numeric_limits<F2>::max_exponent < _exponents[1]) return 0;

		bool l_to_r = std::numeric_limits<F>::min_exponent <= _exponents[0] && _exponents[0] <= _exponents[1] && _exponents[1] - std::numeric_limits<F2>::digits <= _exponents[0];
		bool r_to_l = std::numeric_limits<F2>::min_exponent <= _exponents[1] && _exponents[1] <= _exponents[0] && _exponents[0] - std::numeric_limits<F>::digits <= _exponents[1];
		if (!l_to_r && !r_to_l) return ret;	// done; denormal heuristics are same-type only

		return 0;
	}

	template<class T, class F, class F2>
	typename std::enable_if<std::is_base_of<fp_API, T>::value&& std::is_floating_point<F>::value&& std::is_floating_point<F2>::value && (std::numeric_limits<F>::digits > std::numeric_limits<F2>::digits), int>::type rearrange_sum(F& lhs, F2& rhs)
	{
		int ret = rearrange_sum<T>(rhs,lhs);
		switch (ret)
		{
		case 1: return -1;
		case -1: return 1;
		default: return ret;
		}
	}

	template<class T, class F, class F2>
	typename std::enable_if<std::is_base_of<fp_API, T>::value&& std::is_floating_point<F>::value&& std::is_floating_point<F2>::value, int>::type rearrange_sum(F& lhs, ISK_INTERVAL<F2>& rhs)
	{
		return 0;
	}

	template<class T, class F, class F2>
	typename std::enable_if<std::is_base_of<fp_API, T>::value&& std::is_floating_point<F>::value&& std::is_floating_point<F2>::value, int>::type rearrange_sum(ISK_INTERVAL<F>& lhs, F2& rhs)
	{
		return 0;
	}

	template<class T, class F, class F2>
	typename std::enable_if<std::is_base_of<fp_API, T>::value&& std::is_floating_point<F>::value&& std::is_floating_point<F2>::value, int>::type rearrange_sum(ISK_INTERVAL<F>& lhs, ISK_INTERVAL<F2>& rhs)
	{
		return 0;
	}

	template<class T, class F>
	typename std::enable_if<std::is_base_of<fp_API, T>::value && std::is_floating_point<F>::value, int>::type rearrange_sum(std::shared_ptr<T>& lhs, F& rhs)
	{
		// setup of working will fail badly in a multi-threaded situation
		typename std::remove_reference<decltype(lhs)>::type working;
		if (lhs.unique()) working = lhs;
		else working = std::shared_ptr<_type<_type_spec::_R_SHARP_, _type_spec::none> >(dynamic_cast<_type<_type_spec::_R_SHARP_, _type_spec::none>*>(lhs->clone()));

		int ret = 0;

		auto src = working.get();
		if (auto l = dynamic_cast<_access<float>*>(src)) ret = rearrange_sum<T>(l->value(), rhs);
		else if (auto l = dynamic_cast<_access<ISK_INTERVAL<float> >*>(src)) ret = rearrange_sum<T>(l->value(), rhs);
		else if (auto l = dynamic_cast<_access<double>*>(src)) ret = rearrange_sum<T>(l->value(), rhs);
		else if (auto l = dynamic_cast<_access<ISK_INTERVAL<double> >*>(src)) ret = rearrange_sum<T>(l->value(), rhs);
		else if (auto l = dynamic_cast<_access<long double>*>(src)) ret = rearrange_sum<T>(l->value(), rhs);
		else if (auto l = dynamic_cast<_access<ISK_INTERVAL<long double> >*>(src)) ret = rearrange_sum<T>(l->value(), rhs);

		if (ret) lhs = working;
		return ret;
	}

	template<class T, class F>
	typename std::enable_if<std::is_base_of<fp_API, T>::value && std::is_floating_point<F>::value, int>::type rearrange_sum(std::shared_ptr<T>& lhs, ISK_INTERVAL<F>& rhs)
	{
		// setup of working will fail badly in a multi-threaded situation
		typename std::remove_reference<decltype(lhs)>::type working;
		if (lhs.unique()) working = lhs;
		else working = std::shared_ptr<_type<_type_spec::_R_SHARP_, _type_spec::none> >(dynamic_cast<_type<_type_spec::_R_SHARP_, _type_spec::none>*>(lhs->clone()));

		int ret = 0;

		auto src = working.get();
		if (auto l = dynamic_cast<_access<float>*>(src)) ret = rearrange_sum<T>(l->value(), rhs);
		else if (auto l = dynamic_cast<_access<ISK_INTERVAL<float> >*>(src)) ret = rearrange_sum<T>(l->value(), rhs);
		else if (auto l = dynamic_cast<_access<double>*>(src)) ret = rearrange_sum<T>(l->value(), rhs);
		else if (auto l = dynamic_cast<_access<ISK_INTERVAL<double> >*>(src)) ret = rearrange_sum<T>(l->value(), rhs);
		else if (auto l = dynamic_cast<_access<long double>*>(src)) ret = rearrange_sum<T>(l->value(), rhs);
		else if (auto l = dynamic_cast<_access<ISK_INTERVAL<long double> >*>(src)) ret = rearrange_sum<T>(l->value(), rhs);

		if (ret) lhs = working;
		return ret;
	}

	template<> int rearrange_sum< _type<_type_spec::_R_SHARP_, _type_spec::none> >(std::shared_ptr<_type<_type_spec::_R_SHARP_, _type_spec::none> >& lhs, std::shared_ptr<_type<_type_spec::_R_SHARP_, _type_spec::none> >& rhs)
	{	// we assume we are being called from the zaimoni::sum object.
		// that is, all of the zero and infinity symbolic processing has already happened.

		// setup of working will fail badly in a multi-threaded situation
		typename std::remove_reference<decltype(rhs)>::type working;
		if (rhs.unique()) working = rhs;
		else working = std::shared_ptr<_type<_type_spec::_R_SHARP_, _type_spec::none> >(dynamic_cast<_type<_type_spec::_R_SHARP_, _type_spec::none>*>(rhs->clone()));

		int ret = 0;

		auto src = working.get();
		if (auto r = dynamic_cast<_access<float>*>(src)) ret = rearrange_sum(lhs, r->value());
		else if (auto r = dynamic_cast<_access<ISK_INTERVAL<float> >*>(src)) ret = rearrange_sum(lhs, r->value());
		else if (auto r = dynamic_cast<_access<double>*>(src)) ret = rearrange_sum(lhs, r->value());
		else if (auto r = dynamic_cast<_access<ISK_INTERVAL<double> >*>(src)) ret = rearrange_sum(lhs, r->value());
		else if (auto r = dynamic_cast<_access<long double>*>(src)) ret = rearrange_sum(lhs, r->value());
		else if (auto r = dynamic_cast<_access<ISK_INTERVAL<long double> >*>(src)) ret = rearrange_sum(lhs, r->value());

		if (ret) rhs = working;
		return ret;
	}
#endif

	template int rearrange_product< _type<_type_spec::_R_SHARP_, _type_spec::none> >(std::shared_ptr<_type<_type_spec::_R_SHARP_, _type_spec::none> >& lhs, std::shared_ptr<_type<_type_spec::_R_SHARP_, _type_spec::none> >& rhs);

#if 0
	template _type<_type_spec::_R_SHARP_, _type_spec::none>* eval_quotient< _type<_type_spec::_R_SHARP_, _type_spec::none> >(const std::shared_ptr<_type<_type_spec::_R_SHARP_, _type_spec::none> >& n, const std::shared_ptr<_type<_type_spec::_R_SHARP_, _type_spec::none> >& d);
#else
	// eval_quotient support
	template<class T,class F>
	typename std::enable_if<std::is_base_of<fp_API, T>::value&& std::is_floating_point<F>::value, T*>::type eval_quotient(const ISK_INTERVAL<F>& n, const ISK_INTERVAL<F>& d)
	{
		try {
			auto ret = n / d;
			if (ret.lower() == ret.upper()) return new var<typename ISK_INTERVAL<F>::base_type>(ret.upper());
			return new var<decltype(ret)>(ret);
		} catch (zaimoni::math::numeric_error& e) {
			// doesn't help w/Boost, but our internal interval type should like to throw on overflow, etc.
			return 0;
		}
		return 0;
	}

	template<class T,class F, class F2>
	typename std::enable_if<std::is_base_of<fp_API, T>::value&& std::is_floating_point<F>::value && std::is_floating_point<F2>::value && (std::numeric_limits<F>::max_exponent<std::numeric_limits<F2>::max_exponent), T*>::type eval_quotient(const ISK_INTERVAL<F>& n, const ISK_INTERVAL<F2>& d)
	{
		return eval_quotient<T>(ISK_INTERVAL<F2>(n), d);
	}

	template<class T, class F, class F2>
	typename std::enable_if<std::is_base_of<fp_API, T>::value&& std::is_floating_point<F>::value&& std::is_floating_point<F2>::value && (std::numeric_limits<F>::max_exponent > std::numeric_limits<F2>::max_exponent), T*>::type eval_quotient(const ISK_INTERVAL<F>& n, const ISK_INTERVAL<F2>& d)
	{
		return eval_quotient<T>(n, ISK_INTERVAL<F>(d));
	}

	template<class T,class F>
	typename std::enable_if<std::is_base_of<fp_API, T>::value && std::is_floating_point<F>::value, T*>::type eval_quotient(const std::shared_ptr<T>& n, const ISK_INTERVAL<F>& d)
	{
		if (d == F(0)) throw zaimoni::math::numeric_error("division by zero");	// expected to be caught earlier when called from the quotient class
		if (F(0) > d.lower() && F(0) < d.upper()) throw zaimoni::math::numeric_error("division should result in two disjoint intervals");	// not always, but requires exact zero numerator which should be caught by the quotient class

		auto n_src = n.get();
		if (auto l = dynamic_cast<_access<float>*>(n_src)) return eval_quotient<T>(ISK_INTERVAL<float>(l->value()), d);
		else if (auto l = dynamic_cast<_access<ISK_INTERVAL<float> >*>(n_src)) return eval_quotient<T>(l->value(), d);
		else if (auto l = dynamic_cast<_access<double>*>(n_src)) return eval_quotient<T>(ISK_INTERVAL<double>(l->value()), d);
		else if (auto l = dynamic_cast<_access<ISK_INTERVAL<double> >*>(n_src)) return eval_quotient<T>(l->value(), d);
		else if (auto l = dynamic_cast<_access<long double>*>(n_src)) return eval_quotient<T>(ISK_INTERVAL<long double>(l->value()), d);
		else if (auto l = dynamic_cast<_access<ISK_INTERVAL<long double> >*>(n_src)) return eval_quotient<T>(l->value(), d);

		return 0;
	}

	template<> _type<_type_spec::_R_SHARP_, _type_spec::none>* eval_quotient< _type<_type_spec::_R_SHARP_, _type_spec::none> >(const std::shared_ptr<_type<_type_spec::_R_SHARP_, _type_spec::none> >& n, const std::shared_ptr<_type<_type_spec::_R_SHARP_, _type_spec::none> >& d)
	{	// we currently honor floating point types.  Integral types would also make sense here, mostly
		auto d_src = d.get();
		if (auto r = dynamic_cast<_access<float>*>(d_src)) return eval_quotient(n, ISK_INTERVAL<float>(r->value()));
		else if (auto r = dynamic_cast<_access<ISK_INTERVAL<float> >*>(d_src)) return eval_quotient(n, r->value());
		else if (auto r = dynamic_cast<_access<double>*>(d_src)) return eval_quotient(n, ISK_INTERVAL<double>(r->value()));
		else if (auto r = dynamic_cast<_access<ISK_INTERVAL<double> >*>(d_src)) return eval_quotient(n, r->value());
		else if (auto r = dynamic_cast<_access<long double>*>(d_src)) return eval_quotient(n, ISK_INTERVAL<long double>(r->value()));
		else if (auto r = dynamic_cast<_access<ISK_INTERVAL<long double> >*>(d_src)) return eval_quotient(n, r->value());
		return 0;
	}
#endif
}
}

#ifdef TEST_APP
// fast compile test
// g++ -std=c++14 -otest.exe -Os -D__STDC_LIMIT_MACROS -DTEST_APP arithmetic.cpp -Llib\host.isk -lz_stdio_c -lz_log_adapter -lz_stdio_log -lz_format_util
int main(int argc, char* argv[])
{
	// compile-time checks
	return 0;
}
#endif
