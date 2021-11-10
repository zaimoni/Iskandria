#ifndef COMPLEX_HPP
#define COMPLEX_HPP 1

#include "Zaimoni.STL/eval.hpp"

namespace zaimoni {
namespace math {

// Cartesian coordinate representation.
class complex final : public fp_API, public eval_to_ptr<fp_API>, API_sum<fp_API>, API_addinv, API_product<fp_API>, API_productinv<fp_API> {
	eval_type a;
	eval_type b;

public:
	complex(const decltype(a)& re, const decltype(b)& im);
	complex(const complex& src) = default;
	complex(complex&& src) = default;
	~complex() = default;
	complex& operator=(const complex& src) = default;
	complex& operator=(complex&& src) = default;

	friend auto Re(const complex& z) { return z.a; }
	friend auto Im(const complex& z) { return z.b; }
	friend eval_type Conj(const complex& z);
	friend eval_type norm2(const complex& z);

	// eval_to_ptr<fp_API>
	eval_type destructive_eval() override;
	bool algebraic_self_eval() override;
	bool inexact_self_eval() override;

	// fp_API
	bool self_eval() override;

	const math::type* domain() const override {
		if (is_finite()) return &get<_type<_type_spec::_C_> >();
		return &get<_type<_type_spec::_C_SHARP_> >();
	}

	bool is_zero() const override { return a->is_zero() && b->is_zero(); }
	bool is_one() const override { return a->is_one() && b->is_zero(); }
	int sgn() const override { return is_zero() ? 0 : 1; }
	bool is_scal_bn_identity() const override { return is_scal_bn_identity_default(); }
	intmax_t scal_bn_is_safe(intmax_t scale) const override;
	intmax_t ideal_scal_bn() const override;
	fp_API* clone() const override {
		if (b->is_zero()) return a->clone();
		return new complex(*this);
	}
	std::string to_s() const override;
	int precedence() const override { return std::numeric_limits<int>::max(); }	// numerals outrank all operators
	int precedence_to_s() const override { return _type_spec::Addition; }

	int rearrange_sum(eval_type& rhs) override;
	fp_API* eval_sum(const eval_to_ptr<fp_API>::eval_type& rhs) const override;
	int score_sum(const eval_to_ptr<fp_API>::eval_type& rhs) const override;
	void self_negate() override;
	int rearrange_product(eval_to_ptr<fp_API>::eval_type& rhs) override { return 0; } // stub
	fp_API* eval_product(const typename eval_to_ptr<fp_API>::eval_type& rhs) const override;
	std::optional<std::pair<int, int> > product_op_count(const typename eval_to_ptr<fp_API>::eval_type& rhs) const override;
	int rearrange_divides(eval_to_ptr<fp_API>::eval_type& lhs) override;
	int rearrange_dividedby(eval_to_ptr<fp_API>::eval_type& rhs) override;
	fp_API* eval_divides(const typename eval_to_ptr<fp_API>::eval_type& lhs) const override { return nullptr; } // stub
	fp_API* eval_dividedby(const typename eval_to_ptr<fp_API>::eval_type& rhs) const override { return nullptr; } // stub

private:
	void _scal_bn(intmax_t scale) override;
	fp_API* _eval() const override { return nullptr; }
	std::optional<bool> _is_finite() const override;
};

} // namespace math
} // namespace zaimoni

#endif
