#include "arithmetic.hpp"
#include "Zaimoni.STL/var.hpp"
#include "Zaimoni.STL/interval.hpp"

// #include "quotient.hpp"
#include "product.hpp"
#include "sum.hpp"

#include "test_driver.h"

#include <array>

auto bootstrap_int(intmax_t src)
{
	std::array<zaimoni::eval_to_ptr<zaimoni::fp_API>::eval_type, 8> ret;

	ret[0] = new zaimoni::var_fp<double>(src);
	ret[1] = new zaimoni::var_fp<float>(src);
	ret[2] = new zaimoni::var_fp<long double>(src);
	ret[3] = new zaimoni::var_fp<double>(src);
	ret[4] = new zaimoni::var_fp<float>(src);
	ret[5] = new zaimoni::var_fp<long double>(src);
	ret[6] = new zaimoni::var_fp<intmax_t>(src);
	ret[7] = new zaimoni::var_fp<uintmax_t>(src);

	return ret;
}

static constexpr const char* primitive_numeric[] = {
	"double", "float", "long double",
	"zaimoni::math::interval<double>", "float", "long double",
	"intmax_t", "uintmax_t"
};

int main(int argc, char* argv[])
{
	auto zero = bootstrap_int(0);
	auto one = bootstrap_int(1);
	static_assert(std::end(primitive_numeric) - std::begin(primitive_numeric) == std::end(one) - std::begin(one));

	const zaimoni::product analytic_one;
	STRING_LITERAL_TO_STDOUT("zero-ary product: ");
	INFORM(analytic_one.to_s().c_str());

	const zaimoni::sum analytic_zero;
	STRING_LITERAL_TO_STDOUT("zero-ary sum: ");
	INFORM(analytic_zero.to_s().c_str());

	// this fully short-circuits
	STRING_LITERAL_TO_STDOUT("\ndomain-check 1*1\n");
	zaimoni::product stage_product;
	size_t i = 0, j;
	for(decltype(auto) lhs : one) {
		j = 0;
		for (decltype(auto) rhs : one) {
			stage_product = analytic_one;
			stage_product.append_term(lhs);
			stage_product.append_term(rhs);
			C_STRING_TO_STDOUT(primitive_numeric[i]);
			STRING_LITERAL_TO_STDOUT("*");
			C_STRING_TO_STDOUT(primitive_numeric[j]);
			STRING_LITERAL_TO_STDOUT(": ");
			INFORM(stage_product.to_s().c_str());
			++j;
		}
		++i;
	}

	// this also fully short-circuits
	STRING_LITERAL_TO_STDOUT("\ndomain-check 0+0\n");
	zaimoni::sum stage_sum;
	i = 0;
	for (decltype(auto) lhs : zero) {
		j = 0;
		for (decltype(auto) rhs : zero) {
			stage_sum = analytic_zero;
			stage_sum.append_term(lhs);
			stage_sum.append_term(rhs);
			C_STRING_TO_STDOUT(primitive_numeric[i]);
			STRING_LITERAL_TO_STDOUT("+");
			C_STRING_TO_STDOUT(primitive_numeric[j]);
			STRING_LITERAL_TO_STDOUT(": ");
			INFORM(stage_sum.to_s().c_str());
			++j;
		}
		++i;
	}

	STRING_LITERAL_TO_STDOUT("\ndomain-check 0*1\n");
	i = 0;
	for (decltype(auto) lhs : zero) {
		j = 0;
		for (decltype(auto) rhs : one) {
			stage_product = analytic_one;
			stage_product.append_term(lhs);
			stage_product.append_term(rhs);
			C_STRING_TO_STDOUT(primitive_numeric[i]);
			STRING_LITERAL_TO_STDOUT("*");
			C_STRING_TO_STDOUT(primitive_numeric[j]);
			STRING_LITERAL_TO_STDOUT(": ");
			INFORM(stage_product.to_s().c_str());
			++j;
		}
		++i;
	}

	STRING_LITERAL_TO_STDOUT("\ndomain-check 1*0\n");
	i = 0;
	for (decltype(auto) lhs : one) {
		j = 0;
		for (decltype(auto) rhs : zero) {
			stage_product = analytic_one;
			stage_product.append_term(lhs);
			stage_product.append_term(rhs);
			C_STRING_TO_STDOUT(primitive_numeric[i]);
			STRING_LITERAL_TO_STDOUT("*");
			C_STRING_TO_STDOUT(primitive_numeric[j]);
			STRING_LITERAL_TO_STDOUT(": ");
			INFORM(stage_product.to_s().c_str());
			++j;
		}
		++i;
	}

	STRING_LITERAL_TO_STDOUT("\ndomain-check 0+1\n");
	i = 0;
	for (decltype(auto) lhs : zero) {
		j = 0;
		for (decltype(auto) rhs : one) {
			stage_sum = analytic_zero;
			stage_sum.append_term(lhs);
			stage_sum.append_term(rhs);
			C_STRING_TO_STDOUT(primitive_numeric[i]);
			STRING_LITERAL_TO_STDOUT("+");
			C_STRING_TO_STDOUT(primitive_numeric[j]);
			STRING_LITERAL_TO_STDOUT(": ");
			INFORM(stage_sum.to_s().c_str());
			++j;
		}
		++i;
	}

	STRING_LITERAL_TO_STDOUT("\ndomain-check 1+0\n");
	i = 0;
	for (decltype(auto) lhs : one) {
		j = 0;
		for (decltype(auto) rhs : zero) {
			stage_sum = analytic_zero;
			stage_sum.append_term(lhs);
			stage_sum.append_term(rhs);
			C_STRING_TO_STDOUT(primitive_numeric[i]);
			STRING_LITERAL_TO_STDOUT("+");
			C_STRING_TO_STDOUT(primitive_numeric[j]);
			STRING_LITERAL_TO_STDOUT(": ");
			INFORM(stage_sum.to_s().c_str());
			++j;
		}
		++i;
	}

#if PROTOTYPE
	STRING_LITERAL_TO_STDOUT("\ndomain-check 1+1\n");
	i = 0;
	for (decltype(auto) lhs : one) {
		j = 0;
		for (decltype(auto) rhs : one) {
			stage_sum = analytic_zero;
			stage_sum.append_term(lhs);
			stage_sum.append_term(rhs);
			C_STRING_TO_STDOUT(primitive_numeric[i]);
			STRING_LITERAL_TO_STDOUT("+");
			C_STRING_TO_STDOUT(primitive_numeric[j]);
			STRING_LITERAL_TO_STDOUT(": ");
			INFORM(stage_sum.to_s().c_str());
			while (stage_sum.self_eval());
			INFORM(stage_sum.to_s().c_str());
			++j;
		}
		++i;
	}
#endif

	return 0;
}

