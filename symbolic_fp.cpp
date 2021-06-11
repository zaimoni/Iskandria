#include "symbolic_fp.hpp"
#include "Zaimoni.STL/numeric_error.hpp"
#include "arithmetic.hpp"

namespace zaimoni {

void symbolic_fp::self_negate() {
	if (add_inverted()) bitmap &= ~(1ULL << (int)op::inverse_add);
	else bitmap |= 1ULL << (int)op::inverse_add;
}

bool symbolic_fp::self_square() {
	if (is_zero() || is_one()) return true; // fixed points
	// multiplicative inverse doesn't matter here.

	if (0 < scale_by && 2 > INTMAX_MAX / scale_by) return false;
	if (0 > scale_by && 2 > INTMAX_MIN / scale_by) return false;

	if (zaimoni::math::in_place_square(dest)) {
		bitmap &= ~(1ULL << (int)op::inverse_add); // no longer additively inverted
		scale_by *= 2;
		return true;
	}

	return false;
}

const math::type* symbolic_fp::domain() const {
	if (mult_inverted()) return dest->domain()->inverse(_type_spec::Multiplication);
	return dest->domain();
}

bool symbolic_fp::self_eval() {
	if (scale_by && dest->is_scal_bn_identity()) {
		scale_by = 0;
		return true;
	}
	if (!scale_by && !bitmap) {
		auto working(dest);
		if (auto r = dynamic_cast<symbolic_fp*>(working.get())) {
			*this = *r;
			return true;
		}
		return false;
	}
	if (add_inverted() && zaimoni::math::in_place_negate(dest)) {
		bitmap &= ~(1ULL << (int)op::inverse_add);
		return true;
	}
	if (scale_by) {
		if (!mult_inverted()) {
			if (zaimoni::math::scal_bn(dest, scale_by)) return true;
		}
		else {
			intmax_t ref_scale = (-INTMAX_MAX <= scale_by) ? -scale_by : INTMAX_MAX;
			intmax_t dest_scale = ref_scale;
			if (zaimoni::math::scal_bn(dest, dest_scale)) {
				scale_by -= (ref_scale - dest_scale); // \todo test this
				return true;
			}
		}
	}
	// \todo multiplicative inverse

	// final failover
	if (dest->self_eval()) return true;
	if (fp_API::eval(dest)) return true;
	return false;
}

bool symbolic_fp::is_zero() const {
	if (mult_inverted()) return false;
	return dest->is_zero();
}

bool symbolic_fp::is_one() const {
	if (add_inverted() || 0 != scale_by) return false;
	return dest->is_one();
}

int symbolic_fp::sgn() const {
	if (dest->is_zero()) return 0;
	if (add_inverted() && domain()->is_totally_ordered()) return -dest->sgn();
	return dest->sgn();
}

std::optional<intmax_t> symbolic_fp::_scal_bn_is_unsafe(intmax_t scale) const
{
	if (0 <= scale) {
		if (0 >= scale_by) return std::nullopt;
		const auto delta = INTMAX_MAX - scale_by;
		if (delta >= scale) return std::nullopt;
		return delta;
	}
	// if (0 >= scale) {
	if (0 <= scale_by) return std::nullopt;
	const auto delta = INTMAX_MIN - scale_by;
	if (delta <= scale) return std::nullopt;
	return delta;
	// }
}

intmax_t symbolic_fp::_probe_dest(intmax_t scale) const
{
	if (mult_inverted()) {
		auto neg_scale = (-INTMAX_MAX <= scale) ? -scale : INTMAX_MAX;
		return -dest->scal_bn_is_safe(neg_scale);
	}
	else {
		return dest->scal_bn_is_safe(scale);
	}
}

intmax_t symbolic_fp::scal_bn_is_safe(intmax_t scale) const
{
	decltype(auto) delta = _scal_bn_is_unsafe(scale);
	if (!delta) return scale;
	// internal buffer wasn't enough
	auto probe = _probe_dest(scale);
	if (probe == scale) return scale;
	auto error = scale - probe;
	if (0 < error) {
		if (*delta >= error) return scale;
		return *delta + probe;
	}
	else /* if (0 > error)*/ {
		if (*delta <= error) return scale;
		return *delta + probe;
	}
}

intmax_t symbolic_fp::ideal_scal_bn() const {
	auto interior = dest->ideal_scal_bn();
	const bool tweaked = mult_inverted() && -INTMAX_MAX > interior;
	if (tweaked) ++interior;
	if (mult_inverted()) interior = -interior;

	if (0 <= scale_by && 0 >= interior) return scale_by + interior;
	if (0 >= scale_by && 0 <= interior) return scale_by + interior;
	if (0 < scale_by /* && 0 < interior */) return INTMAX_MAX - scale_by >= interior ? scale_by + interior : INTMAX_MAX;
	/* if (0 > scale_by && 0 > interior )*/ return INTMAX_MIN - scale_by <= interior ? scale_by + interior : INTMAX_MIN;
}

fp_API* symbolic_fp::clone() const {
	if (0 == scale_by && !bitmap) return dest->clone();
	return new symbolic_fp(*this);
}

std::string symbolic_fp::to_s() const {
	decltype(auto) ret(dest->to_s());
	if (!scale_by && !bitmap) return ret;
	if (scale_by || bitmap) {
		ret = "(" + ret + ")";
		if (mult_inverted()) ret += "<sup>-1</sup>";
		if (add_inverted()) ret = "-" + ret;
		if (scale_by) ret += "*2<sup>" + std::to_string(scale_by) + "</sup>";
	}
	return ret;
}


// coordinate with the sum/product types
int symbolic_fp::precedence() const {
	if (mult_inverted()) return _type_spec::Multiplication;
	return _type_spec::Addition;
}

void symbolic_fp::_scal_bn(intmax_t scale)
{
	if (0 < scale) {
		if (0 >= scale_by || INTMAX_MAX - scale_by >= scale) scale_by += scale;
		else throw zaimoni::math::numeric_error("overflowed power-of-two scaling");
	}
	else if (0 > scale) {
		if (0 <= scale_by || -INTMAX_MIN - scale_by <= scale) scale_by += scale;
		else throw zaimoni::math::numeric_error("overflowed power-of-two scaling");
	}
	self_eval();
}

std::shared_ptr<fp_API> symbolic_fp::destructive_eval()
{
	if (!scale_by && !bitmap) return dest;
	return nullptr;
}

bool symbolic_fp::_is_inf() const {
	if (mult_inverted()) return dest->is_zero();
	return dest->is_inf();
}

bool symbolic_fp::_is_finite() const {
	if (mult_inverted()) return !dest->is_zero();
	return dest->is_finite();
}

}
