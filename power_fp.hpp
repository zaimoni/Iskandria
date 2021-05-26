#ifndef POWER_FP_HPP
#define POWER_FP_HPP 1

#include "Zaimoni.STL/eval.hpp"

namespace zaimoni {

	// hard-coded to * for now
	class power_fp final : public fp_API, public eval_shared_ptr<fp_API> {
		std::shared_ptr<fp_API> base;
		std::shared_ptr<fp_API> exponent;
		//	_type_spec::canonical_functions op;

	public:
		//	template<_type_spec::canonical_functions _op> // doesn't work -- uncallable?
		power_fp(std::shared_ptr<fp_API> x, std::shared_ptr<fp_API> y) noexcept : base(x), exponent(y) {}

		power_fp(const power_fp& src) = default;
		power_fp(power_fp&& src) = default;
		~power_fp() = default;
		power_fp& operator=(const power_fp& src) = default;
		power_fp& operator=(power_fp&& src) = default;

		bool self_square();

		const math::type* domain() const override;
		bool self_eval() override;
		bool is_zero() const override;
		bool is_one() const override;
		int sgn() const override;

		bool is_scal_bn_identity() const override { return is_scal_bn_identity_default(); }

		intmax_t scal_bn_is_safe(intmax_t scale) const override {
			return 0;	 // \todo replace null implementation
		}

		intmax_t ideal_scal_bn() const override {
			return 0;	 // \todo replace null implementation
		}

		fp_API* clone() const override {
			return new power_fp(*this);
		}

		std::string to_s() const override;

		// coordinate with the sum/product types
		int precedence() const override {
			return 3; // for multiplication
		}

		void _scal_bn(intmax_t scale) override;
		std::shared_ptr<fp_API> destructive_eval() override;
		fp_API* _eval() const override;
	};

}

#endif
