#include "symbolic_fp.hpp"
#include "Zaimoni.STL/numeric_error.hpp"
#include "arithmetic.hpp"
#include "sum.hpp"
#include "quotient.hpp"

namespace zaimoni {

std::any symbolic_fp::multinv_sum_ok(const typename eval_to_ptr<fp_API>::eval_type& x)
{
	if (const auto symbolic = dynamic_cast<const symbolic_fp*>(x.get_c())) {
		if (symbolic->mult_inverted()) return symbolic;
	}
	return std::any();
}

bool symbolic_fp::would_eval_multinv_sum(const std::any& lhs, const std::any& rhs)
{
	if (!std::any_cast<const symbolic_fp*>(&lhs)) return false;  // invariant?
	if (!std::any_cast<const symbolic_fp*>(&rhs)) return false;  // invariant?
	// \todo bail if multiplication is non-commutative
	// \todo bail if multiplication is incompatible
	// \todo bail if addition is incompatible
	return true;
}

fp_API* symbolic_fp::eval_multinv_sum(const typename eval_to_ptr<fp_API>::eval_type& lhs, const typename eval_to_ptr<fp_API>::eval_type& rhs)
{
	if (const auto l_symbolic = dynamic_cast<const symbolic_fp*>(lhs.get_c())) {
		if (const auto r_symbolic = dynamic_cast<const symbolic_fp*>(rhs.get_c())) {
			// basic form: c/a + d/b = (bc+ad)/(ab)
			// where c, b are (possibly negated) powers of 2
			typename eval_to_ptr<fp_API>::eval_type stage;
			auto l_numerator = std::unique_ptr<fp_API>(l_symbolic->scale_factor());
			if (auto r_numerator = std::unique_ptr<fp_API>(r_symbolic->scale_factor())) {
				if (l_numerator) {
					stage = (r_numerator.release() * l_symbolic->dest + l_numerator.release() * r_symbolic->dest) / (l_symbolic->dest * r_symbolic->dest);
				} else {
					stage = (r_numerator.release() * l_symbolic->dest + r_symbolic->dest) / (l_symbolic->dest * r_symbolic->dest);
				}
			} else {
				if (l_numerator) {
					stage = (l_symbolic->dest + l_numerator.release() * r_symbolic->dest) / (l_symbolic->dest * r_symbolic->dest);
				} else {
					stage = (l_symbolic->dest + r_symbolic->dest) / (l_symbolic->dest * r_symbolic->dest);
				}
			}
			return stage.release();
		}
	}

	return nullptr;
}

void symbolic_fp::global_init()
{
	static bool have_not_run = true;
	if (have_not_run) {
		sum::eval_algebraic_rule(std::pair(std::pair(multinv_sum_ok, multinv_sum_ok), std::pair(would_eval_multinv_sum, eval_multinv_sum)));
		have_not_run = true;
	}
}

symbolic_fp::symbolic_fp(const decltype(dest)& src) noexcept : dest(src), scale_by(0), bitmap(0) {
	assert(src);
	global_init();
}

symbolic_fp::symbolic_fp(decltype(dest) && src) noexcept : dest(std::move(src)), scale_by(0), bitmap(0) {
	assert(src);
	global_init();
}

symbolic_fp::symbolic_fp(const decltype(dest)& src, intmax_t scale_by) noexcept : dest(src), scale_by(scale_by), bitmap(0) {
	assert(src);
	global_init();
}

symbolic_fp::symbolic_fp(decltype(dest) && src, intmax_t scale_by) noexcept : dest(std::move(src)), scale_by(scale_by), bitmap(0) {
	assert(src);
	global_init();
}


fp_API* symbolic_fp::scale_factor() const
{
	if (0 == scale_by && !add_inverted()) return nullptr;	// 1 in _Z_

	auto stage = std::unique_ptr<symbolic_fp>(new symbolic_fp(zaimoni::math::mult_identity(math::get<_type<_type_spec::_Z_>>()), scale_by));
	if (add_inverted()) stage->self_negate();
	auto stage2 = COW<fp_API>(stage.release());
	while (fp_API::algebraic_reduce(stage2));
	return stage2.release();
}


int symbolic_fp::would_rearrange_sum(const typename eval_to_ptr<fp_API>::eval_type& rhs) const
{
	const auto r_symbolic = dynamic_cast<const symbolic_fp*>(rhs.get_c());
	if (!r_symbolic) return 0;
	bool l_mult_inverted = mult_inverted();
	bool r_mult_inverted = r_symbolic->mult_inverted();
	if (l_mult_inverted || r_mult_inverted) {
		if (l_mult_inverted && r_mult_inverted) return -1;	// harmonic mean
	}
	if (add_inverted() != add_inverted()) return 0;
	if (scale_by != r_symbolic->scale_by) return 0;
	return 1; // ok to rearrange-sum
}

int symbolic_fp::rearrange_sum(eval_to_ptr<fp_API>::eval_type& rhs) {
	switch (would_rearrange_sum(rhs)) {
	case 1: {
		if (const auto r_symbolic = ptr::writeable<symbolic_fp>(rhs)) {
			return zaimoni::math::rearrange_sum(dest, r_symbolic -> dest);
		}
	}
//		break;
	};
	return 0;
}

// we want to trigger evaluation to harmonic mean, but don't really have the context for it
// we *do* want to register with the sum type so it doesn't have to know about us
/*
fp_API* symbolic_fp::eval_sum(const typename eval_to_ptr<fp_API>::eval_type& rhs) const {
	switch (would_rearrange_sum(rhs)) {
	case -1: {
	}
		  //		break;
	};
	return nullptr;
}
*/

int symbolic_fp::score_sum(const typename eval_to_ptr<fp_API>::eval_type& rhs) const {
	return std::numeric_limits<int>::min() + ((0 != would_rearrange_sum(rhs)) ? 1 : 0);
}

void symbolic_fp::self_negate() {
	if (add_inverted()) bitmap &= ~(1ULL << (int)op::inverse_add);
	else bitmap |= 1ULL << (int)op::inverse_add;
}

void symbolic_fp::self_multinv() {
	if (mult_inverted()) bitmap &= ~(1ULL << (int)op::inverse_mult);
	else bitmap |= 1ULL << (int)op::inverse_mult;
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
		ret = std::string("(") + ret + ")";
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

symbolic_fp::eval_type symbolic_fp::destructive_eval()
{
	if (!scale_by && !bitmap) return std::move(dest);
	return nullptr;
}

bool symbolic_fp::algebraic_self_eval()
{
	if (fp_API::algebraic_reduce(dest)) return true;

	if (scale_by && dest->is_scal_bn_identity()) {
		scale_by = 0;
		return true;
	}

	if (add_inverted() && zaimoni::math::in_place_negate(dest)) {
		bitmap &= ~(1ULL << (int)op::inverse_add);
		return true;
	}
	if (scale_by) {
		if (!mult_inverted()) {
			if (zaimoni::math::scal_bn(dest, scale_by)) return true;
		} else {
			intmax_t ref_scale = (-INTMAX_MAX <= scale_by) ? -scale_by : INTMAX_MAX;
			intmax_t dest_scale = ref_scale;
			if (zaimoni::math::scal_bn(dest, dest_scale)) {
				scale_by -= (ref_scale - dest_scale); // \todo test this
				return true;
			}
		}
	}
	// \todo multiplicative inverse

	return false;
}

bool symbolic_fp::inexact_self_eval()
{
	if (fp_API::eval(dest)) return true;
	return false;
}

bool symbolic_fp::self_eval() {
	if (algebraic_self_eval()) return true;

	if (!scale_by && !bitmap) { // \todo unclear whether this is dead code
		auto working(dest);
		if (auto r = dynamic_cast<symbolic_fp*>(working.get())) {
			*this = *r;
			return true;
		}
		return false;
	}

	if (inexact_self_eval()) return true;
	return false;
}

fp_API* symbolic_fp::_eval() const {
	if (0 == scale_by && mult_inverted()) {
		COW<fp_API> test = new quotient(mult_identity(math::get<_type<_type_spec::_O_SHARP_>>()), dest);
		if (fp_API::eval(test)) return test.release();
	}

	return nullptr;
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

std::optional<bool> symbolic_fp::_is_finite() const {
	if (mult_inverted() && dest->is_zero()) return false;
	return dest->is_finite_kripke();
}

std::partial_ordering symbolic_fp::_value_compare(const fp_API* rhs) const
{
	if (const auto mine = dynamic_cast<const symbolic_fp*>(rhs)) {
		if (bitmap == mine->bitmap && scale_by == mine->scale_by) return dest->value_compare(mine->dest.get());
	}
	return std::partial_ordering::unordered; // the other case we can handle, is about to self-destructively evaluate anyway
}

}
