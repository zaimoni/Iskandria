#ifndef ARITHMETIC_HPP
#define ARITHMETIC_HPP 1

#include "Zaimoni.STL/eval.hpp"
#include <memory>
#include <vector>

namespace zaimoni {
namespace series {

template<class T>
class sum : public T
{
public:
	typedef std::shared_ptr<T> smart_ptr;	// unsure which type goes here
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
		if (src && !is_zero(*src)) {
			_x.push_back(src);
			_known_stable = false;
		}
	}
	void append_term(smart_ptr&& src) {
		if (src && !is_zero(*src)) {
			_x.push_back(src);
			_known_stable = false;
		}
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
};

template<class T>
class product : public T
{
public:
	typedef std::shared_ptr<T> smart_ptr;	// unsure which type goes here
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
		if (src && !is_one(*src)) {
			_x.push_back(src);
			_known_stable = false;
		}
	}
	void append_term(smart_ptr&& src) {
		if (src && !is_one(*src)) {
			_x.push_back(src);
			_known_stable = false;
		}
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
};

}	// end namespace series

template<class T>
class quotient : public T
{
public:
	typedef std::shared_ptr<T> smart_ptr;	// unsure which type goes here
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
		if (is_one(*_denominator)) return _numerator;
		return 0;
	}

	// \todo fp_API
private:
	const char* _constructor_fatal() const {
		if (!_numerator) return "numerator null";
		if (!_denominator) return "numerator null";
		if (isNaN(*_numerator)) return "numerator NAN";
		if (isNaN(*_denominator)) return "denominator NAN";
		if (is_zero(*_denominator)) return "zero denominator";
		return 0;
	}
};

}

#endif
