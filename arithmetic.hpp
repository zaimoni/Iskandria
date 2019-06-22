#ifndef ARITHMETIC_HPP
#define ARITHMETIC_HPP 1

#include "Zaimoni.STL/eval.hpp"
#include <memory>
#include <vector>

namespace zaimoni {
namespace series {

// T is assumed to require zaimoni::fp_API in all of these classes
template<class T>
class sum : public T, public _interface_of<sum<T>,std::shared_ptr<T>, T::API_code>, public eval_shared_ptr<T>
{
public:
	typedef std::shared_ptr<T> smart_ptr;
private:
	std::vector<smart_ptr> _x;
	unsigned int _heuristic_code;
	enum {
		componentwise_evaluation = 1,
		strict_max_heuristic
	};
public:
	sum() = default;
	sum(const sum& src) = default;
	sum(sum&& src) = default;
	~sum() = default;
	sum& operator=(const sum& src) = default;
	sum& operator=(sum&& src) = default;

	void append_term(const smart_ptr& src) {
		if (!src || src->is_zero()) return;
		_x.push_back(src);	// \todo react to infinity
		_heuristic_code = strict_max_heuristic - 1;
	}
	void append_term(smart_ptr&& src) {
		if (!src || src->is_zero()) return;
		_x.push_back(std::move(src));	// \todo react to infinity
		_heuristic_code = strict_max_heuristic - 1;
	}
	void append_term(T* src) { append_term(smart_ptr(src)); }

	// eval_shared_ptr
	virtual smart_ptr destructive_eval() {
		if (1 == _x.size()) return _x.front();
		return 0;
	}
	// fp_API
	virtual bool self_eval() {
		if (0 >= _heuristic_code) return false;
		_heuristic_code = 0;
		return false;
	}
	virtual bool is_zero() const {
		if (_x.empty()) return true;
		if (1 == _x.size()) return _x.front()->is_zero();
		return false;
	}
	virtual bool is_one() const {
		if (1 == _x.size()) return _x.front()->is_one();
		return false;
	}
	virtual int sgn() const {
		if (is_zero()) return 0;
		unsigned int seen = 0;
		for (auto& x : _x) {
			if (x->is_zero()) continue;	// should have normalized this away
			const auto test = x->sgn();
			if (7U == (seen |= (1 << (test + 1)))) break;
		}
		if (0 == seen) return 0;
		if (4 == seen) return 1;	// only saw positive
		if (1 == seen) return -1;	// only saw negative
		// anything else would need evaluation to get right
		throw std::runtime_error("sum needs to evaluate enough to calculate sgn()");
	}
	virtual bool is_scal_bn_identity() const { return is_zero(); };	// let evaluation handle this, mostly
	virtual std::pair<intmax_t, intmax_t> scal_bn_safe_range() const {
		std::pair<intmax_t, intmax_t> ret(fp_API::max_scal_bn_safe_range());
		for(const auto& x : _x) {
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
		for (const auto& x : _x) {
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
		if (_x.empty()) return "0";
		const auto _size = _x.size();
		if (1 == _size) return _x.front()->to_s();
		bool first = true;
		std::string ret;
		for (auto& x : _x) {
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
		for (auto& x : _x) if (x->is_inf()) return true;
		return false;
	}
	bool _is_finite() const {
		for (auto& x : _x) if (!x->is_finite()) return false;
		return true;
	}
private:
	virtual void _scal_bn(intmax_t scale) {
		for (auto& x : _x) this->__scal_bn(x,scale);
	}
	virtual fp_API* _eval() const { return 0; }	// placeholder
};

template<class T>
class product : public T, public _interface_of<product<T>, std::shared_ptr<T>, T::API_code>, public eval_shared_ptr<T>
{
public:
	typedef std::shared_ptr<T> smart_ptr;
private:
	std::vector<smart_ptr> _x;
	unsigned int _heuristic_code;
	enum {
		componentwise_evaluation = 1,
		strict_max_heuristic
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
		_x.push_back(src);	// \todo react to 0, infinity
		_heuristic_code = strict_max_heuristic - 1;
	}
	void append_term(smart_ptr&& src) {
		if (!src || src->is_one()) return;
		_x.push_back(std::move(src));	// \todo react to 0, infinity
		_heuristic_code = strict_max_heuristic - 1;
	}
	void append_term(T* src) { append_term(smart_ptr(src)); }

	// eval_shared_ptr
	virtual smart_ptr destructive_eval() {
		if (1 == _x.size()) return _x.front();
		return 0;
	}
	// fp_API
	virtual bool self_eval() {
		if (0>= _heuristic_code) return false;
		_heuristic_code = 0;
		return false;
	}
	virtual bool is_zero() const {
		if (1 == _x.size()) return _x.front()->is_zero();
		return false;
	}
	virtual bool is_one() const {
		if (_x.empty()) return true;
		if (1 == _x.size()) return _x.front()->is_one();
		return false;
	}
	virtual int sgn() const {
		if (_x.empty()) return 1;
		int ret = 1;
		for (auto& x : _x) {
			if (const auto test = x->sgn()) ret *= test;
			else return 0;
		}
		return ret;
	}
	virtual bool is_scal_bn_identity() const { return is_zero(); }	// let evaluation handle this -- pathological behavior anyway
	virtual std::pair<intmax_t, intmax_t> scal_bn_safe_range() const {
		std::pair<intmax_t, intmax_t> ret(0, 0);
		if (_x.empty()) return ret;	// should have evaluated
		for(const auto& x : _x) {
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
		for (const auto& x : _x) {
			if (x->is_scal_bn_identity()) return 0;
			clamped_sum_assign(ret, x->ideal_scal_bn());
			if (std::numeric_limits<intmax_t>::max() == ret || std::numeric_limits<intmax_t>::min() == ret) return ret;	// assumes in normal form
		}
		return ret;
	}
	virtual product* clone() const { return new product(*this); }
	std::string to_s() const {
		if (_x.empty()) return "1";
		const auto _size = _x.size();
		if (1 == _size) return _x.front()->to_s();
		bool first = true;
		std::string ret;
		for (auto& x : _x) {
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
		for (auto& x : _x) if (x->is_inf()) return true;
		return false;
	}
	bool _is_finite() const {
		for (auto& x : _x) if (!x->is_finite()) return false;
		return true;
	}
private:
	virtual void _scal_bn(intmax_t scale) {
		bool saw_identity = false;
		// \todo both of these loops can be specialized (scale positive/negative will be invariant)
		for (auto& x : _x) {
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
		for (auto& x : _x) {
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
		if (0 != scale) throw std::runtime_error("scal_bn needed additional factors added");
	}
	virtual fp_API* _eval() const { return 0; }	// placeholder
};

}	// end namespace series

template<class T>
class quotient : public T, public _interface_of<quotient<T>, std::shared_ptr<T>, T::API_code>, public eval_shared_ptr<T>
{
public:
	typedef std::shared_ptr<T> smart_ptr;
private:
	smart_ptr _numerator;
	smart_ptr _denominator;
	unsigned int _heuristic_code;
	enum {
		componentwise_evaluation = 1,
		strict_max_heuristic
	};
public:
	quotient() = default;
	quotient(const smart_ptr& numerator, const smart_ptr& denominator) : _numerator(numerator), _denominator(denominator), _heuristic_code(strict_max_heuristic-1){
		auto err = _constructor_fatal();
		if (err) throw std::runtime_error(err);	// might want the numeric error class instead
	}
	quotient(const smart_ptr& numerator, smart_ptr&& denominator) : _numerator(numerator), _denominator(std::move(denominator)), _heuristic_code(strict_max_heuristic - 1) {
		auto err = _constructor_fatal();
		if (err) throw std::runtime_error(err);	// might want the numeric error class instead
	}
	quotient(const smart_ptr& numerator, T* denominator) : _numerator(numerator), _denominator(smart_ptr(denominator)), _heuristic_code(strict_max_heuristic - 1) {
		auto err = _constructor_fatal();
		if (err) throw std::runtime_error(err);	// might want the numeric error class instead
	}
	quotient(smart_ptr&& numerator, const smart_ptr& denominator) : _numerator(std::move(numerator)), _denominator(denominator), _heuristic_code(strict_max_heuristic - 1) {
		auto err = _constructor_fatal();
		if (err) throw std::runtime_error(err);	// might want the numeric error class instead
	}
	quotient(smart_ptr&& numerator, smart_ptr&& denominator) : _numerator(std::move(numerator)), _denominator(std::move(denominator)), _heuristic_code(strict_max_heuristic - 1) {
		auto err = _constructor_fatal();
		if (err) throw std::runtime_error(err);	// might want the numeric error class instead
	}
	quotient(smart_ptr&& numerator, T* denominator) : _numerator(std::move(numerator)), _denominator(smart_ptr(denominator)), _heuristic_code(strict_max_heuristic - 1) {
		auto err = _constructor_fatal();
		if (err) throw std::runtime_error(err);	// might want the numeric error class instead
	}
	quotient(T* numerator, const smart_ptr& denominator) : _numerator(smart_ptr(numerator)), _denominator(denominator), _heuristic_code(strict_max_heuristic - 1) {
		auto err = _constructor_fatal();
		if (err) throw std::runtime_error(err);	// might want the numeric error class instead
	}
	quotient(T* numerator, smart_ptr&& denominator) : _numerator(smart_ptr(numerator)), _denominator(std::move(denominator)), _heuristic_code(strict_max_heuristic - 1) {
		auto err = _constructor_fatal();
		if (err) throw std::runtime_error(err);	// might want the numeric error class instead
	}
	quotient(T* numerator, T* denominator) : _numerator(smart_ptr(numerator)), _denominator(smart_ptr(denominator)), _heuristic_code(strict_max_heuristic - 1) {
		auto err = _constructor_fatal();
		if (err) throw std::runtime_error(err);	// might want the numeric error class instead
	}
	quotient(const quotient& src) = default;
	quotient(quotient&& src) = default;
	~quotient() = default;
	quotient& operator=(const quotient& src) = default;
	quotient& operator=(quotient&& src) = default;

	// eval_shared_ptr
	virtual smart_ptr destructive_eval() {
		if (_denominator->is_one()) return _numerator;
		if (_numerator->is_zero()) return _numerator;
		return 0;
	}

	// fp_API
	virtual bool self_eval() {
		if (0 >= _heuristic_code) return false;
		// \todo: greatest common integer factor exceeds one
		// \todo: mutual cancellation of negative signs
		// \todo: scalBn of denominator towards 1 (arguably normal-form)
		_heuristic_code = 0;
		return false;
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
	virtual fp_API* _eval() const { return 0; }	// placeholder
};

}

#endif
