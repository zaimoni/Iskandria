#ifndef COMPLEX_HPP
#define COMPLEX_HPP 1

#include "Zaimoni.STL/eval.hpp"

namespace zaimoni {
namespace math {

// Cartesian coordinate representation.
class complex : public fp_API, public eval_to_ptr<fp_API> {
	eval_type a;
	eval_type b;
	mutable unsigned char heuristics;

public:
	complex(const decltype(a)& re, const decltype(b)& im);
	complex(const complex& src) = default;
	complex(complex&& src) = default;
	~complex() = default;
	complex& operator=(const complex& src) = default;
	complex& operator=(complex&& src) = default;

	bool self_eval() override;

	friend auto Re(const complex& z) { return z.a; }
	friend auto Im(const complex& z) { return z.b; }
	friend eval_type Conj(const complex& z);
	friend eval_type norm2(const complex& z);

	const math::type* domain() const override {
		if (_is_finite()) return &get<_type<_type_spec::_C_> >();
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

	eval_type destructive_eval() override {
		if (b->is_zero()) return a;
		return nullptr;
	}

private:
	void _scal_bn(intmax_t scale) override;
	fp_API* _eval() const override { return nullptr; }
	bool _is_inf() const override { return a->is_inf() || b->is_inf(); }
	bool _is_finite() const override { return a->is_finite() && b->is_finite(); }
};

} // namespace math
} // namespace zaimoni

#endif
