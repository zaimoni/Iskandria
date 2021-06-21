#include "sum.hpp"
#include "arithmetic.hpp"
#include "Zaimoni.STL/numeric_error.hpp"
#include "Zaimoni.STL/Logging.h"

namespace zaimoni {

bool sum::_append_infinity(const smart_ptr& src) {
	auto i = this->_x.size();
	const auto sign = src->sgn();
	while (0 < i) {
		const auto& x = this->_x[--i];
		if (x->is_finite()) {
			this->_x.erase(this->_x.begin() + i);
			continue;
		}
		else if (x->is_inf()) {
			const auto x_sign = x->sgn();
			if (sign == x_sign) return false;
			throw zaimoni::math::numeric_error("infinity-infinity");
		}
	}
	return true;
}

void sum::append_term(const smart_ptr& src) {
	if (!src || src->is_zero()) return;
	assert(src->domain());
	if (src->is_inf() && !_append_infinity(src)) return;	// mostly an annihilator
	this->_append_term(src);
}

void sum::append_term(smart_ptr&& src) {
	if (!src || src->is_zero()) return;
	assert(src->domain());
	if (src->is_inf() && !_append_infinity(src)) return;	// mostly an annihilator
	this->_append_term(std::move(src));
}

bool sum::would_fpAPI_eval() const { return 1 >= this->_x.size(); }

sum::result_type sum::destructive_eval() {
	if (1 == this->_x.size()) return this->_x.front();
	return nullptr;
}

bool sum::self_eval() {
	if (!this->_pre_self_eval()) return false;
	if (this->_self_eval(_n_ary_op::is_additive_identity, zaimoni::math::rearrange_sum, zaimoni::math::sum_implemented, zaimoni::math::sum_score, zaimoni::math::eval_sum)) return true;
	//	auto& checking = this->_heuristic.back();
	// \todo process our specific rules
	this->_heuristic.clear();
	return false;
}

bool sum::is_zero() const {
	if (this->_x.empty()) return true;
	if (1 == this->_x.size()) return this->_x.front()->is_zero();
	return false;
}

bool sum::is_one() const {
	if (1 == this->_x.size()) return this->_x.front()->is_one();
	return false;
}

int sum::sgn() const {
	if (is_zero()) return 0;
	unsigned int seen = 0;
	for (auto& x : this->_x) {
		if (x->is_zero()) continue;	// should have normalized this away
		const auto test = x->sgn();
		if (7U == (seen |= (1 << (test + 1)))) break;
	}
	if (0 == seen) return 0;
	if (4 == seen) return 1;	// only saw positive
	if (1 == seen) return -1;	// only saw negative
	// anything else would need evaluation to get right
	throw zaimoni::math::numeric_error("sum needs to evaluate enough to calculate sgn()");
}

intmax_t sum::scal_bn_is_safe(intmax_t scale) const
{
restart:
	for (const auto& x : this->_x) {
		if (x->is_scal_bn_identity()) continue;
		intmax_t test = x->scal_bn_is_safe(scale);
		if (test != scale) {
			if (0 == test) return 0;
			scale = test;
			goto restart;
		}
	}
	return scale;
}

intmax_t sum::ideal_scal_bn() const {
	if (is_scal_bn_identity() || is_one()) return 0;
	intmax_t ret = 0;
	for (const auto& x : this->_x) {
		const auto test = x->ideal_scal_bn();
		if (0 < test) {
			if (0 > ret) return 0;
			if (0 < ret && test >= ret) continue;
			ret = test;
		}
		else if (0 > test) {
			if (0 < ret) return 0;
			if (0 > ret && test <= ret) continue;
			ret = test;
		}
		else return 0;
	}
	return ret;
}

const math::type* sum::domain() const
{
	if (_x.empty()) return &math::get<_type<_type_spec::_R_SHARP_>>(); // omni-zero is unconstrained \todo should be integers
	std::vector<decltype(domain())> accumulator;
	for (decltype(auto) arg : _x) {
		decltype(domain()) test = arg->domain();
		if (!test) continue; // \todo invariant violation
		if (accumulator.empty()) {
			accumulator.push_back(test);
			continue;
		}
		if (auto op_type = math::type::defined(*accumulator.back(), _type_spec::Addition, *test)) {
			accumulator.back() = op_type;
			continue;
		}
		throw std::logic_error("unhandled addition domain");
	}
	const size_t ub = accumulator.size();
	if (0 == ub) return nullptr;
	else if (1 == ub) return accumulator.front();
	else throw std::logic_error("unhandled addition domain");
}

std::string sum::to_s() const {
	if (this->_x.empty()) return "0";
	const auto _size = this->_x.size();
	if (1 == _size) return this->_x.front()->to_s();
	std::string ret;
	for (auto& x : this->_x) {
		auto tmp = x->to_s();
		if (_precedence >= x->precedence_to_s()) tmp = std::string("(") + tmp + ')';
		if (ret.empty()) {
			ret = std::move(tmp);
		} else {
			ret += '+' + tmp;
		}
	}
	return ret;
}

bool sum::_is_inf() const {
	for (auto& x : this->_x) if (x->is_inf()) return true;
	return false;
}

bool sum::_is_finite() const {
	for (auto& x : this->_x) if (!x->is_finite()) return false;
	return true;
}

void sum::_scal_bn(intmax_t scale) {
	for (auto& x : this->_x) self_scalBn(x, scale);
}

} // namespace zaimoni

