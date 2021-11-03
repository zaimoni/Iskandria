#include "complex.hpp"
#include "arithmetic.hpp"
#include "Zaimoni.STL/var.hpp"
#include <memory>

namespace zaimoni {
namespace math {

// heuristic constants
enum complex_state : uintmax_t 
{
	Re_self_eval = 1ULL << 0,
	Im_self_eval = 1ULL << 1,
	Re_eval = 1ULL << 2,
	Im_eval = 1ULL << 3
};

complex::complex(const decltype(a)& re, const decltype(b)& im) : a(re), b(im), heuristics(Re_self_eval | Im_self_eval | Re_eval | Im_eval) {
	decltype(auto) R = get<_type<_type_spec::_R_SHARP_> >();
	const zaimoni::math::type* domain;
	if (!(domain = re->domain()) || 0 < domain->subclass(R)) throw new std::logic_error("non-real coordinate for real part");
	if (!(domain = im->domain()) || 0 < domain->subclass(R)) throw new std::logic_error("non-real coordinate for imaginary part");
};

// friend functions
complex::eval_type Conj(const complex& z) { return complex::eval_type(new complex(z.a, -z.b)); }

complex::eval_type norm2(const complex& z) {
	complex::eval_type two(new var_fp<intmax_t>(2));
	return pow(z.a, two) + pow(z.b, two);
}

bool complex::self_eval() {
	if (!heuristics) return false;
	bool ret = false;
	if ((heuristics & Re_self_eval)) {
		if (a->self_eval()) {
			ret = true;
		} else {
			heuristics &= ~Re_self_eval;
		}
	}
	if ((heuristics & Im_self_eval)) {
		if (b->self_eval()) {
			ret = true;
		} else {
			heuristics &= ~Im_self_eval;
		}
	}
	if (ret) return true;
	if ((heuristics & Re_eval)) {
		if (fp_API::eval(a)) {
			ret = true;
			heuristics |= Re_self_eval;
		} else {
			heuristics &= ~Re_eval;
		}
	}
	if ((heuristics & Im_eval)) {
		if (fp_API::eval(b)) {
			ret = true;
			heuristics |= Im_self_eval;
		} else {
			heuristics &= ~Im_eval;
		}
	}
	return ret;
}

intmax_t complex::scal_bn_is_safe(intmax_t scale) const {
	auto ret = b->scal_bn_is_safe(a->scal_bn_is_safe(scale));
	while (ret != scale) {
		if (0 == ret) return 0;
		scale = ret;
		ret = b->scal_bn_is_safe(a->scal_bn_is_safe(scale));
	}
	return scale;
}

intmax_t complex::ideal_scal_bn() const {
	auto a_ideal = a->ideal_scal_bn();
	auto b_ideal = b->ideal_scal_bn();
	while (0 != a_ideal || 0 != b_ideal) {
		auto test = b->scal_bn_is_safe(a_ideal);
		if (test != a_ideal) {
			a_ideal = a->scal_bn_is_safe(test);
			continue;
		}
		test = a->scal_bn_is_safe(b_ideal);
		if (test != b_ideal) {
			b_ideal = b->scal_bn_is_safe(test);
			continue;
		}
		return (a_ideal < b_ideal) ? b_ideal : a_ideal;
	};
	return 0;
}

std::string complex::to_s() const {
	std::string ret_re(a->to_s());
	std::string ret_im(b->to_s());
	if (_type_spec::Addition > a->precedence_to_s()) {
		ret_re = "(" + ret_re + ")";
	}
	if (_type_spec::Multiplication > b->precedence_to_s()) {
		ret_im = "(" + ret_im + ")";
	}
	return ret_re + " + " + ret_im + "<i>i</i>";
}

void complex::_scal_bn(intmax_t scale) {
	a->scal_bn(scale);
	b->scal_bn(scale);
	heuristics = 0x0F;
}

std::optional<bool> complex::_is_finite() const
{
	const auto re_test = a->is_finite_kripke();
	if (re_test && !*re_test) return false;
	const auto im_test = b->is_finite_kripke();
	if (im_test && !*im_test) return false;
	if (re_test && im_test) return true;
	return std::nullopt;
}

int complex::rearrange_sum(eval_type& rhs)
{
retry:
	if (auto r = rhs.get_rw<complex>()) {
		if (!r->first) {
			rhs = r->second->clone();
			goto retry;
		}
		int ret = 0;
		int zero_rhs = 0;
		if (r->first->a->is_zero()) {
			zero_rhs += 1;
		} else if (a->is_zero()) {
			swap(a, r->first->a);
			zero_rhs += 1;
		}
		if (r->first->b->is_zero()) {
			zero_rhs += 2;
		} else if (b->is_zero()) {
			swap(b, r->first->b);
			zero_rhs += 2;
		}
		if (3 == zero_rhs) return 1;	// rhs now zero
		if (2 != zero_rhs) {
			switch (const int code = zaimoni::math::rearrange_sum(b, r->first->b)) {
			case -1: // lhs annihilation requested
				swap(b, r->first->b);
				[[fallthrough]];
			case -2: // mutual annihilation requested
			case 1: // rhs annihilation requested
				zero_rhs += 2;
				if (3 == zero_rhs) return 1;	// rhs now zero
				break;
			case 2:
				ret = 2;	// pass through "changed"
			}
		}
		if (1 != zero_rhs) {
			switch (const int code = zaimoni::math::rearrange_sum(a, r->first->a)) {
			case -1: // lhs annihilation requested
				swap(a, r->first->a);
				[[fallthrough]];
			case -2: // mutual annihilation requested
			case 1: // rhs annihilation requested
				zero_rhs += 1;
				if (3 == zero_rhs) return 1;	// rhs now zero
				break;
			case 2:
				ret = 2;	// pass through "changed"
			}
		}
		return ret;
	}

	auto rhs_domain = rhs->domain();
	if (!rhs_domain) return 0;	// arguably hard error
	if (0 >= rhs_domain->subclass(math::get<_type<_type_spec::_R_SHARP_>>())) {
		// extended real number -- try to delegate
		if (a->is_zero()) {
			swap(a, rhs);
			return 1; // request annihilating rhs
		}
		switch (const int code = zaimoni::math::rearrange_sum(a, rhs)) {
		case -1: // lhs annihilation requested
			swap(a, rhs);
			return 1; // request annihilating rhs rather than lhs
		case -2: // double annihilation requested
			return b->is_zero() ? -2 : 1;  // but only pass through if our imaginary part is zero, otherwise just request rhs
		default:
			return code;	// original value doesn't request annihilating us
		}
	}

	return 0;
}

// caller will handle checking for asymmetric handling for these two
fp_API* complex::eval_sum(const eval_to_ptr<fp_API>::eval_type& rhs) const
{
	auto src = rhs.get_c();
	if (auto r = dynamic_cast<const complex*>(src)) return new complex(a + r->a, b + r->b);

	auto rhs_domain = rhs->domain();
	if (!rhs_domain) return nullptr;	// arguably hard error
	if (0 >= rhs_domain->subclass(math::get<_type<_type_spec::_R_SHARP_>>())) {
		return new complex(a + rhs, b);
	}

	return nullptr;
}

int complex::score_sum(const eval_to_ptr<fp_API>::eval_type& rhs) const
{
	auto src = rhs.get_c();
	if (auto r = dynamic_cast<const complex*>(src)) {
		if (auto re_test = sum_score(a, r->a); std::numeric_limits<int>::min() + 1 < re_test) return re_test;
		if (auto im_test = sum_score(b, r->b); std::numeric_limits<int>::min() + 1 < im_test) return im_test;
		return std::numeric_limits<int>::min() + 1;
	}

	auto rhs_domain = rhs->domain();
	if (!rhs_domain) return std::numeric_limits<int>::min();	// arguably hard error
	if (0 >= rhs_domain->subclass(math::get<_type<_type_spec::_R_SHARP_>>())) {
		if (auto re_test = sum_score(a, rhs); std::numeric_limits<int>::min() + 1 < re_test) return re_test;
		return std::numeric_limits<int>::min() + 1;
	}

	return std::numeric_limits<int>::min();
}

void complex::self_negate()
{
	negate_in_place(a);
	negate_in_place(b);
}

int complex::rearrange_product(eval_to_ptr<fp_API>::eval_type& rhs)
{
	return 0;	// \todo implement (but might be tricky)
}

fp_API* complex::eval_product(const typename eval_to_ptr<fp_API>::eval_type& rhs) const
{
	auto src = rhs.get_c();
	if (auto r = dynamic_cast<const complex*>(src)) {
		return new complex(a * r->a + -(b * r->b), a * r->b + b * r->a);
	}
	auto rhs_domain = rhs->domain();
	if (!rhs_domain) return nullptr;	// arguably hard error
	if (0 >= rhs_domain->subclass(math::get<_type<_type_spec::_R_SHARP_>>())) {
		return new complex(a * rhs, b * rhs);
	}

	return nullptr;
}

std::optional<std::pair<int, int> > complex::product_op_count(const typename eval_to_ptr<fp_API>::eval_type& rhs) const
{
	auto src = rhs.get_c();
	if (auto r = dynamic_cast<const complex*>(src)) {
		std::pair ret(2, 0);
		update_op_count_product(a, r->a, ret);
		update_op_count_product(b, r->b, ret);
		update_op_count_product(a, r->b, ret);
		update_op_count_product(b, r->a, ret);
		return ret;
	}
	auto rhs_domain = rhs->domain();
	if (!rhs_domain) return std::nullopt;	// arguably hard error
	if (0 >= rhs_domain->subclass(math::get<_type<_type_spec::_R_SHARP_>>())) {
		std::pair ret(0, 0);
		update_op_count_product(a, rhs, ret);
		update_op_count_product(b, rhs, ret);
		return ret;
	}

	return std::nullopt;
}

int complex::rearrange_dividedby(eval_to_ptr<fp_API>::eval_type& rhs)
{
	auto rhs_domain = rhs->domain();
	if (!rhs_domain) return 0;	// arguably hard error
	if (0 >= rhs_domain->subclass(math::get<_type<_type_spec::_R_SHARP_>>())) {
		auto stage_re = std::unique_ptr<fp_API>(zaimoni::math::eval_quotient(a, rhs));
		auto stage_im = std::unique_ptr<fp_API>(zaimoni::math::eval_quotient(b, rhs));
		*this = complex(stage_re ? stage_re.release() : a / rhs, stage_im ? stage_im.release() : b / rhs);
		rhs = zaimoni::math::mult_identity(*rhs_domain);
		return 1;
	};

	return 0;
}


} // namespace math
} // namespace zaimoni
