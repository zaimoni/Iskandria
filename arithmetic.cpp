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

	template<std::floating_point F> requires(zaimoni::precise_demote_v<F>)
	int rearrange_sum(ISK_INTERVAL<F>& lhs, ISK_INTERVAL<typename zaimoni::precise_demote<F>::type>& rhs)
	{
		return rearrange_sum(reinterpret_cast<ISK_INTERVAL<typename zaimoni::precise_demote<F>::type>&>(lhs), rhs);
	}

	template<std::floating_point F> requires(zaimoni::precise_demote_v<F>)
	int rearrange_sum(ISK_INTERVAL<typename zaimoni::precise_demote<F>::type>& lhs, ISK_INTERVAL<F>& rhs)
	{
		return rearrange_sum(lhs, reinterpret_cast<ISK_INTERVAL<typename zaimoni::precise_demote<F>::type>&>(rhs));
	}

	template<std::floating_point F> int rearrange_sum(std::shared_ptr<fp_API>& lhs, F& rhs)
	{
		int ret = 0;
		auto working(lhs);
		auto src = working.get();
retry:
		if (auto l = dynamic_cast<var_fp<float>*>(src)) {
			if (2 < working.use_count()) {
				std::unique_ptr<std::remove_reference_t<decltype(*l->typed_clone())> > stage(l->typed_clone());
				working = std::shared_ptr<fp_API>(l = stage.release());
			}
			ret = rearrange_sum(l->_x, rhs);
		} else if (auto l = dynamic_cast<var_fp<ISK_INTERVAL<float> >*>(src)) {
			if (2 < working.use_count()) {
				working = std::shared_ptr<fp_API>(src = l->clone());
				if (!(l = dynamic_cast<decltype(l)>(src))) goto retry;
			}
			ret = rearrange_sum(l->_x, rhs);
		} else if (auto l = dynamic_cast<var_fp<double>*>(src)) {
			if (2 < working.use_count()) {
				std::unique_ptr<std::remove_reference_t<decltype(*l->typed_clone())> > stage(l->typed_clone());
				working = std::shared_ptr<fp_API>(l = stage.release());
			}
			ret = rearrange_sum(l->_x, rhs);
		} else if (auto l = dynamic_cast<var_fp<ISK_INTERVAL<double> >*>(src)) {
			if (2 < working.use_count()) {
				working = std::shared_ptr<fp_API>(src = l->clone());
				if (!(l = dynamic_cast<decltype(l)>(src))) goto retry;
			}
			ret = rearrange_sum(l->_x, rhs);
		} else if (auto l = dynamic_cast<var_fp<long double>*>(src)) {
			if (2 < working.use_count()) {
				std::unique_ptr<std::remove_reference_t<decltype(*l->typed_clone())> > stage(l->typed_clone());
				working = std::shared_ptr<fp_API>(l = stage.release());
			}
			ret = rearrange_sum(l->_x, rhs);
		} else if (auto l = dynamic_cast<var_fp<ISK_INTERVAL<long double> >*>(src)) {
			if (2 < working.use_count()) {
				working = std::shared_ptr<fp_API>(src = l->clone());
				if (!(l = dynamic_cast<decltype(l)>(src))) goto retry;
			}
			ret = rearrange_sum(l->_x, rhs);
		}

		if (ret) lhs = working;
		return ret;
	}

	template<std::floating_point F> int rearrange_sum(std::shared_ptr<fp_API>& lhs, ISK_INTERVAL<F>& rhs)
	{
		int ret = 0;
		auto working(lhs);
		auto src = working.get();
retry:
		if (auto l = dynamic_cast<var_fp<float>*>(src)) {
			if (2 < working.use_count()) {
				std::unique_ptr<std::remove_reference_t<decltype(*l->typed_clone())> > stage(l->typed_clone());
				working = std::shared_ptr<fp_API>(l = stage.release());
			}
			ret = rearrange_sum(l->_x, rhs);
		} else if (auto l = dynamic_cast<var_fp<ISK_INTERVAL<float> >*>(src)) {
			if (2 < working.use_count()) {
				working = std::shared_ptr<fp_API>(src = l->clone());
				if (!(l = dynamic_cast<decltype(l)>(src))) goto retry;
			}
			ret = rearrange_sum(l->_x, rhs);
		} else if (auto l = dynamic_cast<var_fp<double>*>(src)) {
			if (2 < working.use_count()) {
				std::unique_ptr<std::remove_reference_t<decltype(*l->typed_clone())> > stage(l->typed_clone());
				working = std::shared_ptr<fp_API>(l = stage.release());
			}
			ret = rearrange_sum(l->_x, rhs);
		} else if (auto l = dynamic_cast<var_fp<ISK_INTERVAL<double> >*>(src)) {
			if (2 < working.use_count()) {
				working = std::shared_ptr<fp_API>(src = l->clone());
				if (!(l = dynamic_cast<decltype(l)>(src))) goto retry;
			}
			ret = rearrange_sum(l->_x, rhs);
		} else if (auto l = dynamic_cast<var_fp<long double>*>(src)) {
			if (2 < working.use_count()) {
				std::unique_ptr<std::remove_reference_t<decltype(*l->typed_clone())> > stage(l->typed_clone());
				working = std::shared_ptr<fp_API>(l = stage.release());
			}
			ret = rearrange_sum(l->_x, rhs);
		} else if (auto l = dynamic_cast<var_fp<ISK_INTERVAL<long double> >*>(src)) {
			if (2 < working.use_count()) {
				working = std::shared_ptr<fp_API>(src = l->clone());
				if (!(l = dynamic_cast<decltype(l)>(src))) goto retry;
			}
			ret = rearrange_sum(l->_x, rhs);
		}

		if (ret) lhs = working;
		return ret;
	}

	int rearrange_sum(std::shared_ptr<fp_API>& lhs, std::shared_ptr<fp_API>& rhs)
	{	// we assume we are being called from the zaimoni::sum object.
		// that is, all of the zero and infinity symbolic processing has already happened.
		int ret = 0;
		auto working(rhs);
		auto src = working.get();
retry:
		if (auto r = dynamic_cast<var_fp<float>*>(src)) {
			if (2 < working.use_count()) {
				std::unique_ptr<std::remove_reference_t<decltype(*r->typed_clone())> > stage(r->typed_clone());
				working = std::shared_ptr<fp_API>(r = stage.release());
			}
			ret = rearrange_sum(lhs, r->_x);
		} else if (auto r = dynamic_cast<var_fp<ISK_INTERVAL<float> >*>(src)) {
			if (2 < working.use_count()) {
				working = std::shared_ptr<fp_API>(src = r->clone());
				if (!(r = dynamic_cast<decltype(r)>(src))) goto retry;
			}
			ret = rearrange_sum(lhs, r->_x);
		} else if (auto r = dynamic_cast<var_fp<double>*>(src)) {
			if (2 < working.use_count()) {
				std::unique_ptr<std::remove_reference_t<decltype(*r->typed_clone())> > stage(r->typed_clone());
				working = std::shared_ptr<fp_API>(r = stage.release());
			}
			ret = rearrange_sum(lhs, r->_x);
		} else if (auto r = dynamic_cast<var_fp<ISK_INTERVAL<double> >*>(src)) {
			if (2 < working.use_count()) {
				working = std::shared_ptr<fp_API>(src = r->clone());
				if (!(r = dynamic_cast<decltype(r)>(src))) goto retry;
			}
			ret = rearrange_sum(lhs, r->_x);
		} else if (auto r = dynamic_cast<var_fp<long double>*>(src)) {
			if (2 < working.use_count()) {
				std::unique_ptr<std::remove_reference_t<decltype(*r->typed_clone())> > stage(r->typed_clone());
				working = std::shared_ptr<fp_API>(r = stage.release());
			}
			ret = rearrange_sum(lhs, r->_x);
		} else if (auto r = dynamic_cast<var_fp<ISK_INTERVAL<long double> >*>(src)) {
			if (2 < working.use_count()) {
				working = std::shared_ptr<fp_API>(src = r->clone());
				if (!(r = dynamic_cast<decltype(r)>(src))) goto retry;
			}
			ret = rearrange_sum(lhs, r->_x);
		}

		if (ret) rhs = working;
		return ret;
	}

	// no-op implementation to enable building
	int rearrange_product(std::shared_ptr<fp_API>& lhs, std::shared_ptr<fp_API>& rhs) { return 0; }

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

	template<std::floating_point F, std::floating_point F2>
	std::enable_if_t<(std::numeric_limits<F>::max_exponent < std::numeric_limits<F2>::max_exponent), fp_API*> eval_quotient(const ISK_INTERVAL<F>& n, const ISK_INTERVAL<F2>& d)
	{
		return eval_quotient(ISK_INTERVAL<F2>(n), d);
	}

	template<std::floating_point F, std::floating_point F2>
	std::enable_if_t<(std::numeric_limits<F>::max_exponent > std::numeric_limits<F2>::max_exponent), fp_API*> eval_quotient(const ISK_INTERVAL<F>& n, const ISK_INTERVAL<F2>& d)
	{
		return eval_quotient(n, ISK_INTERVAL<F>(d));
	}

	template<std::floating_point F>
	std::enable_if_t<zaimoni::precise_demote_v<F>, fp_API*> eval_quotient(const ISK_INTERVAL<F>& n, const ISK_INTERVAL<typename zaimoni::precise_demote<F>::type>& d)
	{
		return eval_quotient(reinterpret_cast<const ISK_INTERVAL<typename zaimoni::precise_demote<F>::type>&>(n), d);
	}

	template<std::floating_point F>
	std::enable_if_t<zaimoni::precise_demote_v<F>, fp_API*> eval_quotient(const ISK_INTERVAL<typename zaimoni::precise_demote<F>::type>& n, const ISK_INTERVAL<F>& d)
	{
		return eval_quotient(n, reinterpret_cast<const ISK_INTERVAL<typename zaimoni::precise_demote<F>::type>&>(d));
	}

	template<std::floating_point F> fp_API* eval_quotient(const std::shared_ptr<fp_API>& n, const ISK_INTERVAL<F>& d)
	{
		if (d == F(0)) throw zaimoni::math::numeric_error("division by zero");	// expected to be caught earlier when called from the quotient class
		if (F(0) > d.lower() && F(0) < d.upper()) throw zaimoni::math::numeric_error("division should result in two disjoint intervals");	// not always, but requires exact zero numerator which should be caught by the quotient class

		auto working(n); // for the count lock
		auto n_src = n.get();
		if (auto l = dynamic_cast<var_fp<float>*>(n_src)) return eval_quotient(ISK_INTERVAL<float>(l->_x), d);
		else if (auto l = dynamic_cast<var_fp<ISK_INTERVAL<float> >*>(n_src)) return eval_quotient(l->_x, d);
		else if (auto l = dynamic_cast<var_fp<double>*>(n_src)) return eval_quotient(ISK_INTERVAL<double>(l->_x), d);
		else if (auto l = dynamic_cast<var_fp<ISK_INTERVAL<double> >*>(n_src)) return eval_quotient(l->_x, d);
		else if (auto l = dynamic_cast<var_fp<long double>*>(n_src)) return eval_quotient(ISK_INTERVAL<long double>(l->_x), d);
		else if (auto l = dynamic_cast<var_fp<ISK_INTERVAL<long double> >*>(n_src)) return eval_quotient(l->_x, d);

		return 0;
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

	fp_API* eval_quotient(const std::shared_ptr<fp_API>& n, const std::shared_ptr<fp_API>& d)
	{	// we currently honor floating point types.  Integral types would also make sense here, mostly
		auto working(d); // for the count lock

		auto d_src = d.get();
		if (auto r = dynamic_cast<var_fp<float>*>(d_src)) return eval_quotient(n, ISK_INTERVAL<float>(r->_x));
		else if (auto r = dynamic_cast<var_fp<ISK_INTERVAL<float> >*>(d_src)) return eval_quotient(n, r->_x);
		else if (auto r = dynamic_cast<var_fp<double>*>(d_src)) return eval_quotient(n, ISK_INTERVAL<double>(r->_x));
		else if (auto r = dynamic_cast<var_fp<ISK_INTERVAL<double> >*>(d_src)) return eval_quotient(n, r->_x);
		else if (auto r = dynamic_cast<var_fp<long double>*>(d_src)) return eval_quotient(n, ISK_INTERVAL<long double>(r->_x));
		else if (auto r = dynamic_cast<var_fp<ISK_INTERVAL<long double> >*>(d_src)) return eval_quotient(n, r->_x);
		return 0;
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

	int sum_score(const std::shared_ptr<fp_API>& lhs)
	{
		auto working(lhs); // for the count lock
		auto src = lhs.get();
		if (auto l = dynamic_cast<var_fp<float>*>(src)) return sum_score(l->_x);
		else if (auto l = dynamic_cast<var_fp<ISK_INTERVAL<float> >*>(src)) return sum_score(l->_x);
		else if (auto l = dynamic_cast<var_fp<double>*>(src)) return sum_score(l->_x);
		else if (auto l = dynamic_cast<var_fp<ISK_INTERVAL<double> >*>(src)) return sum_score(l->_x);
		else if (auto l = dynamic_cast<var_fp<long double>*>(src)) return sum_score(l->_x);
		else if (auto l = dynamic_cast<var_fp<ISK_INTERVAL<long double> >*>(src)) return sum_score(l->_x);
		return std::numeric_limits<int>::min();
	}

	int sum_score(const COW<fp_API>& lhs)
	{
		auto src = lhs.get_c();
		if (auto l = dynamic_cast<const var_fp<float>*>(src)) return sum_score(l->_x);
		else if (auto l = dynamic_cast<const var_fp<ISK_INTERVAL<float> >*>(src)) return sum_score(l->_x);
		else if (auto l = dynamic_cast<const var_fp<double>*>(src)) return sum_score(l->_x);
		else if (auto l = dynamic_cast<const var_fp<ISK_INTERVAL<double> >*>(src)) return sum_score(l->_x);
		else if (auto l = dynamic_cast<const var_fp<long double>*>(src)) return sum_score(l->_x);
		else if (auto l = dynamic_cast<const var_fp<ISK_INTERVAL<long double> >*>(src)) return sum_score(l->_x);
		return std::numeric_limits<int>::min();
	}

	int sum_implemented(const std::shared_ptr<fp_API>& x)
	{
		auto working(x); // for the count lock
		auto src = x.get();
		if (auto r = dynamic_cast<var_fp<float>*>(src)) return sum_score(r->_x);
		else if (auto r = dynamic_cast<var_fp<ISK_INTERVAL<float> >*>(src)) return sum_score(r->_x);
		else if (auto r = dynamic_cast<var_fp<double>*>(src)) return sum_score(r->_x);
		else if (auto r = dynamic_cast<var_fp<ISK_INTERVAL<double> >*>(src)) return sum_score(r->_x);
		else if (auto r = dynamic_cast<var_fp<long double>*>(src)) return sum_score(r->_x);
		else if (auto r = dynamic_cast<var_fp<ISK_INTERVAL<long double> >*>(src)) return sum_score(r->_x);
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

	template<std::floating_point F> int sum_score(const std::shared_ptr<fp_API>& lhs, const ISK_INTERVAL<F>& rhs)
	{
		if (const int l_score = sum_score(lhs); std::numeric_limits<int>::min() < l_score) {
			const int r_score = sum_score(rhs);
			return l_score < r_score ? l_score : r_score;
		}
		return std::numeric_limits<int>::min();
	}

	template<std::floating_point F> int sum_score(const COW<fp_API>& lhs, const ISK_INTERVAL<F>& rhs)
	{
		if (const int l_score = sum_score(lhs); std::numeric_limits<int>::min() < l_score) {
			const int r_score = sum_score(rhs);
			return l_score < r_score ? l_score : r_score;
		}
		return std::numeric_limits<int>::min();
	}

	template<std::floating_point F> int sum_score(const std::shared_ptr<fp_API>& lhs, const F& rhs)
	{
		if (const int l_score = sum_score(lhs); std::numeric_limits<int>::min() < l_score) {
			const int r_score = sum_score(rhs);
			return l_score < r_score ? l_score : r_score;
		}
		return std::numeric_limits<int>::min();
	}

	template<std::floating_point F> int sum_score(const COW<fp_API>& lhs, const F& rhs)
	{
		if (const int l_score = sum_score(lhs); std::numeric_limits<int>::min() < l_score) {
			const int r_score = sum_score(rhs);
			return l_score < r_score ? l_score : r_score;
		}
		return std::numeric_limits<int>::min();
	}

	int sum_score(const std::shared_ptr<fp_API>& lhs, const std::shared_ptr<fp_API>& rhs)
	{
		auto working(rhs); // for the count lock
		auto src = rhs.get();
		if (auto r = dynamic_cast<var_fp<float>*>(src)) return sum_score(lhs, r->_x);
		else if (auto r = dynamic_cast<var_fp<ISK_INTERVAL<float> >*>(src)) return sum_score(lhs, r->_x);
		else if (auto r = dynamic_cast<var_fp<double>*>(src)) return sum_score(lhs, r->_x);
		else if (auto r = dynamic_cast<var_fp<ISK_INTERVAL<double> >*>(src)) return sum_score(lhs, r->_x);
		else if (auto r = dynamic_cast<var_fp<long double>*>(src)) return sum_score(lhs, r->_x);
		else if (auto r = dynamic_cast<var_fp<ISK_INTERVAL<long double> >*>(src)) return sum_score(lhs, r->_x);
		return std::numeric_limits<int>::min();
	}

	int sum_score(const COW<fp_API>& lhs, const COW<fp_API>& rhs)
	{
		auto src = rhs.get_c();
		if (auto r = dynamic_cast<const var_fp<float>*>(src)) return sum_score(lhs, r->_x);
		else if (auto r = dynamic_cast<const var_fp<ISK_INTERVAL<float> >*>(src)) return sum_score(lhs, r->_x);
		else if (auto r = dynamic_cast<const var_fp<double>*>(src)) return sum_score(lhs, r->_x);
		else if (auto r = dynamic_cast<const var_fp<ISK_INTERVAL<double> >*>(src)) return sum_score(lhs, r->_x);
		else if (auto r = dynamic_cast<const var_fp<long double>*>(src)) return sum_score(lhs, r->_x);
		else if (auto r = dynamic_cast<const var_fp<ISK_INTERVAL<long double> >*>(src)) return sum_score(lhs, r->_x);
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

	template<std::floating_point F> fp_API* eval_sum(const std::shared_ptr<fp_API>& lhs, const ISK_INTERVAL<F>& rhs)
	{
		auto working(lhs); // for the count lock
		auto src = lhs.get();
		if (auto l = dynamic_cast<var_fp<float>*>(src)) return eval_sum(ISK_INTERVAL<float>(l->_x), rhs);
		else if (auto l = dynamic_cast<var_fp<ISK_INTERVAL<float> >*>(src)) return eval_sum(l->_x, rhs);
		else if (auto l = dynamic_cast<var_fp<double>*>(src)) return eval_sum(ISK_INTERVAL<double>(l->_x), rhs);
		else if (auto l = dynamic_cast<var_fp<ISK_INTERVAL<double> >*>(src)) return eval_sum(l->_x, rhs);
		else if (auto l = dynamic_cast<var_fp<long double>*>(src)) return eval_sum(ISK_INTERVAL<long double>(l->_x), rhs);
		else if (auto l = dynamic_cast<var_fp<ISK_INTERVAL<long double> >*>(src)) return eval_sum(l->_x, rhs);
		return 0;
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

	std::shared_ptr<fp_API> eval_sum(const std::shared_ptr<fp_API>& lhs, const std::shared_ptr<fp_API>& rhs)
	{
		auto working(rhs); // for the count lock
		auto src = rhs.get();
		if (auto r = dynamic_cast<var_fp<float>*>(src)) return std::shared_ptr<fp_API>(eval_sum(lhs, ISK_INTERVAL<float>(r->_x)));
		else if (auto r = dynamic_cast<var_fp<ISK_INTERVAL<float> >*>(src)) return std::shared_ptr<fp_API>(eval_sum(lhs, r->_x));
		else if (auto r = dynamic_cast<var_fp<double>*>(src)) return std::shared_ptr<fp_API>(eval_sum(lhs, ISK_INTERVAL<double>(r->_x)));
		else if (auto r = dynamic_cast<var_fp<ISK_INTERVAL<double> >*>(src)) return std::shared_ptr<fp_API>(eval_sum(lhs, r->_x));
		else if (auto r = dynamic_cast<var_fp<long double>*>(src)) return std::shared_ptr<fp_API>(eval_sum(lhs, ISK_INTERVAL<long double>(r->_x)));
		else if (auto r = dynamic_cast<var_fp<ISK_INTERVAL<long double> >*>(src)) return std::shared_ptr<fp_API>(eval_sum(lhs, r->_x));
		return nullptr;
	}

	std::unique_ptr<fp_API> eval_sum(const COW<fp_API>& lhs, const COW<fp_API>& rhs)
	{
		auto src = rhs.get_c();
		if (auto r = dynamic_cast<const var_fp<float>*>(src)) return std::unique_ptr<fp_API>(eval_sum(lhs, ISK_INTERVAL<float>(r->_x)));
		else if (auto r = dynamic_cast<const var_fp<ISK_INTERVAL<float> >*>(src)) return std::unique_ptr<fp_API>(eval_sum(lhs, r->_x));
		else if (auto r = dynamic_cast<const var_fp<double>*>(src)) return std::unique_ptr<fp_API>(eval_sum(lhs, ISK_INTERVAL<double>(r->_x)));
		else if (auto r = dynamic_cast<const var_fp<ISK_INTERVAL<double> >*>(src)) return std::unique_ptr<fp_API>(eval_sum(lhs, r->_x));
		else if (auto r = dynamic_cast<const var_fp<long double>*>(src)) return std::unique_ptr<fp_API>(eval_sum(lhs, ISK_INTERVAL<long double>(r->_x)));
		else if (auto r = dynamic_cast<const var_fp<ISK_INTERVAL<long double> >*>(src)) return std::unique_ptr<fp_API>(eval_sum(lhs, r->_x));
		return nullptr;
	}

	// this must *not* dynamically allocate a symbolic_fp object
	bool in_place_negate(std::shared_ptr<fp_API>& x)
	{
		auto working(x);
retry:
		auto src = working.get();
		if (auto r = dynamic_cast<var_fp<float>*>(src))  {
			if (2 < working.use_count()) {
				std::unique_ptr<std::remove_reference_t<decltype(*r->typed_clone())> > stage(r->typed_clone());
				working = std::shared_ptr<fp_API>(r = stage.release());
			}
			r->_x = -r->_x;
			x = working;
			return true;
		} else if (auto r = dynamic_cast<var_fp<ISK_INTERVAL<float> >*>(src)) {
			if (2 < working.use_count()) {
				working = std::shared_ptr<fp_API>(src = r->clone());
				if (!(r = dynamic_cast<decltype(r)>(src))) goto retry;
			}
			r->_x = -r->_x;
			x = working;
			return true;
		} else if (auto r = dynamic_cast<var_fp<double>*>(src)) {
			if (2 < working.use_count()) {
				std::unique_ptr<std::remove_reference_t<decltype(*r->typed_clone())> > stage(r->typed_clone());
				working = std::shared_ptr<fp_API>(r = stage.release());
			}
			r->_x = -r->_x;
			x = working;
			return true;
		} else if (auto r = dynamic_cast<var_fp<ISK_INTERVAL<double> >*>(src)) {
			if (2 < working.use_count()) {
				working = std::shared_ptr<fp_API>(src = r->clone());
				if (!(r = dynamic_cast<decltype(r)>(src))) goto retry;
			}
			r->_x = -r->_x;
			x = working;
			return true;
		} else if (auto r = dynamic_cast<var_fp<long double>*>(src)) {
			if (2 < working.use_count()) {
				std::unique_ptr<std::remove_reference_t<decltype(*r->typed_clone())> > stage(r->typed_clone());
				working = std::shared_ptr<fp_API>(r = stage.release());
			}
			r->_x = -r->_x;
			x = working;
			return true;
		} else if (auto r = dynamic_cast<var_fp<ISK_INTERVAL<long double> >*>(src)) {
			if (2 < working.use_count()) {
				working = std::shared_ptr<fp_API>(src = r->clone());
				if (!(r = dynamic_cast<decltype(r)>(src))) goto retry;
			}
			r->_x = -r->_x;
			x = working;
			return true;
		} else if (auto r = dynamic_cast<symbolic_fp*>(src)) {
			if (2 < working.use_count()) {
				working = std::shared_ptr<fp_API>(src = r->clone());
				if (!(r = dynamic_cast<decltype(r)>(src))) goto retry;
			}
			r->self_negate();
			if (auto test = r->destructive_eval()) x = test;
			else x = working;
			return true;
		}
		return false;
	}

	bool in_place_negate(COW<fp_API>& x)
	{
retry:
		if (auto r = x.get_rw<var_fp<float> >()) {
			if (!r->first) x = std::unique_ptr<fp_API>(r->first = r->second->typed_clone());
			auto exec = r->first;
			exec->_x = -exec->_x;
			return true;
		}
		if (auto r = x.get_rw<var_fp<ISK_INTERVAL<float> > >()) {
			if (!r->first) {
				x = std::unique_ptr<fp_API>(r->second->clone());
				r = x.get_rw<var_fp<ISK_INTERVAL<float> > >();
				if (!r) goto retry;
			}
			auto exec = r->first;
			exec->_x = -exec->_x;
			return true;
		}
		if (auto r = x.get_rw<var_fp<double> >()) {
			if (!r->first) x = std::unique_ptr<fp_API>(r->first = r->second->typed_clone());
			auto exec = r->first;
			exec->_x = -exec->_x;
			return true;
		}
		if (auto r = x.get_rw<var_fp<ISK_INTERVAL<double> > >()) {
			if (!r->first) {
				x = std::unique_ptr<fp_API>(r->second->clone());
				r = x.get_rw<var_fp<ISK_INTERVAL<double> > >();
				if (!r) goto retry;
			}
			auto exec = r->first;
			exec->_x = -exec->_x;
			return true;
		}
		if (auto r = x.get_rw<var_fp<long double> >()) {
			if (!r->first) x = std::unique_ptr<fp_API>(r->first = r->second->typed_clone());
			auto exec = r->first;
			exec->_x = -exec->_x;
			return true;
		}
		if (auto r = x.get_rw<var_fp<ISK_INTERVAL<long double> > >()) {
			if (!r->first) {
				x = std::unique_ptr<fp_API>(r->second->clone());
				r = x.get_rw<var_fp<ISK_INTERVAL<long double> > >();
				if (!r) goto retry;
			}
			auto exec = r->first;
			exec->_x = -exec->_x;
			return true;
		}
		if (auto r = x.get_rw<symbolic_fp>()) {
			if (!r->first) {
				x = std::unique_ptr<fp_API>(r->second->clone());
				r = x.get_rw<symbolic_fp>();
				if (!r) goto retry;
			}
			auto exec = r->first;
			exec->self_negate();
			if (auto test = exec->destructive_eval()) x = test;
			return true;
		}
		return false;
	}

	bool in_place_square(std::shared_ptr<fp_API>& x)
	{
		auto working(x);
retry:
		auto src = working.get();
		if (auto r = dynamic_cast<var_fp<float>*>(src)) {
			if (would_overflow<decltype(r->_x)>::square(r->_x)) { // upgrade resolution
				working = std::shared_ptr<fp_API>(new var_fp<double>(r->_x));
				goto retry;
			}
			if (2 < working.use_count()) working = std::shared_ptr<fp_API>(r = r->typed_clone());
			auto stage = square(ISK_INTERVAL<float>(r->_x));
			if (auto test = zaimoni::detail::var_fp_impl<ISK_INTERVAL<float> >::clone(stage)) x = std::shared_ptr<fp_API>(test);
			else x = std::shared_ptr<fp_API>(new var_fp<ISK_INTERVAL<float> >(stage));
			return true;
		} else if (auto r = dynamic_cast<var_fp<ISK_INTERVAL<float> >*>(src)) {
			if (would_overflow<decltype(r->_x)>::square(r->_x)) { // upgrade resolution
				working = std::shared_ptr<fp_API>(new var_fp<ISK_INTERVAL<double> >(r->_x));
				goto retry;
			}
			if (2 < working.use_count()) {
				working = std::shared_ptr<fp_API>(src = r->clone());
				if (!(r = dynamic_cast<decltype(r)>(src))) goto retry;
			}
			r->_x = square(r->_x);
			x = working;
			return true;
		} else if (auto r = dynamic_cast<var_fp<double>*>(src)) {
			if (auto extreme = would_overflow<decltype(r->_x)>::square(r->_x)) {
				auto delta = extreme / 2 + (0 < extreme ? 1 : -1);
				std::unique_ptr<std::remove_reference_t<decltype(*r)> > stage_arg(r->typed_clone());
				stage_arg->scal_bn(-delta);
				std::unique_ptr<symbolic_fp> stage(new symbolic_fp(std::move(stage_arg), delta));
				working = std::move(stage);
				goto retry;
			}
			if (2 < working.use_count()) working = std::shared_ptr<fp_API>(r = r->typed_clone());
			auto stage = square(ISK_INTERVAL<double>(r->_x));
			if (auto test = zaimoni::detail::var_fp_impl<ISK_INTERVAL<double> >::clone(stage)) x = std::shared_ptr<fp_API>(test);
			else x = std::shared_ptr<fp_API>(new var_fp<ISK_INTERVAL<double> >(stage));
			return true;
		} else if (auto r = dynamic_cast<var_fp<ISK_INTERVAL<double> >*>(src)) {
			if (auto extreme = would_overflow<decltype(r->_x)>::square(r->_x)) {
				auto delta = extreme / 2 + (0 < extreme ? 1 : -1);
				std::unique_ptr<fp_API> stage_arg(r->clone());
				stage_arg->scal_bn(-delta);
				std::unique_ptr<symbolic_fp> stage(new symbolic_fp(std::move(stage_arg), delta));
				working = std::move(stage);
				goto retry;
			}
			if (2 < working.use_count()) {
				working = std::shared_ptr<fp_API>(src = r->clone());
				if (!(r = dynamic_cast<decltype(r)>(src))) goto retry;
			}
			r->_x = square(r->_x);
			x = working;
			return true;
		} else if (auto r = dynamic_cast<var_fp<long double>*>(src)) {
			if (auto extreme = would_overflow<decltype(r->_x)>::square(r->_x)) {
				auto delta = extreme / 2 + (0 < extreme ? 1 : -1);
				std::unique_ptr<std::remove_reference_t<decltype(*r)> > stage_arg(r->typed_clone());
				stage_arg->scal_bn(-delta);
				std::unique_ptr<symbolic_fp> stage(new symbolic_fp(std::move(stage_arg), delta));
				working = std::move(stage);
				goto retry;
			}
			if (2 < working.use_count()) working = std::shared_ptr<fp_API>(r = r->typed_clone());
			auto stage = square(ISK_INTERVAL<long double>(r->_x));
			if (auto test = zaimoni::detail::var_fp_impl<ISK_INTERVAL<long double> >::clone(stage)) x = std::shared_ptr<fp_API>(test);
			else x = std::shared_ptr<fp_API>(new var_fp<ISK_INTERVAL<long double> >(stage));
			return true;
		} else if (auto r = dynamic_cast<var_fp<ISK_INTERVAL<long double> >*>(src)) {
			if (auto extreme = would_overflow<decltype(r->_x)>::square(r->_x)) {
				auto delta = extreme / 2 + (0 < extreme ? 1 : -1);
				std::unique_ptr<fp_API> stage_arg(r->clone());
				stage_arg->scal_bn(-delta);
				std::unique_ptr<symbolic_fp> stage(new symbolic_fp(std::move(stage_arg), delta));
				working = std::move(stage);
				goto retry;
			}
			if (2 < working.use_count()) {
				working = std::shared_ptr<fp_API>(src = r->clone());
				if (!(r = dynamic_cast<decltype(r)>(src))) goto retry;
			}
			r->_x = square(r->_x);
			x = working;
			return true;
		} else if (auto r = dynamic_cast<power_fp*>(src)) {
			if (2 < working.use_count()) {
				working = std::shared_ptr<fp_API>(src = r->clone());
				if (!(r = dynamic_cast<decltype(r)>(src))) goto retry;
			}
			return r->self_square();
		} else if (auto r = dynamic_cast<symbolic_fp*>(src)) {
			if (2 < working.use_count()) {
				working = std::shared_ptr<fp_API>(src = r->clone());
				if (!(r = dynamic_cast<decltype(r)>(src))) goto retry;
			}
			return r->self_square();
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
				x = std::unique_ptr<symbolic_fp>(new symbolic_fp(std::move(stage_arg), delta));
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
				x = std::unique_ptr<symbolic_fp>(new symbolic_fp(std::move(stage_arg), delta));
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

	bool scal_bn(std::shared_ptr<fp_API>& x, intmax_t& scale)
	{
		if (0 == scale) return true;	// no-op
		auto working(x);
		auto actual = x->scal_bn_is_safe(scale);
		if (0 == actual) return false;
		if (2 < working.use_count()) working = std::shared_ptr<fp_API>(working->clone());
		try {
			working->scal_bn(actual);
			scale -= actual;
			x = working;
			return true;
		} catch (std::runtime_error& e) {
			return false;
		}
	}

	bool scal_bn(COW<fp_API>& x, intmax_t& scale)
	{
		if (0 == scale) return true;	// no-op
		auto actual = x.get_c()->scal_bn_is_safe(scale);
		if (0 == actual) return false;
		try {
			x.get()->scal_bn(actual);
			return true;
		} catch (std::runtime_error& e) {
			return false;
		}
	}

}	// namespace math

std::shared_ptr<fp_API> operator+(const std::shared_ptr<fp_API>& lhs, const std::shared_ptr<fp_API>& rhs)
{
	std::unique_ptr<sum> ret(new sum());
	ret->append_term(lhs);
	ret->append_term(rhs);
	return std::shared_ptr<fp_API>(ret.release());
}

std::shared_ptr<fp_API>& operator+=(std::shared_ptr<fp_API>& lhs, const std::shared_ptr<fp_API>& rhs)
{
	auto src(lhs);
	if (auto r = dynamic_cast<sum*>(src.get())) {
		if (2 < src.use_count()) {
			std::unique_ptr<sum> staging(new sum(*r));
			lhs = std::shared_ptr<fp_API>(r = staging.release());
		}
		r->append_term(rhs);
	} else {	// yes, want the count-lock to leak into the else clause
		lhs = lhs + rhs;
	}

	return lhs;
}

std::shared_ptr<fp_API> operator*(const std::shared_ptr<fp_API>& lhs, const std::shared_ptr<fp_API>& rhs)
{
	std::unique_ptr<product> ret(new product());
	ret->append_term(lhs);
	ret->append_term(rhs);
	return std::shared_ptr<fp_API>(ret.release());
}

std::shared_ptr<fp_API> operator/(const std::shared_ptr<fp_API>& lhs, const std::shared_ptr<fp_API>& rhs)
{
	return std::shared_ptr<fp_API>(new quotient(lhs, rhs));
}

std::shared_ptr<fp_API> operator-(const std::shared_ptr<fp_API>& lhs)
{
	std::shared_ptr<fp_API> ret(lhs->clone());
	if (zaimoni::math::in_place_negate(ret)) return ret;
	std::unique_ptr<symbolic_fp> staging(new symbolic_fp(ret));
	staging->self_negate();
	return std::shared_ptr<fp_API>(staging.release());
}

std::shared_ptr<fp_API> scalBn(const std::shared_ptr<fp_API>& src, intmax_t scale) {
	std::shared_ptr<fp_API> ret(src->clone());
	ret->scal_bn(scale);
	return ret;
}

COW<fp_API> scalBn(const COW<fp_API>& src, intmax_t scale) {
	std::unique_ptr<fp_API> ret(src.get_c()->clone());
	ret->scal_bn(scale);
	return ret;
}

std::shared_ptr<fp_API> pow(const std::shared_ptr<fp_API>& base, const std::shared_ptr<fp_API>& exponent)
{
	// base case
	return std::shared_ptr<fp_API>(new power_fp(base, exponent));
}

void self_scalBn(std::shared_ptr<fp_API>& src, intmax_t scale)
{
	auto working(src);
	if (2 < working.use_count()) working = std::shared_ptr<fp_API>(working->clone());
	working->scal_bn(scale);
	src = working;
}

void self_scalBn(COW<fp_API>& src, intmax_t scale)
{
	src.get()->scal_bn(scale);
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
