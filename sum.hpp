#ifndef SUM_HPP
#define SUM_HPP 1

#include "n_ary.hpp"
#include "arithmetic.hpp"
#include <any>

namespace zaimoni {

class sum final : public fp_API, public eval_to_ptr<fp_API>, protected n_ary_op<fp_API>
{
public:
	using rule_guard = std::any (*)(const smart_ptr&);
	using would_eval = bool (*)(const std::any&, const std::any&);
	using rule_eval = fp_API* (*)(const smart_ptr&, const smart_ptr&);
	using eval_rule_spec = std::pair<std::pair<rule_guard, rule_guard>, std::pair<would_eval, rule_eval> >;

private:
	enum { strict_max_heuristic = _n_ary_op::strict_max_core_heuristic };

	static std::vector<rule_guard> eval_ok;
	static std::vector<eval_rule_spec> eval_algebraic_rules;
	static std::vector<eval_rule_spec> eval_inexact_rules;

public:
	sum() = default;
	sum(const sum& src) = default;
	sum(sum&& src) = default;
	~sum() = default;
	sum& operator=(const sum& src) = default;
	sum& operator=(sum&& src) = default;

	static void eval_algebraic_rule(const eval_rule_spec& src);
	static void eval_inexact_rule(const eval_rule_spec& src);

private:
	bool _append_infinity(const smart_ptr& src);
	void _append(smart_ptr&& src);

public:
	void append_term(const smart_ptr& src);
	void append_term(smart_ptr&& src);
	void append_term(fp_API* src) { append_term(smart_ptr(src)); }

private:
	bool would_fpAPI_eval() const override;

public:
	// eval_to_ptr<fp_API>
	eval_type destructive_eval() override;
	bool algebraic_self_eval() override;
	bool inexact_self_eval() override { return self_eval(); } // stub

	// fp_API
	bool self_eval() override;
	bool is_zero() const override;
	bool is_one() const override;
	int sgn() const override;

	bool is_scal_bn_identity() const override { return is_zero(); };	// let evaluation handle this, mostly
	intmax_t scal_bn_is_safe(intmax_t scale) const override;
	intmax_t ideal_scal_bn() const override;
	const math::type* domain() const override;
	fp_API* clone() const override { return new sum(*this); }
	sum* typed_clone() const { return new sum(*this); }
	std::string to_s() const override;
	int precedence() const override { return _precedence; }

private:
	static constexpr const auto _precedence = _type_spec::Addition;
	void _scal_bn(intmax_t scale) override;
	fp_API* _eval() const override { return nullptr; }	// placeholder
	std::optional<bool> _is_finite() const override;
};

} // namespace zaimoni

#endif
