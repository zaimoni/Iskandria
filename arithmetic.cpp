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
#include "fp_interchange.hpp"
#include <typeinfo>

namespace zaimoni {

struct type_to_str {
	template<class T>
	std::string operator()(const T&) { return typeid(T).name(); }
};

struct to_INFORM {
	template<class T> void operator()(const T& src) { INFORM(src); }
	template<class T> void operator()(const var_fp<T>*& src) { INFORM(src->_x); }
};

struct to_varfp
{
	template<class T> fp_API* operator()(const T& src) requires requires() { new var_fp(src); }
	{
		return new var_fp(src);
	}
};

struct to_float
{
	auto operator()(const var_fp<intmax_t>* src) {
		fp_interchange relay(src->_x);
		return relay.to_float();
	}

	auto operator()(const var_fp<uintmax_t>* src) {
		fp_interchange relay(src->_x);
		return relay.to_float();
	}
};

namespace math {

	namespace parse_for {
		// typed_clone destinations must be tested after anything that could clone to them
		std::optional<std::variant<var_fp<float>*,
			var_fp<ISK_INTERVAL<float> >*,
			var_fp<double>*,
			var_fp<ISK_INTERVAL<double> >*,
			var_fp<long double>*,
			var_fp<ISK_INTERVAL<long double> >*,
			var_fp<intmax_t>*,
			var_fp<uintmax_t>*
		> > primitive(eval_to_ptr<fp_API>::eval_type& src) {
			if (auto x = ptr::writeable<var_fp<ISK_INTERVAL<float> > >(src)) return x;
			if (auto x = ptr::writeable<var_fp<float> >(src)) return x;
			if (auto x = ptr::writeable<var_fp<ISK_INTERVAL<double> > >(src)) return x;
			if (auto x = ptr::writeable<var_fp<double> >(src)) return x;
			if (auto x = ptr::writeable<var_fp<ISK_INTERVAL<long double> > >(src)) return x;
			if (auto x = ptr::writeable<var_fp<long double> >(src)) return x;
			if (auto x = ptr::writeable<var_fp<intmax_t> >(src)) return x;
			if (auto x = ptr::writeable<var_fp<uintmax_t> >(src)) return x;
			return std::nullopt;
		}

		std::optional<std::variant<const var_fp<float>*,
			const var_fp<ISK_INTERVAL<float> >*,
			const var_fp<double>*,
			const var_fp<ISK_INTERVAL<double> >*,
			const var_fp<long double>*,
			const var_fp<ISK_INTERVAL<long double> >*,
			const var_fp<intmax_t>*,
			const var_fp<uintmax_t>*
		> > const_primitive(const eval_to_ptr<fp_API>::eval_type& src) {
			if (auto x = dynamic_cast<const var_fp<ISK_INTERVAL<float> >*>(src.get_c())) return x;
			if (auto x = dynamic_cast<const var_fp<float>*>(src.get_c())) return x;
			if (auto x = dynamic_cast<const var_fp<ISK_INTERVAL<double> >*>(src.get_c())) return x;
			if (auto x = dynamic_cast<const var_fp<double>*>(src.get_c())) return x;
			if (auto x = dynamic_cast<const var_fp<ISK_INTERVAL<long double> >*>(src.get_c())) return x;
			if (auto x = dynamic_cast<const var_fp<long double>*>(src.get_c())) return x;
			if (auto x = dynamic_cast<const var_fp<intmax_t>*>(src.get_c())) return x;
			if (auto x = dynamic_cast<const var_fp<uintmax_t>* >(src.get_c())) return x;
			return std::nullopt;
		}
	}

	template<class T>
	void self_intersect(std::pair<T, T>& lhs, const std::pair<T, T>& rhs)	// prototype
	{
		if (lhs.first < rhs.first) lhs.first = rhs.first;
		if (lhs.second > rhs.second) lhs.second = rhs.second;
	}

	template<class T>
	std::optional<std::pair<T,T> > intersect(const std::pair<T, T>& lhs, const std::pair<T, T>& rhs)	// prototype
	{
		if (lhs.second < rhs.first) return std::nullopt;
		if (rhs.second < lhs.first) return std::nullopt;
		return std::pair(lhs.first <= rhs.first ? rhs.first : lhs.first, lhs.second <= rhs.second ? lhs.second : rhs.second);
	}

	// rearrange_sum support
	namespace _rearrange {
		struct sum {
			// base cases
			template<std::floating_point F> int operator()(F& lhs, F& rhs)
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

				if ((std::numeric_limits<F>::min_exponent == l_stat.exponent() || FP_SUBNORMAL == fp_type[0])
					&& (std::numeric_limits<F>::min_exponent == r_stat.exponent() || FP_SUBNORMAL == fp_type[1])
					&& !same_sign)
					goto resolve_exact_now;

				if (r_stat.exponent() > l_stat.exponent()) {	// doesn't work for different types
					swap(fp_type[0], fp_type[1]);
					swap(l_stat, r_stat);
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
						if ((std::numeric_limits<F>::digits < l_test) == (std::numeric_limits<F>::digits < r_test)) {
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
				if (delta_cancel(lhs, rhs, delta)) return 1;
				l_stat = lhs;
				r_stat = rhs;
				ret = 2;
				goto restart;
			}

			template<std::floating_point F> int operator()(ISK_INTERVAL<F>& lhs, ISK_INTERVAL<F>& rhs)
			{
				F working[4] = { lhs.lower(),lhs.upper(),rhs.lower(),rhs.upper() };

				// coordinate-wise rearrange_sum (failover, does not completely work)
				// legal values: -2...2
				const int l_code = operator()(working[0], working[2]);
				const int u_code = operator()(working[1], working[3]);
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
					auto safe_add_exponent = stats[2 + upper_parity].safe_add_exponents();
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

			// derived cases
			template<std::floating_point F, std::floating_point F2> requires(std::numeric_limits<F>::digits < std::numeric_limits<F2>::digits)
				int operator()(F& lhs, F2& rhs)
			{
				bool changed = false;

restart:
				int fp_type[2] = { fpclassify(lhs) , fpclassify(rhs) };
				assert(FP_NAN != fp_type[0]);
				assert(FP_NAN != fp_type[1]);
				assert(FP_INFINITE != fp_type[0]);
				assert(FP_INFINITE != fp_type[1]);
				if (FP_ZERO == fp_type[0]) return -1;	// should have intercepted earlier but we know what to do with these
				if (FP_ZERO == fp_type[1]) return 1;
				if (FP_SUBNORMAL == fp_type[0]) return 0;	// bail on mis-matched denormals
				if (FP_SUBNORMAL == fp_type[1]) return 0;

				bool l_negative = std::signbit(lhs);
				bool same_sign = (std::signbit(rhs) == l_negative);

				auto l_edit = edit_fp(lhs);
				auto r_edit = edit_fp(rhs);
				auto l_range = l_edit.safe_subtract_exponents();
				auto r_range = r_edit.safe_subtract_exponents();
				if (!same_sign) {
					if (auto consider = intersect(l_range, r_range)) {
						lhs -= l_edit.delta(consider->second);
						rhs -= r_edit.delta(consider->second);
						changed = true;
						if (0 == lhs) return 0 == rhs ? -2 : -1;
						if (0 == rhs) return 1;
						goto restart;
					}
				} else {
					auto l_consider = intersect(l_edit.safe_add_exponents(), r_range);
					auto r_consider = intersect(l_range, r_edit.safe_add_exponents());
					if (l_consider && r_consider) {
						if (l_edit.exponent() < r_edit.exponent()) l_consider = std::nullopt;
						else if (l_edit.exponent() > r_edit.exponent()) r_consider = std::nullopt;
					}
					if (l_consider) {
						lhs += l_edit.delta(l_consider->second);
						rhs -= r_edit.delta(l_consider->second);
						changed = true;
						if (0 == lhs) return 0 == rhs ? -2 : -1;
						if (0 == rhs) return 1;
						goto restart;
					}
					if (r_consider) {
						lhs -= l_edit.delta(r_consider->second);
						rhs += r_edit.delta(r_consider->second);
						changed = true;
						if (0 == lhs) return 0 == rhs ? -2 : -1;
						if (0 == rhs) return 1;
						goto restart;
					}
				}
				return changed ? 2 : 0;
			}

			template<std::floating_point F, std::floating_point F2> int operator()(F& lhs, ISK_INTERVAL<F2>& rhs)
			{
				throw std::logic_error("need to implement");
				return 0;
			}

			template<std::floating_point F, std::floating_point F2> requires(std::numeric_limits<F>::digits < std::numeric_limits<F2>::digits)
				int operator()(ISK_INTERVAL<F>& lhs, ISK_INTERVAL<F2>& rhs)
			{
				throw std::logic_error("need to implement");
				return 0;
			}

			template<std::floating_point F, std::floating_point F2> requires(std::numeric_limits<F>::digits > std::numeric_limits<F2>::digits)
				int operator()(F& lhs, F2& rhs)
			{
				int ret = operator()(rhs, lhs);
				switch (ret)
				{
				case 1: return -1;
				case -1: return 1;
				default: return ret;
				}
			}

			// CLang: sizeof(long double)==sizeof(double)
			template<std::floating_point F, std::floating_point F2> requires(std::is_same_v<F2, typename zaimoni::precise_demote<F>::type>)
				int operator()(F& lhs, F2& rhs)
			{
				return operator()(reinterpret_cast<F2&>(lhs), rhs);
			}

			template<std::floating_point F, std::floating_point F2> requires(std::is_same_v<F, typename zaimoni::precise_demote<F2>::type>)
				int operator()(F& lhs, F2& rhs)
			{
				return operator()(lhs, reinterpret_cast<F&>(rhs));
			}

			template<std::floating_point F, std::floating_point F2> int operator()(ISK_INTERVAL<F>& lhs, F2& rhs)
			{
				switch (int ret = operator()(rhs, lhs))
				{
				case 1: return -1;
				case -1: return 1;
				default: return ret;
				}
			}

			template<std::floating_point F, std::floating_point F2> requires(std::numeric_limits<F>::digits > std::numeric_limits<F2>::digits)
				int operator()(ISK_INTERVAL<F>& lhs, ISK_INTERVAL<F2>& rhs)
			{
				switch (int ret = operator()(rhs, lhs))
				{
				case 1: return -1;
				case -1: return 1;
				default: return ret;
				}
			}

			template<std::floating_point F, std::floating_point F2> requires(std::is_same_v<F2, typename zaimoni::precise_demote<F>::type>)
				int operator()(ISK_INTERVAL<F>& lhs, ISK_INTERVAL<F2>& rhs)
			{
				return operator()(reinterpret_cast<ISK_INTERVAL<F2>&>(lhs), rhs);
			}

			template<std::floating_point F, std::floating_point F2> requires(std::is_same_v<F, typename zaimoni::precise_demote<F2>::type>)
				int operator()(ISK_INTERVAL<F>& lhs, ISK_INTERVAL<F2>& rhs)
			{
				return operator()(lhs, reinterpret_cast<ISK_INTERVAL<F>&>(rhs));
			}

			template<class F, class F2> int operator()(var_fp<F>* lhs, var_fp<F2>* rhs) {
				return operator()(lhs->_x, rhs->_x);
			}
		};
	}

	namespace parse_for {
		// typed_clone destinations must be tested after anything that could clone to them
		std::optional<std::variant<var_fp<float>*,
			var_fp<ISK_INTERVAL<float> >*,
			var_fp<double>*,
			var_fp<ISK_INTERVAL<double> >*,
			var_fp<long double>*,
			var_fp<ISK_INTERVAL<long double> >*
		> > rearrange_sum(eval_to_ptr<fp_API>::eval_type& src) {
			if (auto x = ptr::writeable<var_fp<ISK_INTERVAL<float> > >(src)) return x;
			if (auto x = ptr::writeable<var_fp<float> >(src)) return x;
			if (auto x = ptr::writeable<var_fp<ISK_INTERVAL<double> > >(src)) return x;
			if (auto x = ptr::writeable<var_fp<double> >(src)) return x;
			if (auto x = ptr::writeable<var_fp<ISK_INTERVAL<long double> > >(src)) return x;
			if (auto x = ptr::writeable<var_fp<long double> >(src)) return x;
			return std::nullopt;
		}
	}

	namespace unhandled {
		std::optional<std::variant<const var_fp<intmax_t>*,
			const var_fp<uintmax_t>*
		> > rearrange_sum(const eval_to_ptr<fp_API>::eval_type& src) {
			auto test = src.get_c();
			if (auto x = dynamic_cast<const var_fp<intmax_t>*>(test)) return x;
			if (auto x = dynamic_cast<const var_fp<uintmax_t>*>(test)) return x;
			return std::nullopt;
		}
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

		auto l = parse_for::rearrange_sum(lhs);
		if (!l) {
			if (unhandled::rearrange_sum(lhs)) {
				if (unhandled::rearrange_sum(rhs)) throw std::logic_error("need to build out zaimoni::math::rearrange_sum");
				if (parse_for::rearrange_sum(rhs)) throw std::logic_error("need to build out zaimoni::math::rearrange_sum");
			}
			return 0;
		}
		if (auto r = parse_for::rearrange_sum(rhs)) return std::visit(_rearrange::sum(), *l, *r);
		if (unhandled::rearrange_sum(rhs)) throw std::logic_error("need to build out zaimoni::math::rearrange_sum");

		return 0;
	}

	namespace _rearrange {
		struct product {
			// base cases
			template<std::floating_point F> int operator()(F& lhs, F& rhs)
			{
				auto l_stat = edit_fp(lhs);
				auto r_stat = edit_fp(rhs);
				int predicted_exponent = l_stat.predict_product_exponent(r_stat);

				// 1.0*1.0 is 1.0
				if (l_stat.product_is_exact()) {
					if (l_stat.exponent_is_normal(predicted_exponent)) {
exact_product:
						lhs *= rhs;
						rhs = 1.0;
						return 1;
					}
					return l_stat.rebalance(r_stat) ? 2 : 0;
				}
				if (r_stat.product_is_exact()) {
					if (l_stat.exponent_is_normal(predicted_exponent)) goto exact_product;
					return r_stat.rebalance(l_stat) ? 2 : 0;
				}

				ISK_INTERVAL<F> predicted_mantissa(l_stat.mantissa()); // XXX \todo fix where long double better than double
				predicted_mantissa *= r_stat.mantissa();
				if (predicted_mantissa.lower() == predicted_mantissa.upper()) {
					if (0.5 <= predicted_mantissa.lower() || -0.5 >= predicted_mantissa.upper()) predicted_exponent++;
					if (l_stat.exponent_is_normal(predicted_exponent)) goto exact_product;
					// \todo adjusting the mantissas is still justifiable
				}

				// XXX want to see the mantissas as integers to decide which one to optimize
				if (l_stat.mantissa_as_int() < r_stat.mantissa_as_int()) return l_stat.rebalance(r_stat) ? 2 : 0;
				return r_stat.rebalance(l_stat) ? 2 : 0;
			}

			template<std::floating_point F> int operator()(ISK_INTERVAL<F>& lhs, ISK_INTERVAL<F>& rhs)
			{
				return 0;	// stub
			}

			// derived cases
			template<std::floating_point F, std::floating_point F2> requires(std::numeric_limits<F>::digits < std::numeric_limits<F2>::digits)
				int operator()(F& lhs, F2& rhs)
			{
				throw std::logic_error("need to implement");
				return 0;
			}

			template<std::floating_point F, std::floating_point F2> int operator()(F& lhs, ISK_INTERVAL<F2>& rhs)
			{
				throw std::logic_error("need to implement");
				return 0;
			}

			template<std::floating_point F, std::floating_point F2> requires(std::numeric_limits<F>::digits < std::numeric_limits<F2>::digits)
				int operator()(ISK_INTERVAL<F>& lhs, ISK_INTERVAL<F2>& rhs)
			{
				throw std::logic_error("need to implement");
				return 0;
			}

			template<std::floating_point F, std::floating_point F2> requires(std::numeric_limits<F>::digits > std::numeric_limits<F2>::digits)
				int operator()(F& lhs, F2& rhs)
			{
				int ret = operator()(rhs, lhs);
				switch (ret)
				{
				case 1: return -1;
				case -1: return 1;
				default: return ret;
				}
			}

			// CLang: sizeof(long double)==sizeof(double)
			template<std::floating_point F, std::floating_point F2> requires(std::is_same_v<F2, typename zaimoni::precise_demote<F>::type>)
				int operator()(F& lhs, F2& rhs)
			{
				return operator()(reinterpret_cast<F2&>(lhs), rhs);
			}

			template<std::floating_point F, std::floating_point F2> requires(std::is_same_v<F, typename zaimoni::precise_demote<F2>::type>)
				int operator()(F& lhs, F2& rhs)
			{
				return operator()(lhs, reinterpret_cast<F&>(rhs));
			}

			template<std::floating_point F, std::floating_point F2> int operator()(ISK_INTERVAL<F>& lhs, F2& rhs)
			{
				switch (int ret = operator()(rhs, lhs))
				{
				case 1: return -1;
				case -1: return 1;
				default: return ret;
				}
			}

			template<std::floating_point F, std::floating_point F2> requires(std::numeric_limits<F>::digits > std::numeric_limits<F2>::digits)
				int operator()(ISK_INTERVAL<F>& lhs, ISK_INTERVAL<F2>& rhs)
			{
				switch (int ret = operator()(rhs, lhs))
				{
				case 1: return -1;
				case -1: return 1;
				default: return ret;
				}
			}

			template<std::floating_point F, std::floating_point F2> requires(std::is_same_v<F2, typename zaimoni::precise_demote<F>::type>)
				int operator()(ISK_INTERVAL<F>& lhs, ISK_INTERVAL<F2>& rhs)
			{
				return operator()(reinterpret_cast<ISK_INTERVAL<F2>&>(lhs), rhs);
			}

			template<std::floating_point F, std::floating_point F2> requires(std::is_same_v<F, typename zaimoni::precise_demote<F2>::type>)
				int operator()(ISK_INTERVAL<F>& lhs, ISK_INTERVAL<F2>& rhs)
			{
				return operator()(lhs, reinterpret_cast<ISK_INTERVAL<F>&>(rhs));
			}

			template<class F, class F2> int operator()(var_fp<F>* lhs, var_fp<F2>* rhs) {
				return operator()(lhs->_x, rhs->_x);
			}
		};
	}

	namespace parse_for {
		std::optional<std::variant<const var_fp<float>*,
			const var_fp<ISK_INTERVAL<float> >*,
			const var_fp<double>*,
			const var_fp<ISK_INTERVAL<double> >*,
			const var_fp<long double>*,
			const var_fp<ISK_INTERVAL<long double> >*
		> > rearrange_product(const eval_to_ptr<fp_API>::eval_type& src) {
			auto test = src.get_c();
			if (auto x = dynamic_cast<const var_fp<float>*>(test)) return x;
			if (auto x = dynamic_cast<const var_fp<ISK_INTERVAL<float> >*>(test)) return x;
			if (auto x = dynamic_cast<const var_fp<double>*>(test)) return x;
			if (auto x = dynamic_cast<const var_fp<ISK_INTERVAL<double> >*>(test)) return x;
			if (auto x = dynamic_cast<const var_fp<long double>*>(test)) return x;
			if (auto x = dynamic_cast<const var_fp<ISK_INTERVAL<long double> >*>(test)) return x;
			return std::nullopt;
		}
	}

	namespace unhandled {
		std::optional<std::variant<const var_fp<intmax_t>*,
			const var_fp<uintmax_t>*
		> > rearrange_product(const eval_to_ptr<fp_API>::eval_type& src) {
			auto test = src.get_c();
			if (auto x = dynamic_cast<const var_fp<intmax_t>*>(test)) return x;
			if (auto x = dynamic_cast<const var_fp<uintmax_t>*>(test)) return x;
			return std::nullopt;
		}
	}

	// no-op implementation to enable building
	int rearrange_product(COW<fp_API>& lhs, COW<fp_API>& rhs) {
		if (auto l = ptr::writeable<API_product<fp_API> >(lhs)) {
			if (const int code = l->rearrange_product(rhs)) return code;
		}

		if (auto r = ptr::writeable<API_product<fp_API> >(rhs)) {
			switch (const int code = r->rearrange_product(lhs)) { // \todo not if product is non-commutative
			case -1: return 1; // left and right arguments are transposed here
			case 1: return -1;
			default: return code; // otherwise, pass through the code unchanged
			}
		}

		if (unhandled::rearrange_product(lhs) && unhandled::rearrange_product(rhs)) {
			auto err = std::string("need to build out zaimoni::math::rearrange_product: ") + std::visit(type_to_str(), *unhandled::rearrange_product(lhs)) + ", " + std::visit(type_to_str(), *unhandled::rearrange_product(rhs));
			throw new std::logic_error(err);
		}
		return 0;
	}

	namespace _eval {
		struct product {
			template<std::floating_point F> fp_API* operator()(const ISK_INTERVAL<F>& lhs, const ISK_INTERVAL<F>& rhs) {
				try {
					auto ret = lhs * rhs;
					if (ret.lower() == ret.upper()) return new var_fp<typename ISK_INTERVAL<F>::base_type>(ret.upper());
					return new var_fp<decltype(ret)>(ret);
				} catch (zaimoni::math::numeric_error& e) {
					return nullptr;
				}
				return nullptr;
			}

			template<std::floating_point F, std::floating_point F2> requires(std::numeric_limits<F>::max_exponent < std::numeric_limits<F2>::max_exponent)
				fp_API* operator()(const ISK_INTERVAL<F>& lhs, const ISK_INTERVAL<F2>& rhs)
			{
				return operator()(ISK_INTERVAL<F2>(lhs), rhs);
			}

			template<std::floating_point F, std::floating_point F2> requires(std::numeric_limits<F>::max_exponent > std::numeric_limits<F2>::max_exponent)
				fp_API* operator()(const ISK_INTERVAL<F>& lhs, const ISK_INTERVAL<F2>& rhs)
			{
				return operator()(lhs, ISK_INTERVAL<F>(rhs));
			}

			template<std::floating_point F, std::floating_point F2> requires(std::is_same_v<F2, typename zaimoni::precise_demote<F>::type>)
				fp_API* operator()(const ISK_INTERVAL<F>& lhs, const ISK_INTERVAL<F2>& rhs)
			{
				return operator()(reinterpret_cast<const ISK_INTERVAL<F2>&>(lhs), rhs);
			}

			template<std::floating_point F, std::floating_point F2> requires(std::is_same_v<F, typename zaimoni::precise_demote<F2>::type>)
				fp_API* operator()(const ISK_INTERVAL<F>& lhs, const ISK_INTERVAL<F2>& rhs)
			{
				return operator()(lhs, reinterpret_cast<const ISK_INTERVAL<F>&>(rhs));
			}

			template<std::floating_point F, std::floating_point F2>
			fp_API* operator()(const F& lhs, const ISK_INTERVAL<F2>& rhs)
			{
				return operator()(ISK_INTERVAL<F>(lhs), rhs);
			}

			template<std::floating_point F, std::floating_point F2>
			fp_API* operator()(const F& lhs, const F2& rhs)
			{
				return operator()(ISK_INTERVAL<F>(lhs), ISK_INTERVAL<F2>(rhs));
			}

			template<std::floating_point F, std::floating_point F2>
			fp_API* operator()(const ISK_INTERVAL<F>& lhs, const F2& rhs)
			{
				return operator()(lhs, ISK_INTERVAL<F2>(rhs));
			}

			template<class F, class F2>
			fp_API* operator()(const var_fp<F>* lhs, const var_fp<F2>* rhs)
			{
				return operator()(lhs->_x, rhs->_x);
			}
		};
	}

	namespace parse_for {
		// typed_clone destinations must be tested after anything that could clone to them
		std::optional<std::variant<const var_fp<float>*,
			const var_fp<ISK_INTERVAL<float> >*,
			const var_fp<double>*,
			const var_fp<ISK_INTERVAL<double> >*,
			const var_fp<long double>*,
			const var_fp<ISK_INTERVAL<long double> >*
		> > eval_product(const eval_to_ptr<fp_API>::eval_type& src) {
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

	namespace unhandled {
		std::optional<std::variant<const var_fp<intmax_t>*,
			const var_fp<uintmax_t>*
		> > eval_product(const eval_to_ptr<fp_API>::eval_type& src) {
			auto test = src.get_c();
			if (auto x = dynamic_cast<const var_fp<intmax_t>*>(test)) return x;
			if (auto x = dynamic_cast<const var_fp<uintmax_t>*>(test)) return x;
			return std::nullopt;
		}
	}

	COW<fp_API> eval_product(const COW<fp_API>& lhs, const COW<fp_API>& rhs)
	{	// we currently honor floating point types.  Integral types would also make sense here, mostly
		if (auto d2 = parse_for::eval_product(rhs)) {
			if (auto n2 = parse_for::eval_product(lhs)) return std::visit(_eval::product(), *n2, *d2);
			if (unhandled::eval_product(rhs)) throw std::logic_error("need to build out zaimoni::math::eval_product");
		}
		if (unhandled::eval_product(lhs)) {
			if (unhandled::eval_product(rhs)) throw std::logic_error("need to build out zaimoni::math::eval_product");
			if (parse_for::eval_product(rhs)) throw std::logic_error("need to build out zaimoni::math::eval_product");
		}
		return nullptr;
	}

	COW<fp_API> add_identity(const type& src)
	{
		COW<fp_API> ret;
		if (0 >= src.subclass(math::get<_type<_type_spec::_O_SHARP_>>())) { // Octonions.
			// should use additive identity for integers Z
			ret = new var_fp<intmax_t>(0);	// ideally would use a static cache to defer thrashing RAM
		};
		// if we were to support the surreal/hyperreal chain we would test for that here as well
		// and use additive identity for integers Z
		// otherwise, ask the type
		return ret;	// trigger NRVO
	}

	COW<fp_API> mult_identity(const type& src)
	{
		COW<fp_API> ret;
		if (0 >= src.subclass(math::get<_type<_type_spec::_O_SHARP_>>())) { // Octonions.
			// should use multiplicative identity for integers Z
			ret = new var_fp<intmax_t>(1);	// ideally would use a static cache to defer thrashing RAM
		};
		// if we were to support the surreal/hyperreal chain we would test for that here as well
		// and use multiplicative identity for integers Z
		// otherwise, ask the type
		return ret;	// trigger NRVO
	}

	// unlike sum scores, working product scores are nonlinear
	int product_score(const COW<fp_API>& lhs)
	{
		if (parse_for::eval_product(lhs)) return std::numeric_limits<int>::min() + 1;
		if (dynamic_cast<const API_product<fp_API>*>(lhs.get_c())) return std::numeric_limits<int>::min() + 1;
		return std::numeric_limits<int>::min();
	}

	static std::optional<std::pair<int, int> > op_count_product(const COW<fp_API>& lhs, const COW<fp_API>& rhs)
	{
		auto elementary_lhs = parse_for::eval_product(lhs);
		auto elementary_rhs = parse_for::eval_product(rhs);
		if (elementary_lhs && elementary_rhs) return std::pair(0, 1);
		auto API_lhs = elementary_lhs ? nullptr : dynamic_cast<const API_product<fp_API>*>(lhs.get_c());
		auto API_rhs = elementary_rhs ? nullptr : dynamic_cast<const API_product<fp_API>*>(rhs.get_c());
		if (API_lhs) {
			auto count = API_lhs->product_op_count(rhs);
			if (count) return count;
		}
		if (API_rhs) {
			auto count = API_rhs->product_op_count(lhs); // should be fine even if multiplication isn't commutative
			if (count) return count;
		}

		return std::nullopt;
	}

	int product_score(const COW<fp_API>& lhs, const COW<fp_API>& rhs)
	{
		if (auto score = op_count_product(lhs, rhs)) {
			// we know how to multiply these, and have some idea how expensive it is
			return std::numeric_limits<int>::max() - score->second - score->first;
		}

		if (std::numeric_limits<int>::min() < product_score(lhs)) {
			// \todo: try to check whether rearrange or eval actually would happen
			return product_score(rhs);
		}
		return std::numeric_limits<int>::min();
	}

	void update_op_count_product(const COW<fp_API>& lhs, const COW<fp_API>& rhs, std::pair<int, int>& accumulator)
	{
		if (auto stage = op_count_product(lhs, rhs)) {
			clamped_sum_assign(accumulator.first, stage->first);
			clamped_sum_assign(accumulator.second, stage->second);
		} else clamped_sum_assign(accumulator.second, 1);
	}

	// eval_quotient support
	namespace _eval {
		class quotient {
		private:
			template<std::floating_point F> ISK_INTERVAL<F> to_interval(const ISK_INTERVAL<F>& x) { return x; }
			template<std::floating_point F> ISK_INTERVAL<F> to_interval(const F& x) { return ISK_INTERVAL<F>(x); }
			template<std::floating_point F> ISK_INTERVAL<F> to_interval(const var_fp <ISK_INTERVAL<F> >* x) { return x->_x; }
			template<std::floating_point F> ISK_INTERVAL<F> to_interval(const var_fp<F>* x) { return ISK_INTERVAL<F>(x->_x); }

		public:
			template<std::floating_point F> fp_API* operator()(const ISK_INTERVAL<F>& n, const ISK_INTERVAL<F>& d) {
				try {
					auto ret = n / d;
					if (ret.lower() == ret.upper()) return new var_fp<decltype(ret.upper())>(ret.upper());
					return new var_fp<decltype(ret)>(ret);
				} catch (zaimoni::math::numeric_error& e) {
					return nullptr;
				}
			}

			template<std::floating_point F, std::floating_point F2> requires(std::numeric_limits<F>::max_exponent < std::numeric_limits<F2>::max_exponent)
				fp_API* operator()(const ISK_INTERVAL<F>& n, const ISK_INTERVAL<F2>& d)
			{
				return operator()(ISK_INTERVAL<F2>(n), d);
			}

			template<std::floating_point F, std::floating_point F2> requires (std::numeric_limits<F>::max_exponent > std::numeric_limits<F2>::max_exponent)
				fp_API* operator()(const ISK_INTERVAL<F>& n, const ISK_INTERVAL<F2>& d)
			{
				return operator()(n, ISK_INTERVAL<F>(d));
			}

			template<std::floating_point F, std::floating_point F2> requires(std::is_same_v<F2, typename zaimoni::precise_demote<F>::type>)
				fp_API* operator()(const ISK_INTERVAL<F>& n, const ISK_INTERVAL<F2>& d)
			{
				return operator()(reinterpret_cast<const ISK_INTERVAL<F2>&>(n), d);
			}

			template<std::floating_point F, std::floating_point F2> requires(std::is_same_v<F, typename zaimoni::precise_demote<F2>::type>)
				fp_API* operator()(const ISK_INTERVAL<F>& n, const ISK_INTERVAL<F2>& d)
			{
				return operator()(n, reinterpret_cast<const ISK_INTERVAL<F>&>(d));
			}

			template<class T, class T2> fp_API* operator()(T n, T2 d) {
				return operator()(to_interval(n), to_interval(d));
			}
		};
	} // namespace _eval

	namespace reject {
		struct divsion_by_zero {
			template<std::floating_point F> void operator()(const ISK_INTERVAL<F>& x) {
				if (x == F(0)) throw zaimoni::math::numeric_error("division by zero");	// expected to be caught earlier when called from the quotient class
				if (F(0) > x.lower() && F(0) < x.upper()) throw zaimoni::math::numeric_error("division should result in two disjoint intervals");	// not always, but requires exact zero numerator which should be caught by the quotient class
			}
			template<std::floating_point F> void operator()(const F& x) {
				if (x == F(0)) throw zaimoni::math::numeric_error("division by zero");	// expected to be caught earlier when called from the quotient class
			}

			template<class F> void operator()(const var_fp<F>* x) {
				operator()(x->_x);
			}
		};
	}

	namespace parse_for {
		// typed_clone destinations must be tested after anything that could clone to them
		std::optional<std::variant<const var_fp<float>*,
			const var_fp<ISK_INTERVAL<float> >*,
			const var_fp<double>*,
			const var_fp<ISK_INTERVAL<double> >*,
			const var_fp<long double>*,
			const var_fp<ISK_INTERVAL<long double> >*
		> > eval_quotient(const eval_to_ptr<fp_API>::eval_type& src) {
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

	namespace unhandled {
		std::optional<std::variant<const var_fp<intmax_t>*,
			const var_fp<uintmax_t>*
		> > eval_quotient(const eval_to_ptr<fp_API>::eval_type& src) {
			auto test = src.get_c();
			if (auto x = dynamic_cast<const var_fp<intmax_t>*>(test)) return x;
			if (auto x = dynamic_cast<const var_fp<uintmax_t>*>(test)) return x;
			return std::nullopt;
		}
	}

	fp_API* eval_quotient(const COW<fp_API>& n, const COW<fp_API>& d)
	{	// we currently honor floating point types.  Integral types would also make sense here, mostly
		static const std::string err_prefix("need to build out zaimoni::math::eval_quotient: ");

		if (auto d2 = parse_for::eval_quotient(d)) {
			std::visit(reject::divsion_by_zero(), *d2);
			if (auto n2 = parse_for::eval_quotient(n)) return std::visit(_eval::quotient(), *n2, *d2);
			if (auto nfail = unhandled::eval_quotient(n)) {
				auto n3 = std::visit(to_float(), *nfail).value();
				return std::visit(_eval::quotient(), n3, *d2);
			}
		}
		if (auto dfail = unhandled::eval_quotient(d)) {
			auto d3 = std::visit(to_float(), *dfail).value();
			if (auto n2 = parse_for::eval_quotient(n)) return std::visit(_eval::quotient(), *n2, d3);
			if (auto nfail = unhandled::eval_quotient(n)) {
				auto n3 = std::visit(to_float(), *nfail).value();
				return std::visit(_eval::quotient(), n3, d3);
			}
		}
		return nullptr;
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

	// generally speaking, for floating point numerals we want to destructively add the smallest two exponents first.
	// * minimizes the numerical error which is controlled by the size of the larger absolute-value numeral
	// * may enable further rearrangement
	// so the score should be largest for denormals and smallest near infinity
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
		if (auto l = dynamic_cast<const API_sum<fp_API>*>(lhs.get_c())) return std::numeric_limits<int>::min()+1;
		if (auto test = parse_for::sum_score(lhs)) return std::visit(score::sum(), *test);
		return std::numeric_limits<int>::min();
	}

	int sum_score(const COW<fp_API>& lhs, const COW<fp_API>& rhs)
	{
		const auto l_sum = dynamic_cast<const API_sum<fp_API>*>(lhs.get_c());
		if (l_sum) {
			if (auto score = l_sum->score_sum(rhs); std::numeric_limits<int>::min() < score) return score;
		}
		const auto r_sum = dynamic_cast<const API_sum<fp_API>*>(rhs.get_c());
		if (r_sum) {
			if (auto score = r_sum->score_sum(lhs); std::numeric_limits<int>::min() < score) return score;
		}
		if (l_sum || r_sum) return std::numeric_limits<int>::min();	// do not need to directly handle base cases

		if (const int l_score = sum_score(lhs); std::numeric_limits<int>::min() < l_score) {
			const int r_score = sum_score(rhs);
			return l_score < r_score ? l_score : r_score;
		}
		return std::numeric_limits<int>::min();
	}

	namespace _eval {
		struct sum {

			template<std::floating_point F> fp_API* operator()(const ISK_INTERVAL<F>& lhs, const ISK_INTERVAL<F>& rhs)
			{
				try {
					auto ret = lhs + rhs;
					if (ret.lower() == ret.upper()) return new var_fp<typename ISK_INTERVAL<F>::base_type>(ret.upper());
					return new var_fp<decltype(ret)>(ret);
				} catch (zaimoni::math::numeric_error& e) {
					return nullptr;
				}
				return nullptr;
			}

			template<std::floating_point F, std::floating_point F2> requires(std::numeric_limits<F>::max_exponent < std::numeric_limits<F2>::max_exponent)
				fp_API* operator()(const ISK_INTERVAL<F>& lhs, const ISK_INTERVAL<F2>& rhs)
			{
				return operator()(ISK_INTERVAL<F2>(lhs), rhs);
			}

			template<std::floating_point F, std::floating_point F2> requires(std::numeric_limits<F>::max_exponent > std::numeric_limits<F2>::max_exponent)
				fp_API* operator()(const ISK_INTERVAL<F>& lhs, const ISK_INTERVAL<F2>& rhs)
			{
				return operator()(lhs, ISK_INTERVAL<F>(rhs));
			}

			template<std::floating_point F, std::floating_point F2> requires(std::is_same_v<F2, typename zaimoni::precise_demote<F>::type>)
				fp_API* operator()(const ISK_INTERVAL<F>& lhs, const ISK_INTERVAL<F2>& rhs)
			{
				return operator()(reinterpret_cast<const ISK_INTERVAL<F2>&>(lhs), rhs);
			}

			template<std::floating_point F, std::floating_point F2> requires(std::is_same_v<F, typename zaimoni::precise_demote<F2>::type>)
				fp_API* operator()(const ISK_INTERVAL<F>& lhs, const ISK_INTERVAL<F2>& rhs)
			{
				return operator()(lhs, reinterpret_cast<const ISK_INTERVAL<F>&>(rhs));
			}

			template<std::floating_point F, std::floating_point F2>
			fp_API* operator()(const F& lhs, const ISK_INTERVAL<F2>& rhs)
			{
				return operator()(ISK_INTERVAL<F>(lhs), rhs);
			}

			template<std::floating_point F, std::floating_point F2>
			fp_API* operator()(const F& lhs, const F2& rhs)
			{
				return operator()(ISK_INTERVAL<F>(lhs), ISK_INTERVAL<F2>(rhs));
			}

			template<std::floating_point F, std::floating_point F2>
			fp_API* operator()(const ISK_INTERVAL<F>& lhs, const F2& rhs)
			{
				return operator()(lhs, ISK_INTERVAL<F2>(rhs));
			}

			template<class F, class F2>
			fp_API* operator()(const var_fp<F>* lhs, const var_fp<F2>* rhs)
			{
				return operator()(lhs->_x, rhs->_x);
			}

		};
	}

	namespace parse_for {
		std::optional<std::variant<const var_fp<float>*,
			const var_fp<ISK_INTERVAL<float> >*,
			const var_fp<double>*,
			const var_fp<ISK_INTERVAL<double> >*,
			const var_fp<long double>*,
			const var_fp<ISK_INTERVAL<long double> >*
		> > eval_sum(const eval_to_ptr<fp_API>::eval_type& src) {
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

	namespace unhandled {
		std::optional<std::variant<const var_fp<intmax_t>*,
			const var_fp<uintmax_t>*
		> > eval_sum(const eval_to_ptr<fp_API>::eval_type& src) {
			auto test = src.get_c();
			if (auto x = dynamic_cast<const var_fp<intmax_t>*>(test)) return x;
			if (auto x = dynamic_cast<const var_fp<uintmax_t>*>(test)) return x;
			return std::nullopt;
		}
	}

	COW<fp_API> eval_sum(const COW<fp_API>& lhs, const COW<fp_API>& rhs)
	{
		const auto l_sum = dynamic_cast<const API_sum<fp_API>*>(lhs.get_c());
		if (l_sum) {
			if (auto result = l_sum->eval_sum(rhs)) return result;
		}
		const auto r_sum = dynamic_cast<const API_sum<fp_API>*>(rhs.get_c());
		if (r_sum) {
			if (auto result = r_sum->eval_sum(lhs)) return result;
		}
		if (l_sum || r_sum) return nullptr;	// do not need to directly handle base cases

		if (auto l = parse_for::eval_sum(lhs)) {
			if (auto r = parse_for::eval_sum(rhs)) {
				return std::visit(_eval::sum(), *l, *r);
				if (unhandled::eval_sum(rhs)) throw std::logic_error("need to build out zaimoni::math::eval_sum");
			}
		}
		if (unhandled::eval_sum(lhs)) {
			if (parse_for::eval_sum(rhs)) throw std::logic_error("need to build out zaimoni::math::eval_sum");
			if (unhandled::eval_sum(rhs)) throw std::logic_error("need to build out zaimoni::math::eval_sum");
		}
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

eval_to_ptr<fp_API>::eval_type& operator*=(eval_to_ptr<fp_API>::eval_type& lhs, const eval_to_ptr<fp_API>::eval_type& rhs)
{
	if (auto r = lhs.get_rw<product>()) {
		if (!r->first) lhs = std::unique_ptr<fp_API>(r->first = r->second->typed_clone());
		r->first->append_term(rhs);
	} else {
		lhs = lhs * rhs;
	}

	return lhs;
}

eval_to_ptr<fp_API>::eval_type operator/(const eval_to_ptr<fp_API>::eval_type& lhs, const eval_to_ptr<fp_API>::eval_type& rhs)
{
	if (lhs->is_one()) {
		auto stage = std::unique_ptr<symbolic_fp>(new symbolic_fp(rhs));
		stage->self_multinv();
		return stage.release();
	}
	return new quotient(lhs, rhs);
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
