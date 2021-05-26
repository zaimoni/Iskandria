#ifndef SYMBOLIC_FP_HPP
#define SYMBOLIC_FP_HPP 1

#include "Zaimoni.STL/eval.hpp"
#include "Zaimoni.STL/Logging.h"
#include <optional>

namespace zaimoni {

	// the following operations on fp_API types are closely related
	// additive inverse
	// multiplicative inverse
	// scal_bn (power of 2 manipulation)

	class symbolic_fp final : public fp_API, public eval_shared_ptr<fp_API> {
		std::shared_ptr<fp_API> dest;
		intmax_t scale_by;
		uintmax_t bitmap;	// anything smaller incurs padding bytes anyway

	public:
		enum class op {
			inverse_add = 0,
			inverse_mult,
		};

		explicit symbolic_fp(const std::shared_ptr<fp_API>& src) noexcept : dest(src), scale_by(0), bitmap(0) {
			assert(src);
		}
		symbolic_fp(const std::shared_ptr<fp_API>& src, intmax_t scale_by) noexcept : dest(src), scale_by(scale_by), bitmap(0) {
			// this doesn't trigger self-evaluation, unlike the scal_bn call
			assert(src);
		}
		symbolic_fp(const symbolic_fp& src) = default;
		symbolic_fp(symbolic_fp&& src) = default;
		~symbolic_fp() = default;
		symbolic_fp& operator=(const symbolic_fp& src) = default;
		symbolic_fp& operator=(symbolic_fp&& src) = default;

		bool add_inverted() const { return bitmap & (1ULL << (int)op::inverse_add); }
		bool mult_inverted() const { return bitmap & (1ULL << (int)op::inverse_mult); }

		void self_negate();
		bool self_square();
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
		void _scal_bn(intmax_t scale) override;
		std::shared_ptr<fp_API> destructive_eval() override;

		fp_API* _eval() const override { return nullptr; }

		bool _is_inf() const override;
		bool _is_finite() const override;
	};

}	// namespace zaimoni

#endif
