#ifndef ARITHMETIC_HPP
#define ARITHMETIC_HPP 1

#include "Zaimoni.STL/eval.hpp"
#include "Zaimoni.STL/numeric_error.hpp"
#include <memory>
#include <vector>
#include <map>

namespace zaimoni {

namespace math {

// prototype, establishing required syntax.  These are good candidates for explicit instantiation.
template<class T>
typename std::enable_if<std::is_base_of<fp_API, T>::value, int>::type rearrange_sum(std::shared_ptr<T>& lhs, std::shared_ptr<T>& rhs);

template<class T>
typename std::enable_if<std::is_base_of<fp_API, T>::value, int>::type rearrange_product(std::shared_ptr<T>& lhs, std::shared_ptr<T>& rhs);

template<class T>
typename std::enable_if<std::is_base_of<fp_API, T>::value, T*>::type eval_quotient(const std::shared_ptr<T>& n, const std::shared_ptr<T>& d);

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
};

// associative operations naturally are n-ary
template<class T>
class n_ary_op
{
public:
	typedef std::shared_ptr<T> smart_ptr;
	typedef std::pair<int, size_t> eval_spec;
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

	bool _self_eval(bool (*is_identity)(const fp_API*),int (*rearrange)(smart_ptr&, smart_ptr&),int (*fold_ok)(const smart_ptr&),int (*fold_score)(const smart_ptr&, const smart_ptr&),int (*fold)(smart_ptr&, smart_ptr&))
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
				const auto result_code = fold(_x[test.first.first], _x[test.first.second]);
				if (0 == result_code) {
					possible.erase(test.first);
					continue;
				}
				switch (result_code) {
				case -1:
					if (test.first.second < ub - 1) swap(_x[test.first.second], _x[ub - 1]);
					_x.erase(_x.begin() + test.first.first);
					_heuristic.push_back(eval_spec(_n_ary_op::linear_scan, ub - 2));
					return true;
				case 1:
					_x.erase(_x.begin() + test.first.second);
					if (test.first.first < ub - 2) swap(_x[test.first.first], _x[ub - 2]);
					_heuristic.push_back(eval_spec(_n_ary_op::linear_scan, ub - 2));
					return true;
//				case -2:	// mutual annihilation; should have been caught at rearranging stage
//				case 2:	// Haskell/F# folding doesn't leave both elements behind
				default: FATAL((std::string("unhandled fold result code ") + std::to_string(result_code)).c_str());
				}
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
			return false;
		}
		}
		return false;
	}

};

// T is assumed to require zaimoni::fp_API in all of these classes
template<class T>
class sum : public T, public _interface_of<sum<T>,std::shared_ptr<T>, T::API_code>, public eval_shared_ptr<T>, protected n_ary_op<T>
{
public:
	static_assert(std::is_base_of<fp_API, T>::value, "need fp_API as a base class");
	typedef typename n_ary_op<T>::smart_ptr smart_ptr;
	typedef typename n_ary_op<T>::eval_spec eval_spec;
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
		if (src->is_inf() && !_append_infinity(src)) return;	// mostly an annihilator
		this->_append_term(src);
	}
	void append_term(smart_ptr&& src) {
		if (!src || src->is_zero()) return;
		if (src->is_inf() && !_append_infinity(src)) return;	// mostly an annihilator
		this->_append_term(std::move(src));
	}
	void append_term(T* src) { append_term(smart_ptr(src)); }

private:
	virtual bool would_fpAPI_eval() const { return 1 >= this->_x.size(); }
public:
	// eval_shared_ptr
	virtual smart_ptr destructive_eval() {
		if (1 == this->_x.size()) return this->_x.front();
		return 0;
	}
	// fp_API
	virtual bool self_eval() {
		if (!this->_pre_self_eval()) return false;
		if (this->_self_eval(_n_ary_op::is_additive_identity,zaimoni::math::rearrange_sum, _n_ary_op::null_fold_ok, _n_ary_op::null_fold_score, _n_ary_op::null_rearrange)) return true;
		//	auto& checking = this->_heuristic.back();
		// \todo process our specific rules
		this->_heuristic.clear();
		return false;
	}
	virtual bool is_zero() const {
		if (this->_x.empty()) return true;
		if (1 == this->_x.size()) return this->_x.front()->is_zero();
		return false;
	}
	virtual bool is_one() const {
		if (1 == this->_x.size()) return this->_x.front()->is_one();
		return false;
	}
	virtual int sgn() const {
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
	virtual bool is_scal_bn_identity() const { return is_zero(); };	// let evaluation handle this, mostly
	virtual std::pair<intmax_t, intmax_t> scal_bn_safe_range() const {
		std::pair<intmax_t, intmax_t> ret(fp_API::max_scal_bn_safe_range());
		for(const auto& x : this->_x) {
			if (x->is_scal_bn_identity()) continue;
			const auto tmp = x->scal_bn_safe_range();
			if (ret.first < tmp.first) ret.first = tmp.first;
			if (ret.second > tmp.second) ret.second = tmp.second;
		}
		return ret;
	}
	virtual intmax_t ideal_scal_bn() const {
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
	}
	virtual sum* clone() const { return new sum(*this); }
	std::string to_s() const {
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
	virtual int precedence() const { return 1; }

	bool _is_inf() const {
		for (auto& x : this->_x) if (x->is_inf()) return true;
		return false;
	}
	bool _is_finite() const {
		for (auto& x : this->_x) if (!x->is_finite()) return false;
		return true;
	}
private:
	virtual void _scal_bn(intmax_t scale) {
		for (auto& x : this->_x) this->__scal_bn(x,scale);
	}
	virtual fp_API* _eval() const { return 0; }	// placeholder
};

template<class T>
class product : public T, public _interface_of<product<T>, std::shared_ptr<T>, T::API_code>, public eval_shared_ptr<T>, protected n_ary_op<T>
{
public:
	static_assert(std::is_base_of<fp_API, T>::value, "need fp_API as a base class");
	typedef typename n_ary_op<T>::smart_ptr smart_ptr;
	typedef typename n_ary_op<T>::eval_spec eval_spec;
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
		this->_append_term(src);
	}
	void append_term(smart_ptr&& src) {
		if (!src || src->is_one()) return;
		this->_append_term(std::move(src));
	}
	void append_term(T* src) { append_term(smart_ptr(src)); }

private:
	virtual bool would_fpAPI_eval() const { return 1 >= this->_x.size(); }
public:
	// eval_shared_ptr
	virtual smart_ptr destructive_eval() {
		if (1 == this->_x.size()) return this->_x.front();
		return 0;
	}
	// fp_API
	virtual bool self_eval() {
		if (!this->_pre_self_eval()) return false;
		if (this->_self_eval(_n_ary_op::is_multiplicative_identity, zaimoni::math::rearrange_product, _n_ary_op::null_fold_ok, _n_ary_op::null_fold_score, _n_ary_op::null_rearrange)) return true;
//		auto& checking = this->_heuristic.back();
		// \todo process our specific rules
		this->_heuristic.clear();
		return false;
	}
	virtual bool is_zero() const {
		if (1 == this->_x.size()) return this->_x.front()->is_zero();
		return false;
	}
	virtual bool is_one() const {
		if (this->_x.empty()) return true;
		if (1 == this->_x.size()) return this->_x.front()->is_one();
		return false;
	}
	virtual int sgn() const {
		if (this->_x.empty()) return 1;
		int ret = 1;
		for (auto& x : this->_x) {
			if (const auto test = x->sgn()) ret *= test;
			else return 0;
		}
		return ret;
	}
	virtual bool is_scal_bn_identity() const { return is_zero(); }	// let evaluation handle this -- pathological behavior anyway
	virtual std::pair<intmax_t, intmax_t> scal_bn_safe_range() const {
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
	virtual intmax_t ideal_scal_bn() const {
		if (is_scal_bn_identity() || is_one()) return 0;
		intmax_t ret = 0;
		for (const auto& x : this->_x) {
			if (x->is_scal_bn_identity()) return 0;
			clamped_sum_assign(ret, x->ideal_scal_bn());
			if (std::numeric_limits<intmax_t>::max() == ret || std::numeric_limits<intmax_t>::min() == ret) return ret;	// assumes in normal form
		}
		return ret;
	}
	virtual product* clone() const { return new product(*this); }
	std::string to_s() const {
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
	virtual int precedence() const { return 2; }

	bool _is_inf() const {
		for (auto& x : this->_x) if (x->is_inf()) return true;
		return false;
	}
	bool _is_finite() const {
		for (auto& x : this->_x) if (!x->is_finite()) return false;
		return true;
	}
private:
	virtual void _scal_bn(intmax_t scale) {
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
				this->__scal_bn(x, _scale);
				if (0 == (scale -= _scale)) return;
			} else if (0 > want && 0 > scale) {
				const auto _scale = (want > scale) ? want : scale;
				this->__scal_bn(x, _scale);
				if (0 == (scale -= _scale)) return;
			}
		};
		if (saw_identity) return;	// likely should not be happening
		for (auto& x : this->_x) {
			const auto legal = x->scal_bn_safe_range();
			if (0 < scale && 0 < legal.second) {
				const auto _scale = (legal.second < scale) ? legal.second : scale;
				this->__scal_bn(x, _scale);
				if (0 == (scale -= _scale)) return;
			} else if (0 > scale && 0 > legal.first) {
				const auto _scale = (legal.first > scale) ? legal.first : scale;
				this->__scal_bn(x, _scale);
				if (0 == (scale -= _scale)) return;
			}
		}
		if (0 != scale) throw zaimoni::math::numeric_error("scal_bn needed additional factors added");
	}
	virtual fp_API* _eval() const { return 0; }	// placeholder
};

template<class T>
class quotient : public T, public _interface_of<quotient<T>, std::shared_ptr<T>, T::API_code>, public eval_shared_ptr<T>
{
public:
	static_assert(std::is_base_of<fp_API, T>::value, "need fp_API as a base class");
	typedef std::shared_ptr<T> smart_ptr;
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
	quotient(const smart_ptr& numerator, T* denominator) : _numerator(numerator), _denominator(smart_ptr(denominator)), _heuristic(strict_max_heuristic - 1, 0) {
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
	quotient(smart_ptr&& numerator, T* denominator) : _numerator(std::move(numerator)), _denominator(smart_ptr(denominator)), _heuristic(strict_max_heuristic - 1, 0) {
		auto err = _constructor_fatal();
		if (err) throw zaimoni::math::numeric_error(err);
	}
	quotient(T* numerator, const smart_ptr& denominator) : _numerator(smart_ptr(numerator)), _denominator(denominator), _heuristic(strict_max_heuristic - 1, 0) {
		auto err = _constructor_fatal();
		if (err) throw zaimoni::math::numeric_error(err);
	}
	quotient(T* numerator, smart_ptr&& denominator) : _numerator(smart_ptr(numerator)), _denominator(std::move(denominator)), _heuristic(strict_max_heuristic - 1, 0) {
		auto err = _constructor_fatal();
		if (err) throw zaimoni::math::numeric_error(err);
	}
	quotient(T* numerator, T* denominator) : _numerator(smart_ptr(numerator)), _denominator(smart_ptr(denominator)), _heuristic(strict_max_heuristic - 1, 0) {
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
	virtual smart_ptr destructive_eval() {
		if (_denominator->is_one()) return _numerator;
		if (_numerator->is_zero()) return _numerator;
		return 0;
	}

	// fp_API
	virtual bool self_eval() {
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
	virtual bool is_zero() const {
		if (_numerator->is_zero()) return true;
		if (_denominator->is_inf()) return true;
		return false;
	}
	virtual bool is_one() const {
		if (_numerator == _denominator) return true;	// we assume that if two std::shared_ptrs are binary-equal that they are the same, even if they are intervals
		if (_numerator->is_one() && _denominator->is_one()) return true;
		return false;
	}
	virtual int sgn() const {
		const auto n_sgn = _numerator->sgn();
		if (const auto d_sgn = _denominator->sgn()) return n_sgn * d_sgn;
		else return n_sgn;	// division by zero hard-errors so more like "don't know"
	}
	virtual bool is_scal_bn_identity() const { return false; };	// let evaluation handle this -- pathological behavior
	virtual std::pair<intmax_t, intmax_t> scal_bn_safe_range() const {
		std::pair<intmax_t, intmax_t> ret(fp_API::max_scal_bn_safe_range());
		if (_numerator->is_scal_bn_identity() || _denominator->is_scal_bn_identity()) return ret;

		ret = _numerator->scal_bn_safe_range();
		const auto tmp = _denominator->scal_bn_safe_range();
		clamped_diff_assign(ret.second, tmp.second);
		clamped_diff_assign(ret.first, tmp.second);
		return ret;
	};
	virtual intmax_t ideal_scal_bn() const {
		if (_numerator->is_scal_bn_identity() || _denominator->is_scal_bn_identity()) return 0;
		intmax_t ret = _numerator->ideal_scal_bn();
		clamped_diff_assign(ret, _denominator->ideal_scal_bn());
		return ret;
	}
	virtual quotient* clone() const { return new quotient(*this); };
	virtual std::string to_s() const {
		auto n = _numerator->to_s();
		if (precedence() >= _numerator->precedence()) n = std::string("(") + n + ')';
		auto d = _denominator->to_s();
		if (precedence() >= _denominator->precedence()) d = std::string("(") + d + ')';
		return n + '/' + d;
	}
	virtual int precedence() const { return 2; }

	bool _is_inf() const {
		return _numerator->is_inf();	// cf. _transform_fatal which requires finite denominator in this case
	}
	bool _is_finite() const {
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
		return _transform_fatal(_numerator, _denominator);
	}
	virtual void _scal_bn(intmax_t scale) {
		auto numerator_scale = _numerator->ideal_scal_bn();
		auto denominator_scale = _denominator->ideal_scal_bn();
		// try to normalize the denominator first
		if (0 < scale && 0 > denominator_scale) {
			const auto _scale = (-scale > denominator_scale) ? -scale : denominator_scale;
			this->__scal_bn(_denominator,_scale);
			if (0 == (scale += _scale)) return;
			denominator_scale -= _scale;
		}
		if (0 < scale && 0 < numerator_scale) {
			const auto _scale = (scale < numerator_scale) ? scale : numerator_scale;
			this->__scal_bn(_numerator,_scale);
			if (0 == (scale -= _scale)) return;
			numerator_scale -= _scale;
		}
		if (0 > scale && 0 < denominator_scale) {
			const auto _scale = (-scale < denominator_scale) ? -scale : denominator_scale;
			this->__scal_bn(_denominator, _scale);
			if (0 == (scale += _scale)) return;
			denominator_scale -= _scale;
		}
		if (0 > scale && 0 > numerator_scale) {
			const auto _scale = (scale > numerator_scale) ? scale : numerator_scale;
			this->__scal_bn(_numerator, _scale);
			if (0 == (scale -= _scale)) return;
			numerator_scale -= _scale;
		}
		const auto legal = _numerator->scal_bn_safe_range();
		if (legal.first > scale) {
			this->__scal_bn(_numerator,legal.first);
			scale -= legal.first;
		} else if (legal.second < scale) {
			this->__scal_bn(_numerator,legal.second);
			scale -= legal.second;
		} else {
			this->__scal_bn(_numerator,scale);
			return;
		}
		this->__scal_bn(_denominator, -scale);
	}
	virtual fp_API* _eval() const { return zaimoni::math::eval_quotient(_numerator,_denominator); }
};

}

#endif
