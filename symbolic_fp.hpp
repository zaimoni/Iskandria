#ifndef SYMBOLIC_FP_HPP
#define SYMBOLIC_FP_HPP 1

#include "Zaimoni.STL/eval.hpp"
#include "Zaimoni.STL/Logging.h"
#include <optional>
#include <any>

namespace zaimoni {

	// the following operations on fp_API types are closely related
	// additive inverse
	// multiplicative inverse
	// scal_bn (power of 2 manipulation)

	class symbolic_fp final : public fp_API, public eval_to_ptr<fp_API>, public API_sum<fp_API>, public API_addinv {
		eval_type dest;
		intmax_t scale_by;
		uintmax_t bitmap;	// anything smaller incurs padding bytes anyway

	public:
		enum class op {
			inverse_add = 0,
			inverse_mult,
		};

		explicit symbolic_fp(const decltype(dest)& src) noexcept;
		symbolic_fp(decltype(dest) && src) noexcept;
		// this doesn't trigger self-evaluation, unlike the scal_bn call
		symbolic_fp(const decltype(dest)& src, intmax_t scale_by) noexcept;
		symbolic_fp(decltype(dest) && src, intmax_t scale_by) noexcept;

		symbolic_fp(const symbolic_fp& src) = default;
		symbolic_fp(symbolic_fp&& src) = default;
		~symbolic_fp() = default;
		symbolic_fp& operator=(const symbolic_fp& src) = default;
		symbolic_fp& operator=(symbolic_fp&& src) = default;

		bool add_inverted() const { return bitmap & (1ULL << (int)op::inverse_add); }
		bool mult_inverted() const { return bitmap & (1ULL << (int)op::inverse_mult); }

		void self_multinv();
		bool self_square();

		// eval_to_ptr<fp_API>
		eval_type destructive_eval() override;
		bool algebraic_self_eval() override;
		bool inexact_self_eval() override;

		// API_sum<fp_API>
		int rearrange_sum(eval_to_ptr<fp_API>::eval_type& rhs) override;
		fp_API* eval_sum(const typename eval_to_ptr<fp_API>::eval_type& rhs) const override { return nullptr; } // stub?
		int score_sum(const typename eval_to_ptr<fp_API>::eval_type& rhs) const override;

		// API_addinv
		void self_negate() override;

		// fp_API
		const math::type* domain() const override;
		bool self_eval() override;
		bool is_zero() const override;
		bool is_one() const override;
		int sgn() const override;

		bool is_scal_bn_identity() const override { return is_scal_bn_identity_default(); }

	private:
		std::optional<intmax_t> _scal_bn_is_unsafe(intmax_t scale) const;
		intmax_t _probe_dest(intmax_t scale) const;

	public:
		intmax_t scal_bn_is_safe(intmax_t scale) const override;
		intmax_t ideal_scal_bn() const override;
		fp_API* clone() const override;
		std::string to_s() const override;
		int precedence() const override;

	private:
		int would_rearrange_sum(const typename eval_to_ptr<fp_API>::eval_type& rhs) const;
		fp_API* scale_factor() const;
		static std::any multinv_sum_ok(const typename eval_to_ptr<fp_API>::eval_type& x);
		static bool would_eval_multinv_sum(const std::any& lhs, const std::any& rhs);
		static fp_API* eval_multinv_sum(const typename eval_to_ptr<fp_API>::eval_type& lhs, const typename eval_to_ptr<fp_API>::eval_type& rhs);
		static void global_init();

		void _scal_bn(intmax_t scale) override;
		fp_API* _eval() const override;
		std::optional<bool> _is_finite() const override;
		std::partial_ordering _value_compare(const fp_API* rhs) const override;
	};

}	// namespace zaimoni

#endif
