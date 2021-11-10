#include "power_fp.hpp"
#include "arithmetic.hpp"
#include "Zaimoni.STL/var.hpp"

namespace zaimoni {

enum {
	inexact_eval = 1,
	algebraic_eval,
	eval_strict_ub,
};
static_assert(SCHAR_MAX >= eval_strict_ub * eval_strict_ub);

enum {
	reset_eval = algebraic_eval * eval_strict_ub + algebraic_eval
};

power_fp::power_fp(const decltype(base)& x, const decltype(exponent)& y) noexcept
	: base(x), exponent(y), heuristic(reset_eval) {
	would_destructive_eval();
}

power_fp::power_fp(const decltype(base)& x, decltype(exponent) && y) noexcept
	: base(x), exponent(std::move(y)), heuristic(reset_eval) {
	would_destructive_eval();
}

power_fp::power_fp(decltype(base) && x, const decltype(exponent)& y) noexcept
	: base(std::move(x)), exponent(y), heuristic(reset_eval) {
	would_destructive_eval();
}

power_fp::power_fp(decltype(base) && x, decltype(exponent) && y) noexcept
	: base(std::move(x)), exponent(std::move(y)), heuristic(reset_eval) {
	would_destructive_eval();
}

bool power_fp::self_square() {
	if (is_zero() || is_one()) return true; // fixed points
	if (exponent->is_scal_bn_identity()) return true;
	if (1 == exponent->scal_bn_is_safe(1)) {
		exponent->scal_bn(1);
		would_destructive_eval();
		return true;
	}
	if (zaimoni::math::in_place_square(base)) {
		would_destructive_eval();
		return true;
	}
	return false;
}

const math::type* power_fp::domain() const {
	if (0 >= exponent->domain()->subclass(zaimoni::math::get<_type<_type_spec::_R_SHARP_>>())) {
		// Some sort of Dedekind cut construction involved.  Correct for +, *
		return base->domain();
	}
	throw zaimoni::math::numeric_error("unhandled domain() for power_fp");
}


/// <returns>1: updated; -1: no change, stalled (should rewrite as product); 0: no change, not stalled</returns>
static int rearrange_pow(COW<fp_API>& base, COW<fp_API>& exponent)
{
	if (auto r = exponent.get_rw<var_fp<uintmax_t> >()) {
		if (0 != r->second->_x % 2) return -1;
		if (!r->first) exponent = std::unique_ptr<std::remove_reference_t<decltype(*(r->second->typed_clone()))> >(r->first = r->second->typed_clone()); // need this to fail first to be ACID
		if (!zaimoni::math::in_place_square(base)) return -1;
		r->first->_x /= 2;
		return 1;
	}
	if (auto r = exponent.get_rw<var_fp<intmax_t> >()) {
		if (0 != r->second->_x % 2) return -1;
		if (!r->first) exponent = std::unique_ptr<std::remove_reference_t<decltype(*(r->second->typed_clone()))> >(r->first = r->second->typed_clone()); // need this to fail first to be ACID
		if (!zaimoni::math::in_place_square(base)) return -1;
		r->first->_x /= 2;
		return 1;
	}
	return 0;
}

bool power_fp::is_zero() const {
	// multiplication, for now
	if (base->is_zero()) return !exponent->is_zero();	// 0^0 is technically undefined
	return false;
}

bool power_fp::is_one() const {
	// multiplication, for now
	if (base->is_one()) return true;
	if (exponent->is_zero()) return !base->is_zero();	// 0^0 is technically undefined
	return false;
}

int power_fp::sgn() const {
	if (is_zero()) return 0;
	if (!domain()->is_totally_ordered()) return 1;
	throw zaimoni::math::numeric_error("unhandled sgn() for power_fp");
}

std::string power_fp::to_s() const {
	auto ret(base->to_s());
	if (std::numeric_limits<int>::max() > base->precedence()) ret = "(" + ret + ")";
	ret += "<sup>" + exponent->to_s() + "</sup>";
	return ret;
}

void power_fp::_scal_bn(intmax_t scale) {
	throw zaimoni::math::numeric_error("power_fp: unhandled power-of-two scaling");
}

enum {
	zero_to_zero = -3,
	eval_to_zero = -2,
	is_base = -1
};

bool power_fp::would_destructive_eval() const
{
	if (0 > heuristic) return is_base == heuristic;
	if (base->is_one()) {
		heuristic = is_base;
		return true;
	}
	if (exponent->is_one()) {
		heuristic = is_base;
		return true;
	}
	if (base->is_zero()) {
		if (exponent->is_zero()) {
			heuristic = zero_to_zero;
			return false;
		}
		heuristic = is_base;
		return true;
	}
	if (exponent->is_zero()) {
		heuristic = eval_to_zero;
		return false;
	}
	return false;
}

power_fp::eval_type power_fp::destructive_eval()
{
	would_destructive_eval();
	switch (heuristic) {
	case is_base: return std::move(base);
	case zero_to_zero: throw zaimoni::math::numeric_error("tried to evaluate 0^0");
	}
	return nullptr;
}

bool power_fp::algebraic_self_eval()
{
	if (0 >= heuristic) return false;
	const auto base_code = heuristic / eval_strict_ub;
	const auto exp_code = heuristic % eval_strict_ub;
	bool ret = false;
	if (algebraic_eval == base_code) {
		if (fp_API::algebraic_reduce(base)) ret = true;
		else heuristic = (algebraic_eval - 1) * eval_strict_ub + exp_code;
	}
	if (algebraic_eval == exp_code) {
		if (fp_API::algebraic_reduce(exponent)) ret = true;
		else heuristic = base_code * eval_strict_ub + (algebraic_eval - 1);
	}
	if (ret) {
		would_destructive_eval();
		return true;
	}

	if (auto code = rearrange_pow(base, exponent)) {
		if (1 == code) {
			heuristic = algebraic_eval * eval_strict_ub + algebraic_eval;
			would_destructive_eval();
			return true;
		}
	}

	return false;
}

bool power_fp::inexact_self_eval()
{
	if (0 >= heuristic) return false;
	const auto base_code = heuristic / eval_strict_ub;
	const auto exp_code = heuristic % eval_strict_ub;

	bool ret = false;
	if (inexact_eval == base_code) {
		if (fp_API::inexact_reduce(base)) ret = true;
		else heuristic = (inexact_eval - 1) * eval_strict_ub + exp_code;
	}
	if (inexact_eval == exp_code) {
		if (fp_API::inexact_reduce(exponent)) ret = true;
		else heuristic = base_code * eval_strict_ub + (inexact_eval - 1);
	}
	if (ret) {
		heuristic = algebraic_eval * eval_strict_ub + algebraic_eval;
		would_destructive_eval();
		return true;
	}
	return false;
}

bool power_fp::self_eval() {
	// don't try to self_eval if we would be destructively evaluating
	if (would_destructive_eval()) return false;
	if (0 >= heuristic) return false;
	if (algebraic_self_eval()) return true;
	if (inexact_self_eval()) return true;
	return false;
}

fp_API* power_fp::_eval() const {
	switch (heuristic) {
	case eval_to_zero: {
		if (0 >= base->domain()->subclass(zaimoni::math::get<_type<_type_spec::_O_SHARP_>>())) {
			return new var_fp<double>(1); // some options here
		}
		break;
	}
	case zero_to_zero: throw zaimoni::math::numeric_error("tried to evaluate 0^0");
	}
	return nullptr;
}

} // namespace zaimoni