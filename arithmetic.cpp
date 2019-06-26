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
	template<class T, class F>
	typename std::enable_if<std::is_base_of<fp_API, T>::value&& std::is_floating_point<F>::value, int>::type rearrange_sum(F& lhs, F& rhs)
	{
		int ret = 0;

		int fp_type[2] = { fpclassify(lhs) , fpclassify(rhs) };
		assert(FP_NAN != fp_type[0]);
		assert(FP_NAN != fp_type[1]);
		assert(FP_INFINITE != fp_type[0]);
		assert(FP_INFINITE != fp_type[1]);
		if (FP_ZERO == fp_type[0]) return -1;	// should have intercepted earlier but we know what to do with these
		if (FP_ZERO == fp_type[1]) return 1;

		bool l_negative = std::signbit(lhs);
		bool same_sign = (std::signbit(rhs) == l_negative);

		if (FP_SUBNORMAL == fp_type[0] && FP_SUBNORMAL == fp_type[1]) {
			// double-denormal; requires that two types be the same
			if (!same_sign) {
resolve_exact_now:
				F tmp = lhs + rhs;
				if (0 == tmp) {
					// mutual annihilation
					lhs = 0;
					rhs = 0;
					return -2;
				}
				if (std::signbit(tmp) == l_negative) {
					lhs = tmp;
					rhs = 0;
					return 1;
				}
				lhs = 0;
				rhs = tmp;
				return -1;
			}
			F anchor = l_negative ? -std::numeric_limits<F>::min() : std::numeric_limits<F>::min();
			F reference = anchor - lhs;
			if (l_negative ? (reference <= rhs) : (reference >= rhs)) {
				// still denormal, proceed
				rhs += lhs;
				lhs = 0;
				return -1;
			}
			// negate the anchor, then use it.
			reference = -anchor;
			reference += rhs;
			lhs += reference;	// should not be zero as that would be "above"
			rhs = anchor;
			return 2;
		}

		// we don't handle infinity or NaN here
		fp_stats<F> l_stat(lhs);
		assert(std::numeric_limits<F>::max_exponent >= l_stat.exponent());
		assert((std::numeric_limits<F>::min_exponent > l_stat.exponent()) == (FP_SUBNORMAL == fp_type[0]));
		fp_stats<F> r_stat(rhs);
		assert(std::numeric_limits<F>::max_exponent >= r_stat.exponent());
		assert((std::numeric_limits<F>::min_exponent > r_stat.exponent()) == (FP_SUBNORMAL == fp_type[1]));

		if (   (std::numeric_limits<F>::min_exponent == l_stat.exponent() || FP_SUBNORMAL == fp_type[0])
			&& (std::numeric_limits<F>::min_exponent == r_stat.exponent() || FP_SUBNORMAL == fp_type[1])
			&& !same_sign)
			goto resolve_exact_now;

		if (r_stat.exponent() > l_stat.exponent()) {	// doesn't work for different types
			swap(fp_type[0], fp_type[1]);
			swap(l_stat,r_stat);
			swap(lhs, rhs);
			l_negative = std::signbit(lhs);
		}
		const int exponent_delta = l_stat.exponent() - (FP_SUBNORMAL == fp_type[1] ? std::numeric_limits<F>::min_exponent : r_stat.exponent());

		if (0 == exponent_delta) {	// depends on two types being same
			if (!same_sign) goto resolve_exact_now;		// proceed (subtractive cancellation ok at this point)
			else if (std::numeric_limits<F>::max_exponent == l_stat.exponent()) return 0; // overflow imminent
			else {	// sum may be overprecise
				auto l_test = mantissa_bitcount(l_stat.mantissa());
				auto r_test = mantissa_bitcount(r_stat.mantissa());
				if ((std::numeric_limits<F>::digits<l_test) == (std::numeric_limits<F>::digits < r_test)) {
					// direct addition ok
					lhs += rhs;
					rhs = 0;
					return 1;
				}

				F bias = l_stat.delta(0);
				F anchor = (l_stat.mantissa() - bias) + (r_stat.mantissa() - bias);

				rhs = std::scalbn(bias, l_stat.exponent() + 1);
				lhs = std::scalbn(anchor, l_stat.exponent());
				ret = 2;
				l_stat = lhs;
				r_stat = rhs;
			}
		}

restart:
		if (std::numeric_limits<F>::digits < exponent_delta) return ret;
		F delta = r_stat.delta(r_stat.exponent());

		if (same_sign) {
			F ceiling = l_stat.delta(l_stat.exponent()+1);
			ceiling -= delta;
			if (ceiling < lhs) {
				auto test = mantissa_bitcount(l_stat.mantissa());
				if (std::numeric_limits<F>::digits < test) return ret;	// would lose the lowest bit
			}
		}

		// controlled subtractive cancellation.
		if (delta_cancel(lhs,rhs,delta)) return 1;
		l_stat = lhs;
		r_stat = rhs;
		ret = 2;
		goto restart;
	}

	template<class T, class F, class F2>
	typename std::enable_if<std::is_base_of<fp_API, T>::value&& std::is_floating_point<F>::value&& std::is_floating_point<F2>::value && (std::numeric_limits<F>::digits<std::numeric_limits<F2>::digits), int>::type rearrange_sum(F& lhs, F2& rhs)
	{
		return 0;
	}

	template<class T, class F, class F2>
	typename std::enable_if<std::is_base_of<fp_API, T>::value&& std::is_floating_point<F>::value&& std::is_floating_point<F2>::value && (std::numeric_limits<F>::digits > std::numeric_limits<F2>::digits), int>::type rearrange_sum(F& lhs, F2& rhs)
	{
		int ret = rearrange_sum<T>(rhs, lhs);
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

	template<class T, class F>
	typename std::enable_if<std::is_base_of<fp_API, T>::value&& std::is_floating_point<F>::value, int>::type rearrange_sum(ISK_INTERVAL<F>& lhs, ISK_INTERVAL<F>& rhs)
	{
		return 0;
	}

	template<class T, class F, class F2>
	typename std::enable_if<std::is_base_of<fp_API, T>::value&& std::is_floating_point<F>::value && std::is_floating_point<F2>::value && (std::numeric_limits<F>::digits < std::numeric_limits<F2>::digits), int>::type rearrange_sum(ISK_INTERVAL<F>& lhs, ISK_INTERVAL<F2>& rhs)
	{
		return 0;
	}

	template<class T, class F, class F2>
	typename std::enable_if<std::is_base_of<fp_API, T>::value&& std::is_floating_point<F>::value && std::is_floating_point<F2>::value && (std::numeric_limits<F>::digits > std::numeric_limits<F2>::digits), int>::type rearrange_sum(ISK_INTERVAL<F>& lhs, ISK_INTERVAL<F2>& rhs)
	{
		int ret = rearrange_sum<T>(rhs, lhs);
		switch (ret)
		{
		case 1: return -1;
		case -1: return 1;
		default: return ret;
		}
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
