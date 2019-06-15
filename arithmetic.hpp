#ifndef ARITHMETIC_HPP
#define ARITHMETIC_HPP 1

#include "Zaimoni.STL/eval.hpp"
#include <memory>
#include <vector>

namespace zaimoni {
namespace series {

// T is assumed to require zaimoni::fp_API in all of these classes
template<class T>
class sum : public T
{
public:
	typedef std::shared_ptr<T> smart_ptr;
private:
	std::vector<smart_ptr> _x;
	bool _known_stable;
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
		_known_stable = false;
	}
	void append_term(smart_ptr&& src) {
		if (!src || src->is_zero()) return;
		_x.push_back(src);	// \todo react to infinity
		_known_stable = false;
	}

	bool self_eval() {
		if (_known_stable) return false;
		_known_stable = true;
		return false;
	}
	smart_ptr eval() {
		if (1 == _x.size()) return _x.front();
		return 0;
	}
	// \todo fp_API
	virtual bool is_zero() const { 
		if (_x.empty()) return true;
		if (1 == _x.size()) return _x.front()->is_zero();
		return false;
	}
	virtual bool is_one() const {
		if (1 == _x.size()) return _x.front()->is_one();
		return false;
	}
	virtual bool is_scal_bn_identity() const { return is_zero(); };	// let evaluation handle this, mostly
	virtual std::pair<intmax_t, intmax_t> scal_bn_safe_range() const {
		std::pair<intmax_t, intmax_t> ret(std::numeric_limits<intmax_t>::min(), std::numeric_limits<intmax_t>::max());
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
	virtual fp_API* clone() const { return new sum(*this); }
};

template<class T>
class product : public T
{
public:
	typedef std::shared_ptr<T> smart_ptr;
private:
	std::vector<smart_ptr> _x;
	bool _known_stable;
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
		_known_stable = false;
	}
	void append_term(smart_ptr&& src) {
		if (!src || src->is_one()) return;
		_x.push_back(src);	// \todo react to 0, infinity
		_known_stable = false;
	}

	bool self_eval() {
		if (_known_stable) return false;
		_known_stable = true;
		return false;
	}
	smart_ptr eval() {
		if (1 == _x.size()) return _x.front();
		return 0;
	}
	// \todo fp_API
	virtual bool is_zero() const {
		if (1 == _x.size()) return _x.front()->is_zero();
		return false;
	}
	virtual bool is_one() const {
		if (_x.empty()) return true;
		if (1 == _x.size()) return _x.front()->is_one();
		return false;
	}
	virtual bool is_scal_bn_identity() const { return is_zero(); }	// let evaluation handle this -- pathological behavior anyway
	virtual std::pair<intmax_t, intmax_t> scal_bn_safe_range() const {
		std::pair<intmax_t, intmax_t> ret(0, 0);
		if (_x.empty()) return ret;	// should have evaluated
		for(const auto& x : _x) {
			if (x->is_scal_bn_identity()) return std::pair<intmax_t, intmax_t>(std::numeric_limits<intmax_t>::min(), std::numeric_limits<intmax_t>::max());
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
	virtual fp_API* clone() const { return new product(*this); }
};

}	// end namespace series

template<class T>
class quotient : public T
{
public:
	typedef std::shared_ptr<T> smart_ptr;
private:
	smart_ptr _numerator;
	smart_ptr _denominator;
	bool _known_stable;
public:
	quotient() = default;
	quotient(smart_ptr&& numerator, smart_ptr&& denominator) : _numerator(numerator), _denominator(denominator), _known_stable(false){
		auto err = _constructor_fatal();
		if (err) throw std::runtime_error(err);	// might want the numeric error class instead
	}
	quotient(const quotient& src) = default;
	quotient(quotient&& src) = default;
	~quotient() = default;
	quotient& operator=(const quotient& src) = default;
	quotient& operator=(quotient&& src) = default;

	bool self_eval() {
		if (_known_stable) return false;
		// \todo: greatest common integer factor exceeds one
		// \todo: mutual cancellation of negative signs
		// \todo: scalBn of denominator towards 1 (arguably normal-form)
		_known_stable = true;
		return false;
	}
	smart_ptr eval() {
		if (_denominator->is_one()) return _numerator;
		if (_numerator->is_zero()) return _numerator;
		return 0;
	}

	// \todo fp_API
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
	virtual bool is_scal_bn_identity() const { return false; };	// let evaluation handle this -- pathological behavior
	virtual std::pair<intmax_t, intmax_t> scal_bn_safe_range() const {
		std::pair<intmax_t, intmax_t> ret(std::numeric_limits<intmax_t>::min(), std::numeric_limits<intmax_t>::max());
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
	virtual fp_API* clone() const { return new quotient(*this); };
private:
	const char* _constructor_fatal() const {
		if (!_numerator) return "numerator null";
		if (!_denominator) return "denominator null";
		if (_denominator->is_zero()) return "zero denominator";
		if (_numerator->is_inf() && _denominator->is_inf()) return "infinity/infinity";
		return 0;
	}
	static bool _scal_bn(smart_ptr& dest,intmax_t scale) {
		smart_ptr working = dest->unique() ? dest : dest->clone();
		if (!working->scal_bn(scale)) return false;
		dest = working;
		return true;
	}
	virtual bool _scal_bn(intmax_t scale) {
		auto numerator_scale = _numerator->ideal_scal_bn();
		auto denominator_scale = _denominator->ideal_scal_bn();
		// try to normalize the denominator first
		if (0 < scale && 0 > denominator_scale) {
			const auto _scale = (-scale > denominator_scale) ? -scale : denominator_scale;
			if (!_scal_bn(_denominator,_scale)) return false;	// likely invariant error
			if (0 == (scale += _scale)) return true;
			denominator_scale -= _scale;
		}
		if (0 < scale && 0 < numerator_scale) {
			const auto _scale = (scale < numerator_scale) ? scale : numerator_scale;
			if (!_scal_bn(_numerator,_scale)) return false;	// likely invariant error
			if (0 == (scale -= _scale)) return true;
			numerator_scale -= _scale;
		}
		if (0 > scale && 0 < denominator_scale) {
			const auto _scale = (-scale < denominator_scale) ? -scale : denominator_scale;
			if (!_scal_bn(_denominator, _scale)) return false;	// likely invariant error
			if (0 == (scale += _scale)) return true;
			denominator_scale -= _scale;
		}
		if (0 > scale && 0 > numerator_scale) {
			const auto _scale = (scale > numerator_scale) ? scale : numerator_scale;
			if (!_scal_bn(_numerator, _scale)) return false;	// likely invariant error
			if (0 == (scale -= _scale)) return true;
			numerator_scale -= _scale;
		}
		const auto legal = _numerator->scal_bn_safe_range();
		const auto legal_denominator = _denominator->scal_bn_safe_range();
		if (legal.first > scale) {
			if (!_scal_bn(_numerator,legal.first)) return false;
			scale -= legal.first;
		} else if (legal.second < scale) {
			if (!_scal_bn(_numerator,legal.second)) return false;
			scale -= legal.second;
		} else return _scal_bn(_numerator,scale);
		if (legal_denominator.first > scale) return false;	// likely invariant error
		else if (legal_denominator.second < scale) return false;	// likely invariant error
		else return _scal_bn(_denominator, scale);
	}

};

}

#endif
