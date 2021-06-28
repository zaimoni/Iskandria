#include "complex.hpp"
#include "arithmetic.hpp"
#include "Zaimoni.STL/var.hpp"

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
	self_scalBn(a, scale);
	self_scalBn(b, scale);
	heuristics = 0x0F;
}

}
}
