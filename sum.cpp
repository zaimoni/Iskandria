#include "sum.hpp"
#include "arithmetic.hpp"
#include "Zaimoni.STL/numeric_error.hpp"
#include "Zaimoni.STL/Logging.h"
#include <algorithm>

namespace zaimoni {

// external inference rule support
std::vector<sum::rule_guard> sum::eval_ok;
std::vector<sum::eval_rule_spec> sum::eval_algebraic_rules;
std::vector<sum::eval_rule_spec> sum::eval_inexact_rules;

#define GUARDED_APPEND(DEST, SRC) \
	do {if (DEST.end() == std::ranges::find(DEST, SRC)) DEST.push_back(SRC);} while(0)

void sum::eval_algebraic_rule(const eval_rule_spec& src)
{
	if (!src.second.second) return;
	if (!src.second.first) return;
	if (!src.first.second) return;
	if (!src.first.first) return;
	GUARDED_APPEND(eval_ok, src.first.first);
	GUARDED_APPEND(eval_ok, src.first.second);
	GUARDED_APPEND(eval_algebraic_rules, src);
}

void sum::eval_inexact_rule(const eval_rule_spec& src)
{
	if (!src.second.second) return;
	if (!src.second.first) return;
	if (!src.first.second) return;
	if (!src.first.first) return;
	GUARDED_APPEND(eval_ok, src.first.first);
	GUARDED_APPEND(eval_ok, src.first.second);
	GUARDED_APPEND(eval_inexact_rules, src);
}

#undef GUARDED_APPEND

// \todo allow domain upgrade
bool sum::_append_infinity(const smart_ptr& src) {
	auto i = this->_x.size();
	const auto sign = src->sgn();
	while (0 < i) {
		const auto& x = this->_x[--i];
		if (x->is_finite()) {
			this->_x.erase(this->_x.begin() + i);
			continue;
		} else if (x->is_inf()) {
			const auto x_sign = x->sgn();
			if (sign == x_sign) return false;
			throw zaimoni::math::numeric_error("infinity-infinity");
		}
	}
	return true;
}

void sum::_append(smart_ptr&& src)
{
	if (src->is_inf() && !_append_infinity(src)) return;	// mostly an annihilator
	this->_append_term(src);
}

void sum::append_term(const smart_ptr& src) {
	if (!src || src->is_zero()) return;
	assert(src->domain());
	_append(smart_ptr(src));
}

void sum::append_term(smart_ptr&& src) {
	if (!src || src->is_zero()) return;
	assert(src->domain());
	_append(std::move(src));
}

bool sum::would_fpAPI_eval() const { return 1 >= this->_x.size(); }

sum::eval_type sum::destructive_eval() {
	if (1 == this->_x.size()) return std::move(this->_x.front());
	return nullptr;
}

static void apply_eval(std::vector<eval_to_ptr<fp_API>::eval_type>& dest, std::unique_ptr<fp_API>& src, ptrdiff_t lhs, ptrdiff_t rhs)
{
	if (lhs > rhs) swap(lhs, rhs);
	dest.erase(dest.begin() + rhs);
	if (sum::is_identity(src.get())) {
		dest.erase(dest.begin() + lhs);
	} else {
		dest[lhs] = std::move(src);
	}

}

bool sum::algebraic_self_eval() {
	// flush removal of identity from prior implementation
	bool ret = false;
	while(!_heuristic.empty()) {
		auto& checking = this->_heuristic.back();
		if (_n_ary_op::remove_identity == checking.first) {
			const auto i = checking.second;
			_heuristic.pop_back();
			if (_x.size() > i) _x.erase(_x.begin() + i);
			ret = true;
		}
		break;
	}
	if (ret) return true;
	if (2 > _x.size()) return false;

	for (decltype(auto) x : _x) {
		if (fp_API::algebraic_reduce(x)) ret = true;
	}
	if (ret) return true;

	if (eval_algebraic_rules.empty()) return false;

	std::map<size_t, std::vector<std::pair<ptrdiff_t, std::any> > > interpreted; // interesting candidate for cache
	// effective iteration order...rule, lhs index, rhs_index
	for (decltype(auto) rule : eval_algebraic_rules) {
		const auto lhs_rule_index = std::ranges::find(eval_ok, rule.first.first) - eval_ok.begin();
		const auto rhs_rule_index = std::ranges::find(eval_ok, rule.first.second) - eval_ok.begin();
		auto lhs_args = interpreted.find(lhs_rule_index);
		if (interpreted.end() == lhs_args) {
			decltype(interpreted.begin()->second) staging;
			const auto origin = _x.begin();
			const auto end = _x.end();
			auto iter = _x.begin();
			do {
				auto test = rule.first.first(*iter);
				if (test.has_value()) staging.push_back(std::pair(iter - origin, test));
			} while (end != ++iter);
			interpreted[lhs_rule_index] = std::move(staging);
			lhs_args = interpreted.find(lhs_rule_index);
		}
		auto& lhs_seen = lhs_args->second;
		if (lhs_seen.empty()) continue;
		if (lhs_rule_index == rhs_rule_index) {
			if (2 <= lhs_seen.size()) {
				ptrdiff_t anchor = lhs_seen.size() - 1;
				auto pivot = anchor - 1;
				do {
					if (rule.second.first(lhs_seen[pivot].second, lhs_seen[anchor].second)) {
						auto stage = std::unique_ptr<fp_API>(rule.second.second(_x[lhs_seen[pivot].first], _x[lhs_seen[anchor].first]));
						if (!stage) continue;
						apply_eval(_x, stage, lhs_seen[pivot].first, lhs_seen[anchor].first);
						// \todo better backpatch; this is a hard reset
						_heuristic.clear();
						_heuristic.push_back(eval_spec(_n_ary_op::linear_scan, 1));
						return true;
					}
				} while (0 <= --pivot);
			}
		} else {
			auto rhs_args = (lhs_rule_index == rhs_rule_index) ? lhs_args : interpreted.find(rhs_rule_index);
			if (interpreted.end() == rhs_args) {
				decltype(interpreted.begin()->second) staging;
				const auto origin = _x.begin();
				const auto end = _x.end();
				auto iter = _x.begin();
				do {
					auto test = rule.first.second(*iter);
					if (test.has_value()) staging.push_back(std::pair(iter - origin, test));
				} while (end != ++iter);
				interpreted[rhs_rule_index] = std::move(staging);
				rhs_args = interpreted.find(rhs_rule_index);
			}
			auto& rhs_seen = rhs_args->second;
			if (!lhs_seen.empty() && !rhs_seen.empty()) {
				ptrdiff_t anchor = rhs_seen.size() - 1;
				do {
					auto pivot = lhs_seen.size() - 1;
					do {
						if (rule.second.first(lhs_seen[pivot].second, rhs_seen[anchor].second)) {
							auto stage = std::unique_ptr<fp_API>(rule.second.second(_x[lhs_seen[pivot].first], _x[rhs_seen[anchor].first]));
							if (!stage) continue;
							apply_eval(_x, stage, lhs_seen[pivot].first, rhs_seen[anchor].first);
							// \todo better backpatch; this is a hard reset
							_heuristic.clear();
							_heuristic.push_back(eval_spec(_n_ary_op::linear_scan, 1));
							return true;
						}
					} while (0 <= --pivot);
				} while (0 <= --anchor);
			}
		}
	}
	return false;
}

bool sum::self_eval() {
	if (!this->_pre_self_eval()) return false;
	if (this->_self_eval(zaimoni::math::rearrange_sum, zaimoni::math::sum_score, zaimoni::math::sum_score, zaimoni::math::eval_sum)) return true;
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

std::optional<bool> sum::_is_finite() const {
	for (decltype(auto) x : this->_x) {
		if (const auto test = x->is_finite_kripke()) {
			if (!*test) return false;
		} else return std::nullopt;
	}
	return true;
}

void sum::_scal_bn(intmax_t scale) {
	for (auto& x : this->_x) x->scal_bn(scale);
}

} // namespace zaimoni

