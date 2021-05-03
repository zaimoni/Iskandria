#ifndef FUNC_HPP
#define FUNC_HPP

#include <vector>
#include <functional>
#include <tuple>
#include "Zaimoni.STL/augment.STL/type_traits"
#include "Zaimoni.STL/Logging.h"

namespace zaimoni {
namespace math {

template<class Range, class...Args>
class func {
public:
	using InvRange = std::conditional_t<
		1==sizeof...(Args),
		nth_element<0, Args...>,
		std::tuple<Args...> >;
private:
	unsigned long _code;
	static std::vector<std::function<Range(Args...)>> _funcs;
	static std::vector<std::function<InvRange(Range)>> _inv_funcs;

public:
	static auto declare(std::function<Range(Args...)> forward, std::function<InvRange(Range)> inverse) {
		_funcs.push_back(forward);
		_inv_funcs.push_back(inverse);
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

	bool can_eval() const { return _funcs[_code]; }
	bool can_inv_eval() const { return _inv_funcs[_code]; }

	Range eval(Args... src) const { return _funcs[_code](src); }
	InvRange inv_eval(Range src) const { return _inv_funcs[_code](src); }
};

}
}

#endif
