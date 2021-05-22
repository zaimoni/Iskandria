#include "product.hpp"

namespace zaimoni {

void product::append_term(const smart_ptr& src) {
	if (!src || src->is_one()) return;
	assert(src->domain());
	this->_append_term(src);
}

void product::append_term(smart_ptr&& src) {
	if (!src || src->is_one()) return;
	assert(src->domain());
	this->_append_term(std::move(src));
}

bool product::would_fpAPI_eval() const { return 1 >= this->_x.size(); }

	// eval_shared_ptr
product::smart_ptr product::destructive_eval() {
	if (1 == this->_x.size()) return this->_x.front();
	return 0;
}

// fp_API
bool product::self_eval() {
	if (!this->_pre_self_eval()) return false;
	if (this->_self_eval(_n_ary_op::is_multiplicative_identity, zaimoni::math::rearrange_product, _n_ary_op::null_fold_ok, _n_ary_op::null_fold_score, _n_ary_op::null_eval)) return true;
	//		auto& checking = this->_heuristic.back();
	// \todo process our specific rules
	this->_heuristic.clear();
	return false;
}

bool product::is_zero() const {
	if (1 == this->_x.size()) return this->_x.front()->is_zero();
	return false;
}

bool product::is_one() const {
	if (this->_x.empty()) return true;
	if (1 == this->_x.size()) return this->_x.front()->is_one();
	return false;
}

int product::sgn() const {
	if (this->_x.empty()) return 1;
	int ret = 1;
	for (decltype(auto) x: this->_x) {
		if (const auto test = x->sgn()) ret *= test;
		else return 0;
	}
	return ret;
}

std::pair<intmax_t, intmax_t> product::scal_bn_safe_range() const {
	std::pair<intmax_t, intmax_t> ret(0, 0);
	if (this->_x.empty()) return ret;	// should have evaluated
	for (const auto& x : this->_x) {
		if (x->is_scal_bn_identity()) return fp_API::max_scal_bn_safe_range();
		const auto tmp = x->scal_bn_safe_range();
		clamped_sum_assign(ret.first, tmp.first);
		clamped_sum_assign(ret.second, tmp.second);
		if (std::numeric_limits<intmax_t>::min() >= ret.first && std::numeric_limits<intmax_t>::max() <= ret.second) return ret;
	}
	return ret;
}

intmax_t product::scal_bn_is_safe(intmax_t scale) const {
	intmax_t to_account_for = scale;
	for (const auto& x : this->_x) {
		if (x->is_scal_bn_identity()) return scale;
		const auto probe = x->scal_bn_is_safe(to_account_for);
		if (probe == to_account_for) return scale;
		to_account_for -= probe;
	}
	return scale - to_account_for;
}

intmax_t product::ideal_scal_bn() const {
	if (is_scal_bn_identity() || is_one()) return 0;
	intmax_t ret = 0;
	for (const auto& x : this->_x) {
		if (x->is_scal_bn_identity()) return 0;
		clamped_sum_assign(ret, x->ideal_scal_bn());
		if (std::numeric_limits<intmax_t>::max() == ret || std::numeric_limits<intmax_t>::min() == ret) return ret;	// assumes in normal form
	}
	return ret;
}

const math::type* product::domain() const
{
	if (_x.empty()) return &math::get<_type<_type_spec::_R_SHARP_>>(); // omni-one is unconstrained \todo should be integers
	std::vector<decltype(domain())> accumulator;
	for (decltype(auto) arg : _x) {
		decltype(domain()) test = arg->domain();
		if (!test) continue; // \todo invariant violation
		if (accumulator.empty()) {
			accumulator.push_back(test);
			continue;
		}
		if (auto op_type = math::type::defined(*accumulator.back(), _type_spec::Multiplication, *test)) {
			accumulator.back() = op_type;
			continue;
		}
		throw std::logic_error("unhandled product domain"); // \todo could push_back and try to recover later
	}
	const size_t ub = accumulator.size();
	if (0 == ub) return nullptr;
	else if (1 == ub) return accumulator.front();
	else throw std::logic_error("unhandled product domain");
}

std::string product::to_s() const {
	if (this->_x.empty()) return "1"; // \todo delegate to domain()
	const auto _size = this->_x.size();
	if (1 == _size) return this->_x.front()->to_s();
	bool first = true;
	std::string ret;
	for (auto& x : this->_x) {
		auto tmp = x->to_s();
		if (precedence() >= x->precedence()) tmp = std::string("(") + tmp + ')';
		if (first) {
			ret = std::move(tmp);
			first = false;
		}
		else {
			ret += '*' + tmp;
		}
	}
	return ret;
}

bool product::_is_inf() const {
	for (auto& x : this->_x) if (x->is_inf()) return true;
	return false;
}

bool product::_is_finite() const {
	for (auto& x : this->_x) if (!x->is_finite()) return false;
	return true;
}

void product::_scal_bn(intmax_t scale) {
	bool saw_identity = false;
	// \todo both of these loops can be specialized (scale positive/negative will be invariant)
	for (auto& x : this->_x) {
		if (x->is_scal_bn_identity()) {
			saw_identity = true;
			continue;
		}
		const auto want = x->ideal_scal_bn();
		if (0 < want && 0 < scale) {
			const auto _scale = (want < scale) ? want : scale;
			self_scalBn(x, _scale);
			if (0 == (scale -= _scale)) return;
		}
		else if (0 > want && 0 > scale) {
			const auto _scale = (want > scale) ? want : scale;
			self_scalBn(x, _scale);
			if (0 == (scale -= _scale)) return;
		}
	};
	if (saw_identity) return;	// likely should not be happening
	for (auto& x : this->_x) {
		const auto legal = x->scal_bn_safe_range();
		if (0 < scale && 0 < legal.second) {
			const auto _scale = (legal.second < scale) ? legal.second : scale;
			self_scalBn(x, _scale);
			if (0 == (scale -= _scale)) return;
		}
		else if (0 > scale && 0 > legal.first) {
			const auto _scale = (legal.first > scale) ? legal.first : scale;
			self_scalBn(x, _scale);
			if (0 == (scale -= _scale)) return;
		}
	}
	if (0 != scale) throw zaimoni::math::numeric_error("scal_bn needed additional factors added");
}

} // namespace zaimoni
