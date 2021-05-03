#ifndef ARY1_HPP
#define ARY1_HPP

#include <vector>
#include <functional>
#include "Zaimoni.STL/Logging.h"

namespace zaimoni {
namespace math {

template<class Range, class...Args>
class func {
	unsigned long _code;
	static std::vector<std::function<Range(Args...)>> _funcs;

public:
	static auto declare(std::function<Range(Args...)> forward) {
		_funcs.push_back(forward);
		return _funcs.size();
	}

	func(unsigned long code = 0) noexcept : _code(code) {
		assert(_funcs.size() > code);
	}
	func(const func& src) = default;
	func(func&& src) = default;
	~func() = default;
	func& operator=(const func& src) = default;
	func& operator=(func&& src) = default;

	auto code() const { return _code; }

};

template<class Domain, class Range>
class ary1
{
	unsigned long _code;
	static std::vector<std::function<Range(Domain)>> _funcs;
	static std::vector<std::function<Domain(Range)>> _inv_funcs;

public:
	static auto declare(std::function<Range(Domain)> forward, std::function<Domain(Range)> inverse) {
		_funcs.push_back(forward);
		_inv_funcs.push_back(inverse);
		return _funcs.size();
	}

	ary1(unsigned long code = 0) noexcept : _code(code) {
		assert(_funcs.size() > code);
	}
	ary1(const ary1& src) = default;
	ary1(ary1&& src) = default;
	~ary1() = default;
	ary1& operator=(const ary1& src) = default;
	ary1& operator=(ary1 && src) = default;

	auto code() const { return _code; }

	bool can_eval() const { return _funcs[_code]; }
	bool can_inv_eval() const { return _inv_funcs[_code]; }

	Range eval(Domain src) const { return _funcs[_code](src); }
	Domain inv_eval(Range src) const { return _inv_funcs[_code](src); }
};

}
}

#endif
