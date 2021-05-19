#ifndef ARITHMETIC_HPP
#define ARITHMETIC_HPP 1

#include "Zaimoni.STL/eval.hpp"
#include "Zaimoni.STL/numeric_error.hpp"
#include "Zaimoni.STL/flat_alg2.hpp"
#include <memory>
#include <vector>
#include <map>
#include <optional>

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

}

// the following operations on fp_API types are closely related
// additive inverse
// left-multiplicative inverse
// right-multiplicative inverse
// scal_bn (power of 2 manipulation)

class symbolic_fp final : public fp_API, public eval_shared_ptr<fp_API> {
	std::shared_ptr<fp_API> dest;
	intmax_t scale_by;
	uintmax_t bitmap;	// anything smaller incurs padding bytes anyway
public:
	enum class op {
		inverse_add = 0,
		left_inverse_mult,
		right_inverse_mult
	};

	symbolic_fp(std::shared_ptr<fp_API>& src) noexcept : dest(src), scale_by(0), bitmap(0) {
		assert(src);
	}
	symbolic_fp(const symbolic_fp& src) = default;
	symbolic_fp(symbolic_fp&& src) = default;
	~symbolic_fp() = default;
	symbolic_fp& operator=(const symbolic_fp& src) = default;
	symbolic_fp& operator=(symbolic_fp&& src) = default;

	bool add_inverted() const { return bitmap & (1ULL << (int)op::inverse_add); }
	bool mult_inverted_left() const { return bitmap & (1ULL << (int)op::left_inverse_mult); }
	bool mult_inverted_right() const { return bitmap & (1ULL << (int)op::right_inverse_mult); }
	bool mult_inverted() const { return bitmap & ((1ULL << (int)op::left_inverse_mult) | (1ULL << (int)op::right_inverse_mult)); }

	void self_negate() {
		if (add_inverted()) bitmap &= ~(1ULL << (int)op::inverse_add);
		else bitmap |= 1ULL << (int)op::inverse_add;
	}

	bool self_square() {
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

	const math::type* domain() const override { return dest->domain(); }

	bool self_eval() override {
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
		bool ret = false;
		if (add_inverted() && zaimoni::math::in_place_negate(dest)) {
			bitmap &= ~(1ULL << (int)op::inverse_add);
			return true;
		}
		if (scale_by) {
			auto range = dest->scal_bn_safe_range();
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
		return ret;
	}

	bool is_zero() const override {
		if (mult_inverted()) return false;
		return dest->is_zero();
	}

	bool is_one() const override {
		if (add_inverted() || 0 != scale_by) return false;
		return dest->is_one();
	}

	int sgn() const override {
		if (dest->is_zero()) return 0;
		if (add_inverted() && domain()->is_totally_ordered()) return -dest->sgn();
		return dest->sgn();
	}

	bool is_scal_bn_identity() const override { return is_scal_bn_identity_default(); }

	std::pair<intmax_t, intmax_t> scal_bn_safe_range() const override {
		std::pair<intmax_t, intmax_t> ret(INTMAX_MIN, INTMAX_MAX);
		if (0 < scale_by) {
			ret.second -= scale_by;
		} else if (0 > scale_by) {
			ret.first -= scale_by;
		}
		return ret;
	}

private:
	std::optional<intmax_t> _scal_bn_is_unsafe(intmax_t scale) const
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

	intmax_t _probe_dest(intmax_t scale) const
	{
		if (mult_inverted()) {
			auto neg_scale = (-INTMAX_MAX <= scale) ? -scale : INTMAX_MAX;
			return -dest->scal_bn_is_safe(neg_scale);
		}
		else {
			return dest->scal_bn_is_safe(scale);
		}
	}

public:
	intmax_t scal_bn_is_safe(intmax_t scale) const override
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
		} else /* if (0 > error)*/ {
			if (*delta <= error) return scale;
			return *delta + probe;
		}
	}

	intmax_t ideal_scal_bn() const override {
		auto interior = dest->ideal_scal_bn();
		const bool tweaked = mult_inverted() && -INTMAX_MAX > interior;
		if (tweaked) ++interior;
		if (mult_inverted()) interior = -interior;

		if (0 <= scale_by && 0 >= interior) return scale_by + interior;
		if (0 >= scale_by && 0 <= interior) return scale_by + interior;
		if (0 < scale_by /* && 0 < interior */) return INTMAX_MAX - scale_by >= interior ? scale_by + interior : INTMAX_MAX;
		/* if (0 > scale_by && 0 > interior )*/ return INTMAX_MIN - scale_by <= interior ? scale_by + interior : INTMAX_MIN;
	}

	fp_API* clone() const override {
		if (0 == scale_by && !bitmap) return dest->clone();
		return new symbolic_fp(*this);
	}

	std::string to_s() const override {
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
	int precedence() const override {
		if (mult_inverted()) return 2;
		return 1;
	}

	void _scal_bn(intmax_t scale) override
	{
		if (0 < scale) {
			if (0 > scale_by || INTMAX_MAX - scale_by >= scale) scale_by += scale;
			else throw zaimoni::math::numeric_error("overflowed power-of-two scaling");
		} else if (0 > scale) {
			if (0 < scale_by || -INTMAX_MIN - scale_by <= scale) scale_by += scale;
			else throw zaimoni::math::numeric_error("overflowed power-of-two scaling");
		}
		self_eval();
	}

	std::shared_ptr<fp_API> destructive_eval() override
	{
		if (!scale_by && !bitmap) return dest;
		return nullptr;
	}

	fp_API* _eval() const override { return nullptr; }

	bool _is_inf() const override {
		if (mult_inverted()) return dest->is_zero();
		return dest->is_inf();
	}
	bool _is_finite() const override {
		if (mult_inverted()) return !dest->is_zero();
		return dest->is_finite();
	}
};

// hard-coded to * for now
class power_fp final : public fp_API, public eval_shared_ptr<fp_API> {
	std::shared_ptr<fp_API> base;
	std::shared_ptr<fp_API> exponent;
//	_type_spec::canonical_functions op;

public:
//	template<_type_spec::canonical_functions _op> // doesn't work -- uncallable?
	power_fp(std::shared_ptr<fp_API> x, std::shared_ptr<fp_API> y) noexcept : base(x), exponent(y) {}

	power_fp(const power_fp& src) = default;
	power_fp(power_fp&& src) = default;
	~power_fp() = default;
	power_fp& operator=(const power_fp& src) = default;
	power_fp& operator=(power_fp&& src) = default;

	const math::type* domain() const override {
		if (0 >= exponent->domain()->subclass(zaimoni::math::get<_type<_type_spec::_R_SHARP_>>())) {
			// Some sort of Dedekind cut construction involved.  Correct for +, *
			return base->domain();
		}
		throw zaimoni::math::numeric_error("unhandled domain() for power_fp");
	}

	bool self_square() {
		if (is_zero() || is_one()) return true; // fixed points
		if (exponent->is_scal_bn_identity()) return true;
		if (1 == exponent->scal_bn_is_safe(1)) {
			exponent->scal_bn(1);
			return true;
		}
		if (zaimoni::math::in_place_square(base)) return true;
		return false;
	}

	bool self_eval() override;

	bool is_zero() const override {
		// multiplication, for now
		if (base->is_zero()) return !exponent->is_zero();	// 0^0 is technically undefined
		return false;
	}

	bool is_one() const override {
		// multiplication, for now
		if (base->is_one()) return true;
		if (exponent->is_zero()) return !base->is_zero();	// 0^0 is technically undefined
		return false;
	}

	int sgn() const override {
		if (is_zero()) return 0;
		if (!domain()->is_totally_ordered()) return 1;
		throw zaimoni::math::numeric_error("unhandled sgn() for power_fp");
	}

	bool is_scal_bn_identity() const override { return is_scal_bn_identity_default(); }

	std::pair<intmax_t, intmax_t> scal_bn_safe_range() const override {
		return std::pair(0, 0);	 // \todo replace null implementation
	}

	intmax_t scal_bn_is_safe(intmax_t scale) const override {
		return 0;	 // \todo replace null implementation
	}

	intmax_t ideal_scal_bn() const override {
		return 0;	 // \todo replace null implementation
	}

	fp_API* clone() const override {
		return new power_fp(*this);
	}

	std::string to_s() const override {
		decltype(auto) ret(base->to_s());
		if (std::numeric_limits<int>::max() > base->precedence()) ret = "(" + ret + ")";
		ret += "<sup>"+exponent->to_s()+"</sup>";
		return ret;
	}

	// coordinate with the sum/product types
	int precedence() const override {
		return 3; // for multiplication
	}

	void _scal_bn(intmax_t scale) override {
		throw zaimoni::math::numeric_error("power_fp: unhandled power-of-two scaling");
	}

	std::shared_ptr<fp_API> destructive_eval() override;
	fp_API* _eval() const override;
};

struct _n_ary_op {
	enum {
		componentwise_evaluation = 1,
		remove_identity,
		linear_scan,
		fold,	// Haskell/F#; pairwise destructive evaluation
		strict_max_core_heuristic
	};

	static bool is_additive_identity(const fp_API* x) { return x->is_zero(); }
	static bool is_multiplicative_identity(const fp_API* x) { return x->is_one(); }
	// bridge support
	template<class T> static int null_rearrange(T& lhs, T& rhs) { return 0; }
	template<class T> static int null_fold_ok(const T&) { return std::numeric_limits<int>::min(); }
	template<class T> static int null_fold_score(const T& lhs, const T& rhs) { return std::numeric_limits<int>::min(); }
	template<class T> static T null_eval(const T& lhs, const T& rhs) { return 0; }
};

// associative operations naturally are n-ary
template<class T>
class n_ary_op
{
public:
	using smart_ptr = std::shared_ptr<T>;
	using eval_spec = std::pair<int, size_t>;
protected:
	std::vector<smart_ptr> _x;
	std::vector<eval_spec> _heuristic;

	n_ary_op() = default;
	n_ary_op(const n_ary_op& src) = default;
	n_ary_op(n_ary_op&& src) = default;
	~n_ary_op() = default;
	n_ary_op& operator=(const n_ary_op& src) = default;
	n_ary_op& operator=(n_ary_op&& src) = default;

	void _append_term(const smart_ptr& src) {
		if (!_x.empty()) {
			if (_heuristic.empty() || _n_ary_op::linear_scan != _heuristic.back().first) _heuristic.push_back(eval_spec(_n_ary_op::linear_scan, _x.size()));
		}
		_x.push_back(src);
	}

	void _append_term(smart_ptr&& src) {
		if (!_x.empty()) {
			if (_heuristic.empty() || _n_ary_op::linear_scan != _heuristic.back().first) _heuristic.push_back(eval_spec(_n_ary_op::linear_scan, _x.size()));
		}
		_x.push_back(std::move(src));
	}


	virtual bool would_fpAPI_eval() const = 0;

	bool _pre_self_eval()
	{
restart:
		if (_heuristic.empty()) return false;
		auto& checking = this->_heuristic.back();
		if (0 >= checking.first) return false;	// doing something else instead
		switch (checking.first)
		{
		case _n_ary_op::linear_scan:
			if (_x.size() <= checking.second || 1 > checking.second) {
				_heuristic.pop_back();
				goto restart;
			}
			return true;
		case _n_ary_op::remove_identity:
			if (_x.size() <= checking.second) {
				_heuristic.pop_back();
				goto restart;
			}
			return true;
		case _n_ary_op::componentwise_evaluation:
			if (_x.size() <= checking.second) {
				_heuristic.pop_back();
				goto restart;
			}
			return true;
		}
		return true;
	}

	bool _self_eval(bool (*is_identity)(const fp_API*),int (*rearrange)(smart_ptr&, smart_ptr&),int (*fold_ok)(const smart_ptr&),int (*fold_score)(const smart_ptr&, const smart_ptr&),smart_ptr (*fold)(const smart_ptr&, const smart_ptr&))
	{
restart:
		auto& checking = this->_heuristic.back();
		switch (checking.first)
		{
		case _n_ary_op::fold:
		{
			std::vector<size_t> legal;
			std::map<size_t, int> legal_scores;
			const size_t ub = _x.size();
			ptrdiff_t i = -1;
			while (ub > ++i) {
				auto score = fold_ok(_x[i]);
				if (std::numeric_limits<int>::min() < score) {
					legal.push_back(i);
					legal_scores[i] = score;
				}
			}
			const size_t legal_ub = legal.size();
			if (2 > legal_ub) {
				_heuristic.pop_back();
				if (_pre_self_eval()) goto restart;
				return false;
			}
			std::map<std::pair<size_t, size_t>, int> possible;
			i = 0;
			while (legal_ub > ++i) {
				ptrdiff_t j = -1;
				while (i > ++j) {
					auto score = fold_score(_x[legal[i]], _x[legal[j]]);
					if (std::numeric_limits<int>::min() < score) {
						bool ok = true;
						if (!possible.empty()) {
							for(const auto& x : possible) {
								if (x.second > score) {
									ok = false;
									break;
								} else if (x.second < score) {
									possible.clear();
									break;
								}
							}
						}
						if (ok) possible[std::pair<size_t, size_t>(j, i)] = score;
					}
				}
			}
			while(!possible.empty()) {
				const auto test = *possible.begin();
				auto result = fold(_x[test.first.first], _x[test.first.second]);
				if (!result) {
					possible.erase(test.first);
					continue;
				}
				_x.erase(_x.begin() + test.first.second);
				_x.erase(_x.begin() + test.first.first);
				_x.push_back(result);
				_heuristic.push_back(eval_spec(_n_ary_op::linear_scan, ub - 2));
				return true;
			}
			_heuristic.pop_back();
			if (_pre_self_eval()) goto restart;
			return false;
		}
		case _n_ary_op::linear_scan:
		{	// O(n^2) pairwise interaction checks (addition is always commutative)
			// \todo non-commutative version (blocks e.g. matrix multiplication)
		linear_scan_restart:
			if (_x.size() > checking.second && 1 <= checking.second) {
				auto& viewpoint = this->_x[checking.second];
				size_t anchor = checking.second;
				size_t i = 0;
				do {
					auto& target = _x[i];
					const auto result_code = rearrange(target, viewpoint);
					if (0 == result_code) continue;	// no interaction
					switch (result_code)
					{
					case -2:	// mutual annihilation; should be very rare
						{
						if (1 < checking.second) checking.second--;
						if (1 < checking.second) checking.second--;
						_heuristic.push_back(eval_spec(_n_ary_op::remove_identity, i));
						_heuristic.push_back(eval_spec(_n_ary_op::remove_identity, anchor));
						return true;
						}
					case -1:		// lhs annihilated.  assume mutual kill is reported as rhs
						{
						if (1 < checking.second) checking.second--;
						_heuristic.push_back(eval_spec(_n_ary_op::remove_identity, i));
						return true;
						}
					case 1:		// rhs annihilated (commutative operation may have swapped to ensure this).  Mutual kill should be very rare
						{
						if (1 < checking.second) checking.second--;
						if (is_identity(target.get())) _heuristic.push_back(eval_spec(_n_ary_op::remove_identity, i));
						else if (i + 1 < anchor) swap(target, _x[anchor - 1]);
						_heuristic.push_back(eval_spec(_n_ary_op::remove_identity, anchor));
						return true;
						}
					case 2:		// non-annihiliating change
						{
						if (i + 1 < checking.second) swap(target, _x[checking.second - 1]);
						if (1 < checking.second) checking.second--;
						return true;
						}
					}
				} while (++i < checking.second);
				// we fell through.  Everything at or below us does not interact (strong natural induction)
				checking.second++;
				goto linear_scan_restart;
			}
			_heuristic.pop_back();
			if (_heuristic.empty()) _heuristic.push_back(eval_spec(_n_ary_op::componentwise_evaluation, 0));
			goto restart;
		}
		case _n_ary_op::remove_identity:
			{
			const auto i = checking.second;
			_heuristic.pop_back();
			_x.erase(_x.begin() + i);
			}
			if (would_fpAPI_eval()) _heuristic.clear();
			return true;
		case _n_ary_op::componentwise_evaluation:
			{
			const auto strict_ub = _x.size();
			while (strict_ub > checking.second) {
				auto& viewpoint = _x[checking.second];
				if (viewpoint->self_eval()) {
					if (is_identity(viewpoint.get())) {
						_heuristic.push_back(eval_spec(_n_ary_op::remove_identity, checking.second));
						return true;
					}
					if (strict_ub - 1 > checking.second) swap(viewpoint, _x.back());
					_heuristic.push_back(eval_spec(_n_ary_op::linear_scan, strict_ub - 1));
					return true;
				}
				if (fp_API::eval(viewpoint)) {
					if (is_identity(viewpoint.get())) {
						_heuristic.push_back(eval_spec(_n_ary_op::remove_identity, checking.second));
						return true;
					}
					if (strict_ub - 1 > checking.second) swap(viewpoint, _x.back());
					_heuristic.push_back(eval_spec(_n_ary_op::linear_scan, strict_ub - 1));
					return true;
				}
				checking.second++;
			}
			_heuristic.pop_back();
			if (_pre_self_eval()) goto restart;
			if (_heuristic.empty()) {
				_heuristic.push_back(eval_spec(_n_ary_op::fold, 0));
				if (_pre_self_eval()) goto restart;
			}
			return false;
		}
		}
		return false;
	}
};

class sum : public fp_API, public eval_shared_ptr<fp_API>, protected n_ary_op<fp_API>
{
public:
	using raw_type = fp_API;
	using smart_ptr = n_ary_op<raw_type>::smart_ptr;
	using eval_spec = n_ary_op<raw_type>::eval_spec;
private:
	enum {
		strict_max_heuristic = _n_ary_op::strict_max_core_heuristic
	};
public:
	sum() = default;
	sum(const sum& src) = default;
	sum(sum&& src) = default;
	~sum() = default;
	sum& operator=(const sum& src) = default;
	sum& operator=(sum&& src) = default;

private:
	bool _append_infinity(const smart_ptr& src) {
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
public:
	void append_term(const smart_ptr& src) {
		if (!src || src->is_zero()) return;
		assert(src->domain());
		if (src->is_inf() && !_append_infinity(src)) return;	// mostly an annihilator
		this->_append_term(src);
	}
	void append_term(smart_ptr&& src) {
		if (!src || src->is_zero()) return;
		assert(src->domain());
		if (src->is_inf() && !_append_infinity(src)) return;	// mostly an annihilator
		this->_append_term(std::move(src));
	}
	void append_term(raw_type* src) { append_term(smart_ptr(src)); }

private:
	bool would_fpAPI_eval() const override { return 1 >= this->_x.size(); }
public:
	// eval_shared_ptr
	smart_ptr destructive_eval() override {
		if (1 == this->_x.size()) return this->_x.front();
		return 0;
	}
	// fp_API
	bool self_eval() override {
		if (!this->_pre_self_eval()) return false;
		if (this->_self_eval(_n_ary_op::is_additive_identity,zaimoni::math::rearrange_sum, zaimoni::math::sum_implemented, zaimoni::math::sum_score, zaimoni::math::eval_sum)) return true;
		//	auto& checking = this->_heuristic.back();
		// \todo process our specific rules
		this->_heuristic.clear();
		return false;
	}
	bool is_zero() const override {
		if (this->_x.empty()) return true;
		if (1 == this->_x.size()) return this->_x.front()->is_zero();
		return false;
	}
	bool is_one() const override {
		if (1 == this->_x.size()) return this->_x.front()->is_one();
		return false;
	}
	int sgn() const override {
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
	bool is_scal_bn_identity() const override { return is_zero(); };	// let evaluation handle this, mostly
	std::pair<intmax_t, intmax_t> scal_bn_safe_range() const override {
		std::pair<intmax_t, intmax_t> ret(fp_API::max_scal_bn_safe_range());
		for(const auto& x : this->_x) {
			if (x->is_scal_bn_identity()) continue;
			const auto tmp = x->scal_bn_safe_range();
			if (ret.first < tmp.first) ret.first = tmp.first;
			if (ret.second > tmp.second) ret.second = tmp.second;
		}
		return ret;
	}

	intmax_t scal_bn_is_safe(intmax_t scale) const override
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

	intmax_t ideal_scal_bn() const override {
		if (is_scal_bn_identity() || is_one()) return 0;
		intmax_t ret = 0;
		for (const auto& x : this->_x) {
			const auto test = x->ideal_scal_bn();
			if (0 < test) {
				if (0 > ret) return 0;
				if (0 < ret && test >= ret) continue;
				ret = test;
			} else if (0 > test) {
				if (0 < ret) return 0;
				if (0 > ret && test <= ret) continue;
				ret = test;
			} else return 0;
		}
		return ret;
	}
	const math::type* domain() const override
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

	fp_API* clone() const override { return new sum(*this); }

	std::string to_s() const override {
		if (this->_x.empty()) return "0";
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
				ret += '+' + tmp;
			}
		}
		return ret;
	}
	int precedence() const override { return 1; }

	bool _is_inf() const override {
		for (auto& x : this->_x) if (x->is_inf()) return true;
		return false;
	}
	bool _is_finite() const override {
		for (auto& x : this->_x) if (!x->is_finite()) return false;
		return true;
	}
private:
	void _scal_bn(intmax_t scale) override {
		for (auto& x : this->_x) self_scalBn(x, scale);
	}
	fp_API* _eval() const override { return 0; }	// placeholder
};

class product : public fp_API, public eval_shared_ptr<fp_API>, protected n_ary_op<fp_API>
{
public:
	using raw_type = fp_API;
	using smart_ptr = n_ary_op<raw_type>::smart_ptr;
	using eval_spec = n_ary_op<raw_type>::eval_spec;
private:
	enum {
		strict_max_heuristic = _n_ary_op::strict_max_core_heuristic
	};
public:
	product() = default;
	product(const product& src) = default;
	product(product&& src) = default;
	~product() = default;
	product& operator=(const product& src) = default;
	product& operator=(product&& src) = default;

	void append_term(const smart_ptr& src) {
		if (!src || src->is_one()) return;
		assert(src->domain());
		this->_append_term(src);
	}
	void append_term(smart_ptr&& src) {
		if (!src || src->is_one()) return;
		assert(src->domain());
		this->_append_term(std::move(src));
	}
	void append_term(raw_type* src) { append_term(smart_ptr(src)); }

private:
	bool would_fpAPI_eval() const override { return 1 >= this->_x.size(); }
public:
	// eval_shared_ptr
	smart_ptr destructive_eval() override {
		if (1 == this->_x.size()) return this->_x.front();
		return 0;
	}
	// fp_API
	bool self_eval() override {
		if (!this->_pre_self_eval()) return false;
		if (this->_self_eval(_n_ary_op::is_multiplicative_identity, zaimoni::math::rearrange_product, _n_ary_op::null_fold_ok, _n_ary_op::null_fold_score, _n_ary_op::null_eval)) return true;
//		auto& checking = this->_heuristic.back();
		// \todo process our specific rules
		this->_heuristic.clear();
		return false;
	}
	bool is_zero() const override {
		if (1 == this->_x.size()) return this->_x.front()->is_zero();
		return false;
	}
	bool is_one() const override {
		if (this->_x.empty()) return true;
		if (1 == this->_x.size()) return this->_x.front()->is_one();
		return false;
	}
	int sgn() const override {
		if (this->_x.empty()) return 1;
		int ret = 1;
		for (auto& x : this->_x) {
			if (const auto test = x->sgn()) ret *= test;
			else return 0;
		}
		return ret;
	}
	bool is_scal_bn_identity() const override { return is_zero(); }	// let evaluation handle this -- pathological behavior anyway
	std::pair<intmax_t, intmax_t> scal_bn_safe_range() const override {
		std::pair<intmax_t, intmax_t> ret(0, 0);
		if (this->_x.empty()) return ret;	// should have evaluated
		for(const auto& x : this->_x) {
			if (x->is_scal_bn_identity()) return fp_API::max_scal_bn_safe_range();
			const auto tmp = x->scal_bn_safe_range();
			clamped_sum_assign(ret.first,tmp.first);
			clamped_sum_assign(ret.second,tmp.second);
			if (std::numeric_limits<intmax_t>::min() >= ret.first && std::numeric_limits<intmax_t>::max() <= ret.second) return ret;
		}
		return ret;
	}

	intmax_t scal_bn_is_safe(intmax_t scale) const override {
		intmax_t to_account_for = scale;
		for (const auto& x : this->_x) {
			if (x->is_scal_bn_identity()) return scale;
			const auto probe = x->scal_bn_is_safe(to_account_for);
			if (probe == to_account_for) return scale;
			to_account_for -= probe;
		}
		return scale - to_account_for;
	}

	intmax_t ideal_scal_bn() const override {
		if (is_scal_bn_identity() || is_one()) return 0;
		intmax_t ret = 0;
		for (const auto& x : this->_x) {
			if (x->is_scal_bn_identity()) return 0;
			clamped_sum_assign(ret, x->ideal_scal_bn());
			if (std::numeric_limits<intmax_t>::max() == ret || std::numeric_limits<intmax_t>::min() == ret) return ret;	// assumes in normal form
		}
		return ret;
	}
	const math::type* domain() const override
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

	fp_API* clone() const override { return new product(*this); }

	std::string to_s() const override {
		if (this->_x.empty()) return "1";
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
			} else {
				ret += '*' + tmp;
			}
		}
		return ret;
	}
	int precedence() const override { return 2; }

	bool _is_inf() const override {
		for (auto& x : this->_x) if (x->is_inf()) return true;
		return false;
	}
	bool _is_finite() const override {
		for (auto& x : this->_x) if (!x->is_finite()) return false;
		return true;
	}
private:
	void _scal_bn(intmax_t scale) override {
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
			} else if (0 > want && 0 > scale) {
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
			} else if (0 > scale && 0 > legal.first) {
				const auto _scale = (legal.first > scale) ? legal.first : scale;
				self_scalBn(x, _scale);
				if (0 == (scale -= _scale)) return;
			}
		}
		if (0 != scale) throw zaimoni::math::numeric_error("scal_bn needed additional factors added");
	}
	fp_API* _eval() const override { return 0; }	// placeholder
};

class quotient : public fp_API, public eval_shared_ptr<fp_API>
{
public:
	using raw_type = fp_API;
	using smart_ptr = std::shared_ptr<raw_type>;
private:
	smart_ptr _numerator;
	smart_ptr _denominator;
	std::pair<unsigned int, unsigned int> _heuristic;
	enum {
		componentwise_evaluation = 1,
		strict_max_heuristic
	};
public:
	quotient() = default;
	quotient(const smart_ptr& numerator, const smart_ptr& denominator) : _numerator(numerator), _denominator(denominator), _heuristic(strict_max_heuristic-1,0) {
		auto err = _constructor_fatal();
		if (err) throw zaimoni::math::numeric_error(err);
	}
	quotient(const smart_ptr& numerator, smart_ptr&& denominator) : _numerator(numerator), _denominator(std::move(denominator)), _heuristic(strict_max_heuristic - 1, 0) {
		auto err = _constructor_fatal();
		if (err) throw zaimoni::math::numeric_error(err);
	}
	quotient(const smart_ptr& numerator, raw_type* denominator) : _numerator(numerator), _denominator(smart_ptr(denominator)), _heuristic(strict_max_heuristic - 1, 0) {
		auto err = _constructor_fatal();
		if (err) throw zaimoni::math::numeric_error(err);
	}
	quotient(smart_ptr&& numerator, const smart_ptr& denominator) : _numerator(std::move(numerator)), _denominator(denominator), _heuristic(strict_max_heuristic - 1, 0) {
		auto err = _constructor_fatal();
		if (err) throw zaimoni::math::numeric_error(err);
	}
	quotient(smart_ptr&& numerator, smart_ptr&& denominator) : _numerator(std::move(numerator)), _denominator(std::move(denominator)), _heuristic(strict_max_heuristic - 1, 0) {
		auto err = _constructor_fatal();
		if (err) throw zaimoni::math::numeric_error(err);
	}
	quotient(smart_ptr&& numerator, raw_type* denominator) : _numerator(std::move(numerator)), _denominator(smart_ptr(denominator)), _heuristic(strict_max_heuristic - 1, 0) {
		auto err = _constructor_fatal();
		if (err) throw zaimoni::math::numeric_error(err);
	}
	quotient(raw_type* numerator, const smart_ptr& denominator) : _numerator(smart_ptr(numerator)), _denominator(denominator), _heuristic(strict_max_heuristic - 1, 0) {
		auto err = _constructor_fatal();
		if (err) throw zaimoni::math::numeric_error(err);
	}
	quotient(raw_type* numerator, smart_ptr&& denominator) : _numerator(smart_ptr(numerator)), _denominator(std::move(denominator)), _heuristic(strict_max_heuristic - 1, 0) {
		auto err = _constructor_fatal();
		if (err) throw zaimoni::math::numeric_error(err);
	}
	quotient(raw_type* numerator, raw_type* denominator) : _numerator(smart_ptr(numerator)), _denominator(smart_ptr(denominator)), _heuristic(strict_max_heuristic - 1, 0) {
		auto err = _constructor_fatal();
		if (err) throw zaimoni::math::numeric_error(err);
	}
	quotient(const quotient& src) = default;
	quotient(quotient&& src) = default;
	~quotient() = default;
	quotient& operator=(const quotient& src) = default;
	quotient& operator=(quotient&& src) = default;

private:
	bool would_destructive_eval() const {
		if (_denominator->is_one()) return true;
		if (_numerator->is_zero()) return true;
		return false;
	}
public:
	// eval_shared_ptr
	smart_ptr destructive_eval() override {
		if (_denominator->is_one()) return _numerator;
		if (_numerator->is_zero()) return _numerator;
		return 0;
	}

	// fp_API
	bool self_eval() override {
		if (0 >= _heuristic.first) return false;
		// \todo: greatest common integer factor exceeds one
		// \todo: mutual cancellation of negative signs
		// \todo: scalBn of denominator towards 1 (arguably normal-form)
		switch (_heuristic.first) {
		case componentwise_evaluation:
			{
			unsigned int n_state = _heuristic.second % 3;	// chinese remainder theorem encoding
			unsigned int d_state = _heuristic.second / 3;
			switch (n_state) {
			case 0:
				if (_numerator->self_eval()) break;
				n_state = 1;
				// intentional fall-through
			case 1:
				n_state = fp_API::eval(_numerator) ? 0 : 2;
			};
			switch (d_state) {
			case 0:
				if (_denominator->self_eval()) break;
				d_state = 1;
				// intentional fall-through
			case 1:
				d_state = fp_API::eval(_denominator) ? 0 : 2;
			};
			if (8 > (_heuristic.second = 3 * d_state + n_state)) {
				if (auto msg = _transform_fatal(_numerator, _denominator)) throw zaimoni::math::numeric_error(msg);
				if (would_destructive_eval()) _heuristic.first = 0;
				return true;
			}
			}
			// intentional fall-through
		default:
			_heuristic.first = 0;
			return false;
		}
	}
	bool is_zero() const override {
		if (_numerator->is_zero()) return true;
		if (_denominator->is_inf()) return true;
		return false;
	}
	bool is_one() const override {
		if (_numerator == _denominator) return true;	// we assume that if two std::shared_ptrs are binary-equal that they are the same, even if they are intervals
		if (_numerator->is_one() && _denominator->is_one()) return true;
		return false;
	}
	int sgn() const override {
		const auto n_sgn = _numerator->sgn();
		if (const auto d_sgn = _denominator->sgn()) return n_sgn * d_sgn;
		else return n_sgn;	// division by zero hard-errors so more like "don't know"
	}
	bool is_scal_bn_identity() const override { return false; };	// let evaluation handle this -- pathological behavior
	std::pair<intmax_t, intmax_t> scal_bn_safe_range() const override {
		std::pair<intmax_t, intmax_t> ret(fp_API::max_scal_bn_safe_range());
		if (_numerator->is_scal_bn_identity() || _denominator->is_scal_bn_identity()) return ret;

		ret = _numerator->scal_bn_safe_range();
		const auto tmp = _denominator->scal_bn_safe_range();
		clamped_diff_assign(ret.second, tmp.second);
		clamped_diff_assign(ret.first, tmp.second);
		return ret;
	};

	intmax_t scal_bn_is_safe(intmax_t scale) const override
	{
		auto probe_numerator = _numerator->scal_bn_is_safe(scale);
		if (probe_numerator == scale) return scale;
		auto to_account_for = scale - probe_numerator;
		auto neg_to_account_for = (-INTMAX_MAX <= to_account_for ? -to_account_for : INTMAX_MAX);
		auto probe_denominator = -_denominator->scal_bn_is_safe(neg_to_account_for);
		if (probe_denominator == to_account_for) return scale;
		return probe_numerator + probe_denominator;
	}

	intmax_t ideal_scal_bn() const override {
		if (_numerator->is_scal_bn_identity() || _denominator->is_scal_bn_identity()) return 0;
		intmax_t ret = _numerator->ideal_scal_bn();
		clamped_diff_assign(ret, _denominator->ideal_scal_bn());
		return ret;
	}
	const math::type* domain() const override
	{
		decltype(auto) d_domain = _denominator->domain();
		if (d_domain) d_domain = d_domain->inverse(_type_spec::Multiplication);
		if (!d_domain) return nullptr;
		if (decltype(auto) n_domain = _numerator->domain()) {
			if (decltype(auto) ret = math::type::defined(*n_domain, _type_spec::Multiplication, *d_domain)) return ret;
		}
		return nullptr;
	}

	fp_API* clone() const override { return new quotient(*this); };

	std::string to_s() const override {
		auto n = _numerator->to_s();
		if (precedence() >= _numerator->precedence()) n = std::string("(") + n + ')';
		auto d = _denominator->to_s();
		if (precedence() >= _denominator->precedence()) d = std::string("(") + d + ')';
		return n + '/' + d;
	}
	int precedence() const override { return 2; }

	bool _is_inf() const override {
		return _numerator->is_inf();	// cf. _transform_fatal which requires finite denominator in this case
	}
	bool _is_finite() const override {
		if (_numerator->is_finite()) return true;
		else if (_numerator->is_inf()) return false;
		else if (_denominator->is_inf()) return true;	// presumed not-undefined
		return false;
	}
private:
	static const char* _transform_fatal(const smart_ptr& n, const smart_ptr& d)
	{
		if (d->is_zero()) return "zero denominator";
		if (n->is_inf() && d->is_inf()) return "infinity/infinity";
		return 0;
	}
	const char* _constructor_fatal() const {
		if (!_numerator) return "numerator null";
		if (!_denominator) return "denominator null";
		if (!domain()) return "division not defined";
		return _transform_fatal(_numerator, _denominator);
	}
	void _scal_bn(intmax_t scale) override {
		auto numerator_scale = _numerator->ideal_scal_bn();
		auto denominator_scale = _denominator->ideal_scal_bn();
		// try to normalize the denominator first
		if (0 < scale) {
			if (0 > denominator_scale) {
				const auto _scale = zaimoni::max(-scale, denominator_scale);
				self_scalBn(_denominator, _scale);
				if (0 == (scale += _scale)) return;
				denominator_scale -= _scale;
			}
			if (0 < numerator_scale) {
				const auto _scale = zaimoni::min(scale, numerator_scale);
				self_scalBn(_numerator, _scale);
				if (0 == (scale -= _scale)) return;
				numerator_scale -= _scale;
			}
		} else if (0 > scale) {
			if (0 < denominator_scale) {
				const auto _scale = zaimoni::min(-scale, denominator_scale);
				self_scalBn(_denominator, _scale);
				if (0 == (scale += _scale)) return;
				denominator_scale -= _scale;
			}
			if (0 > numerator_scale) {
				const auto _scale = zaimoni::max(scale, numerator_scale);
				self_scalBn(_numerator, _scale);
				if (0 == (scale -= _scale)) return;
				numerator_scale -= _scale;
			}
		}
		const auto legal = _numerator->scal_bn_safe_range();
		if (legal.first > scale) {
			self_scalBn(_numerator, legal.first);
			scale -= legal.first;
		} else if (legal.second < scale) {
			self_scalBn(_numerator, legal.second);
			scale -= legal.second;
		} else {
			self_scalBn(_numerator, scale);
			return;
		}
		self_scalBn(_denominator, -scale);
	}
	fp_API* _eval() const override { return zaimoni::math::eval_quotient(_numerator,_denominator); }
};

std::shared_ptr<fp_API> operator+(const std::shared_ptr<fp_API>& lhs, const std::shared_ptr<fp_API>& rhs);
std::shared_ptr<fp_API> operator*(const std::shared_ptr<fp_API>& lhs, const std::shared_ptr<fp_API>& rhs);
std::shared_ptr<fp_API> operator/(const std::shared_ptr<fp_API>& lhs, const std::shared_ptr<fp_API>& rhs);

std::shared_ptr<fp_API> operator-(const std::shared_ptr<fp_API>& lhs);

std::shared_ptr<fp_API> pow(const std::shared_ptr<fp_API>& base, const std::shared_ptr<fp_API>& exponent);

}

#endif
