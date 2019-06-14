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
	virtual bool is_scal_bn_identity() const { return false; };	// let evaluation handle this
	virtual void scal_bn_safe_range(intmax_t& lb, intmax_t& ub) const {
		lb = std::numeric_limits<intmax_t>::min();
		ub = std::numeric_limits<intmax_t>::max();
		intmax_t _lb;
		intmax_t _ub;
		for(const auto& x : _x) {
			if (x->is_scal_bn_identity()) continue;
			x->scal_bn_safe_range(_lb, _ub);
			if (lb < _lb) lb = _lb;
			if (ub > _ub) ub = _ub;
		}
//		if (0 < lb) lb = 0;	// if we wanted to explicitly enforce invariants
//		if (0 > ub) ub = 0;
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
	virtual bool is_scal_bn_identity() const { return false; }	// let evaluation handle this -- pathological behavior anyway
	virtual void scal_bn_safe_range(intmax_t& lb, intmax_t& ub) const {
		lb = 0;
		ub = 0;
		if (_x.empty()) return;	// should have evaluated
		intmax_t _lb;
		intmax_t _ub;
		for(const auto& x : _x) {
			if (x->is_scal_bn_identity()) {
				lb = std::numeric_limits<intmax_t>::min();
				ub = std::numeric_limits<intmax_t>::max();
				return;
			}
			x->scal_bn_safe_range(_lb, _ub);
			bool lb_not_clamped = false;
			bool ub_not_clamped = false;
			if (0 > _lb) {
				if (lb_not_clamped = (std::numeric_limits<intmax_t>::min() - _lb < lb)) lb += _lb;
				else lb = std::numeric_limits<intmax_t>::min();
			} else lb_not_clamped = (std::numeric_limits<intmax_t>::min() < lb);
			if (0 < _ub) {
				if (ub_not_clamped = (std::numeric_limits<intmax_t>::max() - _ub > ub)) ub += _ub;
				else ub = std::numeric_limits<intmax_t>::max();
			} else ub_not_clamped = (std::numeric_limits<intmax_t>::max() > ub);
			if (!lb_not_clamped && !ub_not_clamped) return;
		}
		//		if (0 < lb) lb = 0;	// if we wanted to explicitly enforce invariants
		//		if (0 > ub) ub = 0;
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
		// \todo: scalBn of denominator towards 1
		_known_stable = true;
		return false;
	}
	smart_ptr eval() {
		if (_denominator->is_one()) return _numerator;
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
		if (_numerator.is_one() && _denominator.is_one()) return true;
		return false;
	}
	virtual bool is_scal_bn_identity() const { return false; };	// let evaluation handle this -- pathological behavior
	virtual void scal_bn_safe_range(intmax_t& lb, intmax_t& ub) const {
		if (_numerator->is_scal_bn_identity() || _denominator->is_scal_bn_identity()) {
			lb = std::numeric_limits<intmax_t>::min();
			ub = std::numeric_limits<intmax_t>::max();
			return;
		}
		_numerator->scal_bn_safe_range(lb, ub);
		intmax_t _lb;
		intmax_t _ub;
		_denominator->scal_bn_safe_range(_lb, _ub);
		if (0 > _lb && std::numeric_limits<intmax_t>::max() + _lb >= ub) ub -= _lb;
		else ub = std::numeric_limits<intmax_t>::max();
		if (0 < _ub && std::numeric_limits<intmax_t>::min() + _ub <= lb) lb -= _ub;
		else lb = std::numeric_limits<intmax_t>::min();
		//		if (0 < lb) lb = 0;	// if we wanted to explicitly enforce invariants
		//		if (0 > ub) ub = 0;
	};
	virtual fp_API* clone() const { return new quotient(*this); };
private:
	const char* _constructor_fatal() const {
		if (!_numerator) return "numerator null";
		if (!_denominator) return "numerator null";
		if (_denominator->is_zero()) return "zero denominator";
		if (_numerator->is_inf() && _denominator->is_inf()) return "infinity/infinity";
		return 0;
	}
};

}

#endif
