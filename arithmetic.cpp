// order matters: interval must be before var.hpp and eval.hpp to prevent compile errors
#include "interval_shim.hpp"
#include "arithmetic.hpp"
#include "Zaimoni.STL/var.hpp"
#include "symbolic_fp.hpp"
#include "power_fp.hpp"
#include "quotient.hpp"
#include "product.hpp"
#include "sum.hpp"
#include "complex.hpp"

namespace zaimoni {

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
};

namespace math {

	// rearrange_sum support
	template<std::floating_point F> int rearrange_sum(F& lhs, F& rhs)
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
restart:
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

				lhs = std::scalbn(bias, l_stat.exponent() + 1);
				rhs = std::scalbn(anchor, l_stat.exponent());
				ret = 2;
				l_stat = lhs;
				r_stat = rhs;
				goto restart;
			}
		}

		if (std::numeric_limits<F>::digits < exponent_delta) return ret;

		F delta = r_stat.delta(r_stat.exponent());

		if (same_sign) {
			const auto lhs_safe(l_stat.safe_add_exponents());
			if (lhs_safe.second < r_stat.exponent()) delta = r_stat.delta(lhs_safe.second);
		}

		// controlled subtractive cancellation.
		if (delta_cancel(lhs,rhs,delta)) return 1;
		l_stat = lhs;
		r_stat = rhs;
		ret = 2;
		goto restart;
	}

	template<std::floating_point F, std::floating_point F2> requires(std::numeric_limits<F>::digits < std::numeric_limits<F2>::digits)
	int rearrange_sum(F& lhs, F2& rhs)
	{
		FATAL("need to implement");
		return 0;
	}

	template<std::floating_point F, std::floating_point F2> requires(std::numeric_limits<F>::digits > std::numeric_limits<F2>::digits)
	int rearrange_sum(F& lhs, F2& rhs)
	{
		int ret = rearrange_sum(rhs, lhs);
		switch (ret)
		{
		case 1: return -1;
		case -1: return 1;
		default: return ret;
		}
	}

	// CLang: sizeof(long double)==sizeof(double)
	template<std::floating_point F> requires(zaimoni::precise_demote_v<F>)
	int rearrange_sum(F& lhs, typename zaimoni::precise_demote<F>::type& rhs)
	{
		return rearrange_sum(reinterpret_cast<typename zaimoni::precise_demote<F>::type&>(lhs), rhs);
	}

	template<std::floating_point F> requires(zaimoni::precise_demote_v<F>)
	int rearrange_sum(typename zaimoni::precise_demote<F>::type& lhs, F& rhs)
	{
		return rearrange_sum(lhs, reinterpret_cast<typename zaimoni::precise_demote<F>::type&>(rhs));
	}

	template<std::floating_point F, std::floating_point F2> int rearrange_sum(F& lhs, ISK_INTERVAL<F2>& rhs)
	{
		FATAL("need to implement");
		return 0;
	}

	template<std::floating_point F, std::floating_point F2> int rearrange_sum(ISK_INTERVAL<F>& lhs, F2& rhs)
	{
		switch (int ret = rearrange_sum(rhs, lhs))
		{
		case 1: return -1;
		case -1: return 1;
		default: return ret;
		}
	}

	template<class T>
	void self_intersect(std::pair<T, T>& lhs, const std::pair<T, T>& rhs)	// prototype
	{
		if (lhs.first < rhs.first) lhs.first = rhs.first;
		if (lhs.second > rhs.second) lhs.second = rhs.second;
	}

	template<std::floating_point F> int rearrange_sum(ISK_INTERVAL<F>& lhs, ISK_INTERVAL<F>& rhs)
	{
		F working[4] = { lhs.lower(),lhs.upper(),rhs.lower(),rhs.upper() };

		// coordinate-wise rearrange_sum (failover, does not completely work)
		// legal values: -2...2
		const int l_code = rearrange_sum(working[0], working[2]);
		const int u_code = rearrange_sum(working[1], working[3]);
		assert(-2 <= l_code && 2 >= l_code);
		assert(-2 <= u_code && 2 >= u_code);
		if (0 == l_code && 0 == u_code) return 0;
		const int CRM_code = 5 * l_code + u_code;
		int ret = 0;
		switch (CRM_code)
		{
		case 0: return 0;	// no change
		case 5 * -2 - 2: FATAL("unexpected double-cancellation of upper and lower bounds");
		case 5 * -2 - 1:
			assert(0 < working[3]);
			lhs = F(0);
			rhs.assign(F(0), working[3]);
			return -1;
		case 5 * -2 + 1:
			assert(0 < working[1]);
			lhs = F(0);
			rhs.assign(F(0), working[1]);
			return -1;
		case 5 * -2 + 0:
		case 5 * -2 + 2:
			assert(0 < working[1] || 0 < working[3]);
			if (0 > working[1] || 0 > working[3]) {
				ret = -3;
				break;	// not normalizable (possibly should error out)
			}
			lhs.assign(F(0), working[1]);
			rhs.assign(F(0), working[3]);
			return 2;

		case 5 * -1 - 2:
			assert(0 > working[2]);
			lhs = F(0);
			rhs.assign(working[2], F(0));
			return -1;
		case 5 * 1 + -2:
			assert(0 > working[0]);
			lhs = F(0);
			rhs.assign(F(0), working[0]);
			return -1;
		case 5 * 0 - 2:
		case 5 * 2 - 2:
			assert(0 > working[0] || 0 > working[2]);
			if (0 < working[0] || 0 < working[2]) {
				ret = -3;
				break;	// not normalizable (possibly should error out)
			}
			lhs.assign(working[0], F(0));
			rhs.assign(working[2], F(0));
			return 2;

		case 5 * -1 - 1:
			assert(working[2] <= working[3]);
			lhs = F(0);
			rhs.assign(working[2], working[3]);
			return -1;
		case 5 * -1 + 1:
			assert(working[2] <= working[1]);
			lhs = F(0);
			rhs.assign(working[2], working[1]);
			return -1;
		case 5 * 1 - 1:
			assert(working[0] <= working[3]);
			lhs = F(0);
			rhs.assign(working[0], working[3]);
			return -1;
		case 5 * 1 + 1:
			assert(working[0] <= working[1]);
			lhs = F(0);
			rhs.assign(working[0], working[1]);
			return -1;

		default:
			if (working[0] > working[1]) {
				if (working[2] <= working[1] && working[0] <= working[3]) swap(working[0], working[2]);
				else if (working[0] <= working[3] && working[2] <= working[1]) swap(working[1], working[3]);
				else {
					ret = -3;
					break;	// not normalizable (possibly should error out)
				}
			}
			else if (working[2] > working[3]) {
				if (working[2] <= working[1] && working[0] <= working[3]) swap(working[0], working[2]);
				else if (working[0] <= working[3] && working[2] <= working[1]) swap(working[1], working[3]);
				else {
					ret = -3;
					break;	// not normalizable (possibly should error out)
				}
			}
			ret = 2;
			break;
		}
		if (-3 == ret) {	// results of rearrange_sum were non-normalizable, re-initialize
			working[0] = lhs.lower();
			working[1] = lhs.upper();
			working[2] = rhs.lower();
			working[3] = rhs.upper();
			ret = 0;
		}
		// version of fp_stats for intervals would make sense here
restart:
		fp_stats<F> stats[4] = { fp_stats<F>(working[0]), fp_stats<F>(working[1]),  fp_stats<F>(working[2]),  fp_stats<F>(working[3]) };
		// \todo try to get at least one of the two pairs "very close" in endpoints

		if (0 < working[0] && 0 < working[2]) {
			const int upper_parity = (working[1] <= working[3]) ? 1 : -1;
			auto safe_add_exponent = stats[2+upper_parity].safe_add_exponents();
			self_intersect(safe_add_exponent, stats[1 + upper_parity].safe_add_exponents());
			if (safe_add_exponent.first <= safe_add_exponent.second) {
				auto safe_subtract_exponent = stats[2 - upper_parity].safe_subtract_exponents();
				self_intersect(safe_subtract_exponent, stats[1 - upper_parity].safe_subtract_exponents());
				if (safe_subtract_exponent.first <= safe_subtract_exponent.second) {
					self_intersect(safe_add_exponent, safe_subtract_exponent);
					if (safe_add_exponent.first <= safe_add_exponent.second) {
						F delta = stats[2 + upper_parity].delta(safe_add_exponent.second);
						ret = 2;
						bool lower_cancel = delta_cancel(working[1 + upper_parity], working[1 - upper_parity], delta);
						bool upper_cancel = delta_cancel(working[2 + upper_parity], working[2 - upper_parity], delta);
						if (lower_cancel || upper_cancel) goto final_exit;
						goto restart;
					}
					if (stats[1 + upper_parity].exponent() == stats[2 + upper_parity].exponent()) {
						int test = stats[2 + upper_parity].exponent() - std::numeric_limits<F>::digits;
						if (safe_subtract_exponent.first <= test && safe_subtract_exponent.second >= test) FATAL("need trailing-bit kill heuristic");
					}
				}
			}
			// retry, but independently: upper bound first
			safe_add_exponent = stats[2 + upper_parity].safe_add_exponents();
			self_intersect(safe_add_exponent, stats[2 - upper_parity].safe_add_exponents());
			{
				const F backup[2] = { working[2 - upper_parity], working[2 + upper_parity] };
				while (safe_add_exponent.first <= safe_add_exponent.second) {
					const bool upper_cancel = delta_cancel(working[2 + upper_parity], working[2 - upper_parity], stats[2 + upper_parity].delta(safe_add_exponent.second));
					if (working[1 - upper_parity] > working[2 - upper_parity]) {
						// denormalized: retry
						working[2 - upper_parity] = backup[0];
						working[2 + upper_parity] = backup[1];
						safe_add_exponent.second--;
						continue;
					}
					ret = 2;
					if (upper_cancel) break;
					goto restart;
				}
			}

			const int lower_parity = (working[0] <= working[2]) ? 1 : -1;
			safe_add_exponent = stats[1 + lower_parity].safe_add_exponents();
			self_intersect(safe_add_exponent, stats[1 - lower_parity].safe_add_exponents());
			{
				const F backup[2] = { working[1 - lower_parity], working[1 + lower_parity] };
				while (safe_add_exponent.first <= safe_add_exponent.second) {
					const bool upper_cancel = delta_cancel(working[1 + lower_parity], working[1 - lower_parity], stats[1 + lower_parity].delta(safe_add_exponent.second));
					if (working[1 - lower_parity] > working[2 - lower_parity]) {
						// denormalized: retry
						working[1 - lower_parity] = backup[0];
						working[1 + lower_parity] = backup[1];
						safe_add_exponent.second--;
						continue;
					}
					ret = 2;
					if (upper_cancel) break;
					goto restart;
				}
			}
			goto final_exit;
		}
		if (0 > working[1] && 0 > working[2]) FATAL("need to mirror interval-arithmetic double-positive block for double-negative");

final_exit:
		if (2 == ret) {
			lhs.assign(working[0], working[1]);
			rhs.assign(working[2], working[3]);
		}
		return ret;
	}

	template<std::floating_point F, std::floating_point F2> requires(std::numeric_limits<F>::digits < std::numeric_limits<F2>::digits)
	int rearrange_sum(ISK_INTERVAL<F>& lhs, ISK_INTERVAL<F2>& rhs)
	{
		FATAL("need to implement");
		return 0;
	}

	template<std::floating_point F, std::floating_point F2> requires(std::numeric_limits<F>::digits > std::numeric_limits<F2>::digits)
	int rearrange_sum(ISK_INTERVAL<F>& lhs, ISK_INTERVAL<F2>& rhs)
	{
		switch (int ret = rearrange_sum(rhs, lhs))
		{
		case 1: return -1;
		case -1: return 1;
		default: return ret;
		}
	}

	template<std::floating_point F, std::floating_point F2> requires(std::is_same_v<F2, typename zaimoni::precise_demote<F>::type>)
		int rearrange_sum(ISK_INTERVAL<F>& lhs, ISK_INTERVAL<F2>& rhs)
	{
		return rearrange_sum(reinterpret_cast<ISK_INTERVAL<F2>&>(lhs), rhs);
	}

	template<std::floating_point F, std::floating_point F2> requires(std::is_same_v<F, typename zaimoni::precise_demote<F2>::type>)
		int rearrange_sum(ISK_INTERVAL<F>& lhs, ISK_INTERVAL<F2>& rhs)
	{
		return rearrange_sum(lhs, reinterpret_cast<ISK_INTERVAL<F>&>(rhs));
	}

	template<std::floating_point F> int rearrange_sum(COW<fp_API>& lhs, F& rhs)
	{
		if (auto l = ptr::writeable<var_fp<ISK_INTERVAL<float> > >(lhs)) return rearrange_sum(l->_x, rhs);
		if (auto l = ptr::writeable<var_fp<float> >(lhs)) return rearrange_sum(l->_x, rhs);
		if (auto l = ptr::writeable<var_fp<ISK_INTERVAL<double> > >(lhs)) return rearrange_sum(l->_x, rhs);
		if (auto l = ptr::writeable<var_fp<double> >(lhs)) return rearrange_sum(l->_x, rhs);
		if (auto l = ptr::writeable<var_fp<ISK_INTERVAL<long double> > >(lhs)) return rearrange_sum(l->_x, rhs);
		if (auto l = ptr::writeable<var_fp<long double> >(lhs)) return rearrange_sum(l->_x, rhs);
		return 0;
	}

	template<std::floating_point F> int rearrange_sum(COW<fp_API>& lhs, ISK_INTERVAL<F>& rhs)
	{
		if (auto l = ptr::writeable<var_fp<ISK_INTERVAL<float> > >(lhs)) return rearrange_sum(l->_x, rhs);
		if (auto l = ptr::writeable<var_fp<float> >(lhs)) return rearrange_sum(l->_x, rhs);
		if (auto l = ptr::writeable<var_fp<ISK_INTERVAL<double> > >(lhs)) return rearrange_sum(l->_x, rhs);
		if (auto l = ptr::writeable<var_fp<double> >(lhs)) return rearrange_sum(l->_x, rhs);
		if (auto l = ptr::writeable<var_fp<ISK_INTERVAL<long double> > >(lhs)) return rearrange_sum(l->_x, rhs);
		if (auto l = ptr::writeable<var_fp<long double> >(lhs)) return rearrange_sum(l->_x, rhs);
		return 0;
	}

	int rearrange_sum(COW<fp_API>& lhs, COW<fp_API>& rhs)
	{	// we assume we are being called from the zaimoni::sum object.
		// that is, all of the zero and infinity symbolic processing has already happened.
		// check for opt-in interface
		if (auto l = ptr::writeable<API_sum<fp_API> >(lhs)) {
			if (const int code = l->rearrange_sum(rhs)) return code;
		}

		if (auto r = ptr::writeable<API_sum<fp_API> >(rhs)) {
			switch(const int code = r->rearrange_sum(lhs)) {
			case -1: return 1; // left and right arguments are transposed here
			case 1: return -1;
			default: return code; // otherwise, pass through the code unchanged
			}
		}

		// 2021-07-19: big-bang to std::visit did not work out
		// these RHS do not implement the opt-in interface
		if (auto r = ptr::writeable<var_fp<ISK_INTERVAL<float> > >(rhs)) return rearrange_sum(lhs, r->_x);
		if (auto r = ptr::writeable<var_fp<float> >(rhs)) return rearrange_sum(lhs, r->_x);
		if (auto r = ptr::writeable<var_fp<ISK_INTERVAL<double> > >(rhs)) return rearrange_sum(lhs, r->_x);
		if (auto r = ptr::writeable<var_fp<double> >(rhs)) return rearrange_sum(lhs, r->_x);
		if (auto r = ptr::writeable<var_fp<ISK_INTERVAL<long double> > >(rhs)) return rearrange_sum(lhs, r->_x);
		if (auto r = ptr::writeable<var_fp<long double> >(rhs)) return rearrange_sum(lhs, r->_x);

		return 0;
	}

	// no-op implementation to enable building
	int rearrange_product(COW<fp_API>& lhs, COW<fp_API>& rhs) { return 0; }

	// eval_quotient support
	template<std::floating_point F> fp_API* eval_quotient(const ISK_INTERVAL<F>& n, const ISK_INTERVAL<F>& d)
	{
		try {
			auto ret = n / d;
			if (ret.lower() == ret.upper()) return new var_fp<decltype(ret.upper())>(ret.upper());
			return new var_fp<decltype(ret)>(ret);
		} catch (zaimoni::math::numeric_error& e) {
			return nullptr;
		}
		return nullptr;
	}

	template<std::floating_point F, std::floating_point F2> requires(std::numeric_limits<F>::max_exponent < std::numeric_limits<F2>::max_exponent)
	fp_API* eval_quotient(const ISK_INTERVAL<F>& n, const ISK_INTERVAL<F2>& d)
	{
		return eval_quotient(ISK_INTERVAL<F2>(n), d);
	}

	template<std::floating_point F, std::floating_point F2> requires (std::numeric_limits<F>::max_exponent > std::numeric_limits<F2>::max_exponent)
	fp_API* eval_quotient(const ISK_INTERVAL<F>& n, const ISK_INTERVAL<F2>& d)
	{
		return eval_quotient(n, ISK_INTERVAL<F>(d));
	}

	template<std::floating_point F, std::floating_point F2> requires(std::is_same_v<F2, typename zaimoni::precise_demote<F>::type>)
	fp_API* eval_quotient(const ISK_INTERVAL<F>& n, const ISK_INTERVAL<F2>& d)
	{
		return eval_quotient(reinterpret_cast<const ISK_INTERVAL<F2>&>(n), d);
	}

	template<std::floating_point F, std::floating_point F2> requires(std::is_same_v<F, typename zaimoni::precise_demote<F2>::type>)
	fp_API* eval_quotient(const ISK_INTERVAL<F>& n, const ISK_INTERVAL<F2>& d)
	{
		return eval_quotient(n, reinterpret_cast<const ISK_INTERVAL<F>&>(d));
	}

	template<std::floating_point F> fp_API* eval_quotient(const COW<fp_API>& n, const ISK_INTERVAL<F>& d)
	{
		if (d == F(0)) throw zaimoni::math::numeric_error("division by zero");	// expected to be caught earlier when called from the quotient class
		if (F(0) > d.lower() && F(0) < d.upper()) throw zaimoni::math::numeric_error("division should result in two disjoint intervals");	// not always, but requires exact zero numerator which should be caught by the quotient class

		auto n_src = n.get_c();
		if (auto l = dynamic_cast<const var_fp<float>*>(n_src)) return eval_quotient(ISK_INTERVAL<float>(l->_x), d);
		else if (auto l = dynamic_cast<const var_fp<ISK_INTERVAL<float> >*>(n_src)) return eval_quotient(l->_x, d);
		else if (auto l = dynamic_cast<const var_fp<double>*>(n_src)) return eval_quotient(ISK_INTERVAL<double>(l->_x), d);
		else if (auto l = dynamic_cast<const var_fp<ISK_INTERVAL<double> >*>(n_src)) return eval_quotient(l->_x, d);
		else if (auto l = dynamic_cast<const var_fp<long double>*>(n_src)) return eval_quotient(ISK_INTERVAL<long double>(l->_x), d);
		else if (auto l = dynamic_cast<const var_fp<ISK_INTERVAL<long double> >*>(n_src)) return eval_quotient(l->_x, d);

		return nullptr;
	}

	fp_API* eval_quotient(const COW<fp_API>& n, const COW<fp_API>& d)
	{	// we currently honor floating point types.  Integral types would also make sense here, mostly
		auto d_src = d.get_c();
		if (auto r = dynamic_cast<const var_fp<float>*>(d_src)) return eval_quotient(n, ISK_INTERVAL<float>(r->_x));
		else if (auto r = dynamic_cast<const var_fp<ISK_INTERVAL<float> >*>(d_src)) return eval_quotient(n, r->_x);
		else if (auto r = dynamic_cast<const var_fp<double>*>(d_src)) return eval_quotient(n, ISK_INTERVAL<double>(r->_x));
		else if (auto r = dynamic_cast<const var_fp<ISK_INTERVAL<double> >*>(d_src)) return eval_quotient(n, r->_x);
		else if (auto r = dynamic_cast<const var_fp<long double>*>(d_src)) return eval_quotient(n, ISK_INTERVAL<long double>(r->_x));
		else if (auto r = dynamic_cast<const var_fp<ISK_INTERVAL<long double> >*>(d_src)) return eval_quotient(n, r->_x);
		return nullptr;
	}

	// generally speaking, for floating point numerals we want to destructively add the smallest two exponents first.
	// * minimizes the numerical error which is controlled by the size of the larger absolute-value numeral
	// * may enable further rearrangement
	// so the score should be largest for denormals and smallest near infinity
	template<std::floating_point F> int sum_score(const ISK_INTERVAL<F>& x)
	{
		fp_stats<F> test_l(x.lower());
		fp_stats<F> test_r(x.upper());
		return std::numeric_limits<long double>::max_exponent - (test_l.exponent() < test_r.exponent() ? test_l.exponent() : test_r.exponent());
	}

	template<std::floating_point F> int sum_score(const F& x)
	{
		fp_stats<F> test(x);
		return std::numeric_limits<long double>::max_exponent - test.exponent();
	}

	namespace parse_for {
		// uintmax_t intentionally omitted
		// typed_clone destinations must be tested after anything that could clone to them
		std::optional<std::variant<const var_fp<float>*,
			const var_fp<ISK_INTERVAL<float> >*,
			const var_fp<double>*,
			const var_fp<ISK_INTERVAL<double> >*,
			const var_fp<long double>*,
			const var_fp<ISK_INTERVAL<long double> >*
		> > sum_score(const eval_to_ptr<fp_API>::eval_type& src) {
			auto test = src.get_c();
			if (auto x = dynamic_cast<const var_fp<float>*>(test)) return x;
			else if (auto x = dynamic_cast<const var_fp<ISK_INTERVAL<float> >*>(test)) return x;
			else if (auto x = dynamic_cast<const var_fp<double>*>(test)) return x;
			else if (auto x = dynamic_cast<const var_fp<ISK_INTERVAL<double> >*>(test)) return x;
			else if (auto x = dynamic_cast<const var_fp<long double>*>(test)) return x;
			else if (auto x = dynamic_cast<const var_fp<ISK_INTERVAL<long double> >*>(test)) return x;
			return std::nullopt;
		}
	}

	namespace score {
		struct sum
		{
			template<std::floating_point F> int operator()(const ISK_INTERVAL<F>& x)
			{
				fp_stats<F> test_l(x.lower());
				fp_stats<F> test_r(x.upper());
				return std::numeric_limits<long double>::max_exponent - (test_l.exponent() < test_r.exponent() ? test_l.exponent() : test_r.exponent());
			}

			template<std::floating_point F> int operator()(const F& x)
			{
				fp_stats<F> test(x);
				return std::numeric_limits<long double>::max_exponent - test.exponent();
			}

			template<class T> int operator()(const var_fp<T>* x) /* requires requires() { operator()(x->_x); } */ // 2021-07-18 this requires clause stops the build on MSVC++
			{
				return operator()(x->_x);
			}
		};
	}

	int sum_score(const COW<fp_API>& lhs)
	{
		if (auto test = parse_for::sum_score(lhs)) return std::visit(score::sum(), *test);
		return std::numeric_limits<int>::min();
	}

	int sum_implemented(const COW<fp_API>& x)
	{
		auto src = x.get_c();
		if (auto r = dynamic_cast<const var_fp<float>*>(src)) return sum_score(r->_x);
		else if (auto r = dynamic_cast<const var_fp<ISK_INTERVAL<float> >*>(src)) return sum_score(r->_x);
		else if (auto r = dynamic_cast<const var_fp<double>*>(src)) return sum_score(r->_x);
		else if (auto r = dynamic_cast<const var_fp<ISK_INTERVAL<double> >*>(src)) return sum_score(r->_x);
		else if (auto r = dynamic_cast<const var_fp<long double>*>(src)) return sum_score(r->_x);
		else if (auto r = dynamic_cast<const var_fp<ISK_INTERVAL<long double> >*>(src)) return sum_score(r->_x);
		return std::numeric_limits<int>::min();
	}

	int sum_score(const COW<fp_API>& lhs, const COW<fp_API>& rhs)
	{
		if (const int l_score = sum_score(lhs); std::numeric_limits<int>::min() < l_score) {
			const int r_score = sum_score(rhs);
			return l_score < r_score ? l_score : r_score;
		}
		return std::numeric_limits<int>::min();
	}

	template<std::floating_point F> fp_API* eval_sum(const ISK_INTERVAL<F>& lhs, const ISK_INTERVAL<F>& rhs)
	{
		try {
			auto ret = lhs + rhs;
			if (ret.lower() == ret.upper()) return new var_fp<typename ISK_INTERVAL<F>::base_type>(ret.upper());
			return new var_fp<decltype(ret)>(ret);
		}
		catch (zaimoni::math::numeric_error& e) {
			// doesn't help w/Boost, but our internal interval type should like to throw on overflow, etc.
			return nullptr;
		}
		return nullptr;
	}

	template<std::floating_point F, std::floating_point F2>
	std::enable_if_t<(std::numeric_limits<F>::max_exponent < std::numeric_limits<F2>::max_exponent), fp_API*> eval_sum(const ISK_INTERVAL<F>& lhs, const ISK_INTERVAL<F2>& rhs)
	{
		return eval_sum(ISK_INTERVAL<F2>(lhs), rhs);
	}

	template<std::floating_point F, std::floating_point F2>
	std::enable_if_t<(std::numeric_limits<F>::max_exponent > std::numeric_limits<F2>::max_exponent), fp_API*> eval_sum(const ISK_INTERVAL<F>& lhs, const ISK_INTERVAL<F2>& rhs)
	{
		return eval_sum(lhs, ISK_INTERVAL<F>(rhs));
	}

	template<std::floating_point F>
	std::enable_if_t<zaimoni::precise_demote_v<F>, fp_API*> eval_sum(const ISK_INTERVAL<F>& lhs, const ISK_INTERVAL<typename zaimoni::precise_demote<F>::type>& rhs)
	{
		return eval_sum(reinterpret_cast<const ISK_INTERVAL<typename zaimoni::precise_demote<F>::type>&>(lhs), rhs);
	}

	template<std::floating_point F>
	std::enable_if_t<zaimoni::precise_demote_v<F>, fp_API*> eval_sum(const ISK_INTERVAL<typename zaimoni::precise_demote<F>::type>& lhs, const ISK_INTERVAL<F>& rhs)
	{
		return eval_sum(lhs, reinterpret_cast<const ISK_INTERVAL<typename zaimoni::precise_demote<F>::type>&>(rhs));
	}

	template<std::floating_point F> fp_API* eval_sum(const COW<fp_API>& lhs, const ISK_INTERVAL<F>& rhs)
	{
		auto src = lhs.get_c();
		if (auto l = dynamic_cast<const var_fp<float>*>(src)) return eval_sum(ISK_INTERVAL<float>(l->_x), rhs);
		else if (auto l = dynamic_cast<const var_fp<ISK_INTERVAL<float> >*>(src)) return eval_sum(l->_x, rhs);
		else if (auto l = dynamic_cast<const var_fp<double>*>(src)) return eval_sum(ISK_INTERVAL<double>(l->_x), rhs);
		else if (auto l = dynamic_cast<const var_fp<ISK_INTERVAL<double> >*>(src)) return eval_sum(l->_x, rhs);
		else if (auto l = dynamic_cast<const var_fp<long double>*>(src)) return eval_sum(ISK_INTERVAL<long double>(l->_x), rhs);
		else if (auto l = dynamic_cast<const var_fp<ISK_INTERVAL<long double> >*>(src)) return eval_sum(l->_x, rhs);
		return nullptr;
	}

	COW<fp_API> eval_sum(const COW<fp_API>& lhs, const COW<fp_API>& rhs)
	{
		if (auto l = dynamic_cast<const API_sum<fp_API>*>(lhs.get_c())) return l->eval_sum(rhs);

		auto src = rhs.get_c();
		if (auto r = dynamic_cast<const API_sum<fp_API>*>(src)) return r->eval_sum(lhs);
		else if (auto r = dynamic_cast<const var_fp<float>*>(src)) return eval_sum(lhs, ISK_INTERVAL<float>(r->_x));
		else if (auto r = dynamic_cast<const var_fp<ISK_INTERVAL<float> >*>(src)) return eval_sum(lhs, r->_x);
		else if (auto r = dynamic_cast<const var_fp<double>*>(src)) return eval_sum(lhs, ISK_INTERVAL<double>(r->_x));
		else if (auto r = dynamic_cast<const var_fp<ISK_INTERVAL<double> >*>(src)) return eval_sum(lhs, r->_x);
		else if (auto r = dynamic_cast<const var_fp<long double>*>(src)) return eval_sum(lhs, ISK_INTERVAL<long double>(r->_x));
		else if (auto r = dynamic_cast<const var_fp<ISK_INTERVAL<long double> >*>(src)) return eval_sum(lhs, r->_x);
		return nullptr;
	}

	namespace parse_for {
		// uintmax_t intentionally omitted
		// typed_clone destinations must be tested after anything that could clone to them
		std::optional<std::variant<API_addinv*,
			var_fp<float>*,
			var_fp<ISK_INTERVAL<float> >*,
			var_fp<double>*,
			var_fp<ISK_INTERVAL<double> >*,
			var_fp<long double>*,
			var_fp<ISK_INTERVAL<long double> >*,
			var_fp<intmax_t>*
		> > negate(eval_to_ptr<fp_API>::eval_type& src) {
			if (auto x = ptr::writeable<API_addinv>(src)) return x;
			if (auto x = ptr::writeable<var_fp<ISK_INTERVAL<float> > >(src)) return x;
			if (auto x = ptr::writeable<var_fp<float> >(src)) return x;
			if (auto x = ptr::writeable<var_fp<ISK_INTERVAL<double> > >(src)) return x;
			if (auto x = ptr::writeable<var_fp<double> >(src)) return x;
			if (auto x = ptr::writeable<var_fp<ISK_INTERVAL<long double> > >(src)) return x;
			if (auto x = ptr::writeable<var_fp<long double> >(src)) return x;
			if (auto x = ptr::writeable<var_fp<intmax_t> >(src)) return x;
			return std::nullopt;
		}
	}

	namespace in_place {
		struct negate
		{
			eval_to_ptr<fp_API>::eval_type& src; // non-copyable isn't an issue

			negate(eval_to_ptr<fp_API>::eval_type& src) : src(src) {}

			void operator()(API_addinv* x) {
				x->self_negate();
				while (fp_API::algebraic_reduce(src));
			}

			void operator()(var_fp<intmax_t>* x) { x->_x = -x->_x; }
			template<std::floating_point F> void operator()(var_fp<F>* x) { x->_x = -x->_x; }
			template<std::floating_point F> void operator()(var_fp<ISK_INTERVAL<F> >* x) { x->_x = -x->_x; }
		};
	}

	// this must *not* dynamically allocate a symbolic_fp object
	bool in_place_negate(eval_to_ptr<fp_API>::eval_type& x)
	{
		if (auto test = parse_for::negate(x)) {
			std::visit(in_place::negate(x), *test);
			return true;
		}
		return false;
	}

	bool in_place_square(COW<fp_API>& x)
	{
retry:
		if (auto r = x.get_rw<var_fp<float> >()) {
			auto test = r->second;
			if (would_overflow<decltype(test->_x)>::square(test->_x)) { // upgrade resolution
				x = std::unique_ptr<fp_API>(new var_fp<double>(test->_x));
				goto upgrade_double;
			}
			auto stage = square(ISK_INTERVAL<float>(test->_x));
			if (auto rewrite = zaimoni::detail::var_fp_impl<decltype(stage)>::clone(stage)) x = std::unique_ptr<fp_API>(rewrite);
			else x = std::unique_ptr<fp_API>(new var_fp<decltype(stage)>(stage));
			return true;
		}
		if (auto r = x.get_rw<var_fp<ISK_INTERVAL<float> > >()) {
			auto test = r->second;
			if (would_overflow<decltype(test->_x)>::square(test->_x)) { // upgrade resolution
				x = std::unique_ptr<fp_API>(new var_fp<ISK_INTERVAL<double> >(test->_x));
				goto upgrade_interval_double;
			}
			auto stage = square(test->_x);
			if (auto rewrite = zaimoni::detail::var_fp_impl<decltype(stage)>::clone(stage)) x = std::unique_ptr<fp_API>(rewrite);
			else x = std::unique_ptr<fp_API>(new var_fp<decltype(stage)>(stage));
			return true;
		}
upgrade_double:
		if (auto r = x.get_rw<var_fp<double> >()) {
			auto test = r->second;
			if (auto extreme = would_overflow<decltype(test->_x)>::square(test->_x)) {
				auto delta = extreme / 2 + (0 < extreme ? 1 : -1);
				std::unique_ptr<std::remove_reference_t<decltype(*(test->typed_clone()))> > stage_arg(test->typed_clone());
				stage_arg->scal_bn(-delta);
				x = std::unique_ptr<symbolic_fp>(new symbolic_fp(stage_arg.release(), delta));
				goto symbolic_overflow;
			}
			auto stage = square(ISK_INTERVAL<double>(test->_x));
			if (auto rewrite = zaimoni::detail::var_fp_impl<decltype(stage)>::clone(stage)) x = std::unique_ptr<fp_API>(rewrite);
			else x = std::unique_ptr<fp_API>(new var_fp<decltype(stage)>(stage));
			return true;
		}
upgrade_interval_double:
		if (auto r = x.get_rw< var_fp<ISK_INTERVAL<double> > >()) {
			auto test = r->second;
			if (auto extreme = would_overflow<decltype(test->_x)>::square(test->_x)) {
				auto delta = extreme / 2 + (0 < extreme ? 1 : -1);
				std::unique_ptr<fp_API> stage_arg(test->clone());
				stage_arg->scal_bn(-delta);
				x = std::unique_ptr<symbolic_fp>(new symbolic_fp(std::move(stage_arg), delta));
				goto symbolic_overflow;
			}
			auto stage = square(test->_x);
			if (auto rewrite = zaimoni::detail::var_fp_impl<decltype(stage)>::clone(stage)) x = std::unique_ptr<fp_API>(rewrite);
			else x = std::unique_ptr<fp_API>(new var_fp<decltype(stage)>(stage));
			return true;
		}
		if (auto r = x.get_rw<var_fp<long double> >()) {
			auto test = r->second;
			if (auto extreme = would_overflow<decltype(test->_x)>::square(test->_x)) {
				auto delta = extreme / 2 + (0 < extreme ? 1 : -1);
				std::unique_ptr<std::remove_reference_t<decltype(*(test->typed_clone()))> > stage_arg(test->typed_clone());
				stage_arg->scal_bn(-delta);
				x = std::unique_ptr<symbolic_fp>(new symbolic_fp(stage_arg.release(), delta));
				goto symbolic_overflow;
			}
			auto stage = square(ISK_INTERVAL<long double>(test->_x));
			if (auto rewrite = zaimoni::detail::var_fp_impl<decltype(stage)>::clone(stage)) x = std::unique_ptr<fp_API>(rewrite);
			else x = std::unique_ptr<fp_API>(new var_fp<decltype(stage)>(stage));
			return true;
		}
		if (auto r = x.get_rw< var_fp<ISK_INTERVAL<long double> > >()) {
			auto test = r->second;
			if (auto extreme = would_overflow<decltype(test->_x)>::square(test->_x)) {
				auto delta = extreme / 2 + (0 < extreme ? 1 : -1);
				std::unique_ptr<fp_API> stage_arg(test->clone());
				stage_arg->scal_bn(-delta);
				x = std::unique_ptr<symbolic_fp>(new symbolic_fp(std::move(stage_arg), delta));
				goto symbolic_overflow;
			}
			auto stage = square(test->_x);
			if (auto rewrite = zaimoni::detail::var_fp_impl<decltype(stage)>::clone(stage)) x = std::unique_ptr<fp_API>(rewrite);
			else x = std::unique_ptr<fp_API>(new var_fp<decltype(stage)>(stage));
			return true;
		}
		if (auto r = x.get_rw<power_fp>()) {
			if (!r->first) {
				x = std::unique_ptr<fp_API>(r->second->clone());
				r = x.get_rw<power_fp>();
				if (!r) goto retry;
			}
			return r->first->self_square();
		}
symbolic_overflow:
		if (auto r = x.get_rw<symbolic_fp>()) {
			if (!r->first) {
				x = std::unique_ptr<fp_API>(r->second->clone());
				r = x.get_rw<symbolic_fp>();
				if (!r) goto retry;
			}
			return r->first->self_square();
		}
		return false;
	}

	bool scal_bn(COW<fp_API>& x, intmax_t& scale)
	{
		if (0 == scale) return true;	// no-op
		auto actual = x.get_c()->scal_bn_is_safe(scale);
		if (0 == actual) return false;
		try {
			x->scal_bn(actual);
			scale -= actual;
			return true;
		} catch (std::runtime_error& e) {
			return false;
		}
	}
}	// namespace math

eval_to_ptr<fp_API>::eval_type operator+(const eval_to_ptr<fp_API>::eval_type& lhs, const eval_to_ptr<fp_API>::eval_type& rhs)
{
	std::unique_ptr<sum> ret(new sum());
	ret->append_term(lhs);
	ret->append_term(rhs);
	return eval_to_ptr<fp_API>::eval_type(ret.release());
}

eval_to_ptr<fp_API>::eval_type& operator+=(eval_to_ptr<fp_API>::eval_type& lhs, const eval_to_ptr<fp_API>::eval_type& rhs)
{
	if (auto r = lhs.get_rw<sum>()) {
		if (!r->first) lhs = std::unique_ptr<fp_API>(r->first = r->second->typed_clone());
		r->first->append_term(rhs);
	} else {
		lhs = lhs + rhs;
	}

	return lhs;
}

eval_to_ptr<fp_API>::eval_type operator*(const eval_to_ptr<fp_API>::eval_type& lhs, const eval_to_ptr<fp_API>::eval_type& rhs)
{
	std::unique_ptr<product> ret(new product());
	ret->append_term(lhs);
	ret->append_term(rhs);
	return eval_to_ptr<fp_API>::eval_type(ret.release());
}

eval_to_ptr<fp_API>::eval_type operator/(const eval_to_ptr<fp_API>::eval_type& lhs, const eval_to_ptr<fp_API>::eval_type& rhs)
{
	return eval_to_ptr<fp_API>::eval_type(new quotient(lhs, rhs));
}

void negate_in_place(eval_to_ptr<fp_API>::eval_type& lhs)
{
	if (!zaimoni::math::in_place_negate(lhs)) {
		std::unique_ptr<symbolic_fp> staging(new symbolic_fp(lhs));
		staging->self_negate();
		lhs = staging.release();
	}
}

eval_to_ptr<fp_API>::eval_type operator-(const eval_to_ptr<fp_API>::eval_type& lhs)
{
	COW<fp_API> ret(lhs);
	negate_in_place(ret);
	return ret;
}

COW<fp_API> scalBn(const COW<fp_API>& src, intmax_t scale) {
	auto ret(src);
	ret->scal_bn(scale);
	return ret;
}

eval_to_ptr<fp_API>::eval_type pow(const eval_to_ptr<fp_API>::eval_type& base, const eval_to_ptr<fp_API>::eval_type& exponent)
{
	// base case
	return eval_to_ptr<fp_API>::eval_type(new power_fp(base, exponent));
}

}	// namespace zaimoni

#ifdef TEST_APP
// fast compile test
// g++ -std=c++14 -otest.exe -Os -D__STDC_LIMIT_MACROS -DTEST_APP arithmetic.cpp -Llib\host.isk -lz_stdio_c -lz_log_adapter -lz_stdio_log -lz_format_util
int main(int argc, char* argv[])
{
	// compile-time checks
	return 0;
}
#endif
