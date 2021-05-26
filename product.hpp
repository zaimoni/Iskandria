#ifndef PRODUCT_HPP
#define PRODUCT_HPP 1

#include "n_ary.hpp"
#include "arithmetic.hpp"

namespace zaimoni {

class product final : public fp_API, public eval_shared_ptr<fp_API>, protected n_ary_op<fp_API>
{
	enum { strict_max_heuristic = _n_ary_op::strict_max_core_heuristic };

public:
	using smart_ptr = n_ary_op<fp_API>::smart_ptr;
	using eval_spec = n_ary_op<fp_API>::eval_spec;

	product() = default;
	product(const product& src) = default;
	product(product&& src) = default;
	~product() = default;
	product& operator=(const product& src) = default;
	product& operator=(product&& src) = default;

	void append_term(const smart_ptr& src);
	void append_term(smart_ptr&& src);
	void append_term(fp_API* src) { append_term(smart_ptr(src)); }

private:
	bool would_fpAPI_eval() const override;

public:
	smart_ptr destructive_eval() override; // eval_shared_ptr
	
	// fp_API
	bool self_eval() override;
	bool is_zero() const override;
	bool is_one() const override;
	int sgn() const override;
	bool is_scal_bn_identity() const override { return is_zero(); }	// let evaluation handle this -- pathological behavior anyway
	std::pair<intmax_t, intmax_t> scal_bn_safe_range() const override;
	intmax_t scal_bn_is_safe(intmax_t scale) const override;
	intmax_t ideal_scal_bn() const override;
	const math::type* domain() const override;
	fp_API* clone() const override { return new product(*this); }
	std::string to_s() const override;
	int precedence() const override { return 2; }
	bool _is_inf() const override;
	bool _is_finite() const override;

private:
	void _scal_bn(intmax_t scale) override;
	fp_API* _eval() const override { return nullptr; }	// placeholder
};

} // namespace zaimoni

#endif
