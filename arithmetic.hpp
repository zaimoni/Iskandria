#ifndef ARITHMETIC_HPP
#define ARITHMETIC_HPP 1

#include "Zaimoni.STL/eval.hpp"

namespace zaimoni {

std::shared_ptr<fp_API> scalBn(const std::shared_ptr<fp_API>& src, intmax_t scale);
void self_scalBn(std::shared_ptr<fp_API>& src, intmax_t scale);

namespace math {

int rearrange_sum(std::shared_ptr<fp_API>& lhs, std::shared_ptr<fp_API>& rhs);
int rearrange_product(std::shared_ptr<fp_API>& lhs, std::shared_ptr<fp_API>& rhs);
fp_API* eval_quotient(const std::shared_ptr<fp_API>& n, const std::shared_ptr<fp_API>& d);
int sum_implemented(const std::shared_ptr<fp_API>& x);
int sum_score(const std::shared_ptr<fp_API>& lhs, const std::shared_ptr<fp_API>& rhs);
std::shared_ptr<fp_API> eval_sum(const std::shared_ptr<fp_API>& lhs, const std::shared_ptr<fp_API>& rhs);
bool in_place_negate(std::shared_ptr<fp_API>& lhs);
bool in_place_square(std::shared_ptr<fp_API>& lhs);
bool scal_bn(std::shared_ptr<fp_API>& x, intmax_t& scale);
bool scal_bn(COW<fp_API>& x, intmax_t& scale);

}

std::shared_ptr<fp_API> operator+(const std::shared_ptr<fp_API>& lhs, const std::shared_ptr<fp_API>& rhs);
std::shared_ptr<fp_API>& operator+=(std::shared_ptr<fp_API>& lhs, const std::shared_ptr<fp_API>& rhs);

std::shared_ptr<fp_API> operator*(const std::shared_ptr<fp_API>& lhs, const std::shared_ptr<fp_API>& rhs);
std::shared_ptr<fp_API> operator/(const std::shared_ptr<fp_API>& lhs, const std::shared_ptr<fp_API>& rhs);

std::shared_ptr<fp_API> operator-(const std::shared_ptr<fp_API>& lhs);

std::shared_ptr<fp_API> pow(const std::shared_ptr<fp_API>& base, const std::shared_ptr<fp_API>& exponent);

}

#endif
