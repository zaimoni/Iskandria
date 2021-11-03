#ifndef ARITHMETIC_HPP
#define ARITHMETIC_HPP 1

#include "Zaimoni.STL/eval.hpp"

namespace zaimoni {

COW<fp_API> scalBn(const COW<fp_API>& src, intmax_t scale);

namespace math {

int rearrange_sum(COW<fp_API>& lhs, COW<fp_API>& rhs);
int rearrange_product(COW<fp_API>& lhs, COW<fp_API>& rhs);

int product_score(const COW<fp_API>& lhs);
int product_score(const COW<fp_API>& lhs, const COW<fp_API>& rhs);
// (+, *) elementary operation counts
void update_op_count_product(const COW<fp_API>& lhs, const COW<fp_API>& rhs, std::pair<int, int>& accumulator);
COW<fp_API> eval_product(const COW<fp_API>& lhs, const COW<fp_API>& rhs);
COW<fp_API> mult_identity(const type& src);

fp_API* eval_quotient(const COW<fp_API>& n, const COW<fp_API>& d);

int sum_score(const COW<fp_API>& x); // i.e., do we have a backend for this
int sum_score(const COW<fp_API>& lhs, const COW<fp_API>& rhs);
COW<fp_API> eval_sum(const COW<fp_API>& lhs, const COW<fp_API>& rhs);
COW<fp_API> add_identity(const type& src);

bool in_place_negate(eval_to_ptr<fp_API>::eval_type& lhs);
bool in_place_square(COW<fp_API>& x);
bool scal_bn(COW<fp_API>& x, intmax_t& scale);

}

eval_to_ptr<fp_API>::eval_type operator+(const eval_to_ptr<fp_API>::eval_type& lhs, const eval_to_ptr<fp_API>::eval_type& rhs);
eval_to_ptr<fp_API>::eval_type& operator+=(eval_to_ptr<fp_API>::eval_type& lhs, const eval_to_ptr<fp_API>::eval_type& rhs);

eval_to_ptr<fp_API>::eval_type operator*(const eval_to_ptr<fp_API>::eval_type& lhs, const eval_to_ptr<fp_API>::eval_type& rhs);
eval_to_ptr<fp_API>::eval_type& operator*=(eval_to_ptr<fp_API>::eval_type& lhs, const eval_to_ptr<fp_API>::eval_type& rhs);
eval_to_ptr<fp_API>::eval_type operator/(const eval_to_ptr<fp_API>::eval_type& lhs, const eval_to_ptr<fp_API>::eval_type& rhs);

void negate_in_place(eval_to_ptr<fp_API>::eval_type& lhs);
eval_to_ptr<fp_API>::eval_type operator-(const eval_to_ptr<fp_API>::eval_type& lhs);

eval_to_ptr<fp_API>::eval_type pow(const eval_to_ptr<fp_API>::eval_type& base, const eval_to_ptr<fp_API>::eval_type& exponent);

}

#endif
