#include "power_fp.hpp"
#include "arithmetic.hpp"
#include "Zaimoni.STL/var.hpp"

namespace zaimoni {

bool power_fp::self_square() {
	if (is_zero() || is_one()) return true; // fixed points
	if (exponent->is_scal_bn_identity()) return true;
	if (1 == exponent->scal_bn_is_safe(1)) {
		exponent->scal_bn(1);
		return true;
	}
	if (zaimoni::math::in_place_square(base)) return true;
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

bool power_fp::self_eval() {
	// don't try to self_eval if we would be destructively evaluating
	if (base->is_one()) return false;
	if (exponent->is_one()) return false;
	if (base->is_zero()) return false;
	if (exponent->is_zero()) return false;

	bool base_eval = base->self_eval();
	bool exp_eval = exponent->self_eval();
	if (base_eval || exp_eval) return true;

	if (auto code = rearrange_pow(base, exponent)) return 1 == code;

	// final failover
	base_eval = fp_API::eval(base);
	exp_eval = fp_API::eval(exponent);

	return base_eval || exp_eval;
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

power_fp::eval_type power_fp::destructive_eval()
{
	if (base->is_one()) return std::move(base);
	if (exponent->is_one()) return std::move(base);
	if (base->is_zero()) {
		if (exponent->is_zero()) throw zaimoni::math::numeric_error("tried to evaluate 0^0");
		return std::move(base);
	}
	if (exponent->is_zero()) return nullptr; // forwarding to raw evaluation
	return nullptr;
}

fp_API* power_fp::_eval() const {
	if (exponent->is_zero()) {
		// \todo lift to a function against the type
		if (0 >= base->domain()->subclass(zaimoni::math::get<_type<_type_spec::_O_SHARP_>>())) {
			return new var_fp<double>(1); // some options here
		}
	}
	return nullptr;
}

} // namespace zaimoni