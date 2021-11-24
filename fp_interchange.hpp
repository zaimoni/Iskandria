#ifndef FP_INTERCHANGE_HPP
#define FP_INTERCHANGE_HPP 1

#include "Zaimoni.STL/interval.hpp"
#include <optional>
#include <variant>

namespace zaimoni {

class fp_interchange
{
	uintmax_t mantissa_as_int;
	int fp_exp;
	int mantissa_bits;
	bool negative;

public:
	template<std::floating_point F>
	explicit fp_interchange(const F& src) : negative(signbit(src))
	{
		frexp(src, &fp_exp);
		mantissa_as_int = _mantissa_as_int(src);
		mantissa_bits = mantissa_bitcount(src);
	};

	explicit fp_interchange(const uintmax_t& src) : negative(false)
	{
		if (0 == src) canonical_zero();
		else {
			mantissa_as_int = _mantissa_as_int(src);
			mantissa_bits = INT_LOG2(mantissa_as_int) + 1;
			fp_exp = mantissa_bits + INT_LOG2(src / mantissa_as_int);
		}
	}

	explicit fp_interchange(const intmax_t& src) : negative(0 > src)
	{
		if (0 == src) canonical_zero();
		else {
			mantissa_as_int = _mantissa_as_int(src);
			mantissa_bits = INT_LOG2(mantissa_as_int) + 1;
			fp_exp = mantissa_bits + INT_LOG2(src / mantissa_as_int);
		}
	}

	std::optional<std::variant<intmax_t, uintmax_t> > to_int() {
		if (0 == mantissa_as_int) return 0LL;
		uintmax_t ret = mantissa_as_int;
		if (fp_exp > mantissa_bits) ret <<= (fp_exp - mantissa_bits);
		if (negative) {
			if (std::numeric_limits<intmax_t>::max() >= ret) return -((intmax_t)ret);
		}
		else {
			if (sizeof(uintmax_t) * CHAR_BIT >= fp_exp) {
				if (std::numeric_limits<intmax_t>::max() >= ret) return (intmax_t)ret;
				return ret;
			}
		}

		return std::nullopt;
	}

	std::optional<std::variant<float,
		double, ISK_INTERVAL<double>,
		long double, ISK_INTERVAL<long double> > > to_float() {
		if (0 == mantissa_as_int) return 0.0f;
		if (std::numeric_limits<long double>::max_exponent < fp_exp
			|| std::numeric_limits<long double>::min_exponent > fp_exp)
			return std::nullopt;
		if (std::numeric_limits<float>::digits >= mantissa_bits
			&& std::numeric_limits<float>::max_exponent >= fp_exp
			&& std::numeric_limits<float>::min_exponent <= fp_exp)
		{
			if (const auto test = as_fp<float>()) return *test;
		}
		if (std::numeric_limits<double>::max_exponent >= fp_exp
			&& std::numeric_limits<double>::min_exponent <= fp_exp)
		{
			if (const auto test = as_fp<double>()) return *test;
			if constexpr (std::numeric_limits<long double>::digits <= std::numeric_limits<double>::digits) return as_interval<double>();
		}
		if constexpr (std::numeric_limits<long double>::digits > std::numeric_limits<double>::digits) {
			if (const auto test = as_fp<long double>()) return *test;
			return as_interval<long double>();
		}
		return std::nullopt;
	}

private:
	void canonical_zero() {
		mantissa_as_int = 0;
		mantissa_bits = 1;
		fp_exp = INT_MIN;
	}

	template<std::floating_point F> F _as_fp(uintmax_t src, int bits) {
		F ret = src;
		if (negative) ret = -ret;
		return std::scalbn(ret, fp_exp - bits);
	}

	template<std::floating_point F> std::optional<F> as_fp() {
		if (std::numeric_limits<F>::digits < mantissa_bits) return std::nullopt;
		return _as_fp<F>(mantissa_as_int, mantissa_bits);
	}

	template<std::floating_point F> ISK_INTERVAL<F> as_interval() {
		decltype(mantissa_as_int) lb = mantissa_as_int;
		decltype(mantissa_as_int) ub = mantissa_as_int;
		decltype(mantissa_bits) bits = mantissa_bits;
		// \todo: check assembly here
		while (std::numeric_limits<F>::digits < bits) {
			auto ub_ceiling = ub % 2;
			lb /= 2;
			ub /= 2;
			if (ub_ceiling) ++ub;
			--bits;
		}
		if (negative) return ISK_INTERVAL(_as_fp<F>(ub, bits), _as_fp<F>(lb, bits));
		return ISK_INTERVAL(_as_fp<F>(lb, bits), _as_fp<F>(ub, bits));
	}
};

}	// namespace zaimoni

#endif
