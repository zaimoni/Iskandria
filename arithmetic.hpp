#ifndef ARITHMETIC_HPP
#define ARITHMETIC_HPP 1

#include "Zaimoni.STL/eval.hpp"
#include "Zaimoni.STL/numeric_error.hpp"
#include "Zaimoni.STL/flat_alg2.hpp"
#include "Zaimoni.STL/Logging.h"
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

std::shared_ptr<fp_API> operator+(const std::shared_ptr<fp_API>& lhs, const std::shared_ptr<fp_API>& rhs);
std::shared_ptr<fp_API>& operator+=(std::shared_ptr<fp_API>& lhs, const std::shared_ptr<fp_API>& rhs);

std::shared_ptr<fp_API> operator*(const std::shared_ptr<fp_API>& lhs, const std::shared_ptr<fp_API>& rhs);
std::shared_ptr<fp_API> operator/(const std::shared_ptr<fp_API>& lhs, const std::shared_ptr<fp_API>& rhs);

std::shared_ptr<fp_API> operator-(const std::shared_ptr<fp_API>& lhs);

std::shared_ptr<fp_API> pow(const std::shared_ptr<fp_API>& base, const std::shared_ptr<fp_API>& exponent);

}

#endif
