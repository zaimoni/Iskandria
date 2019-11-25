#ifndef STACKTRACE_HPP
#define STACKTRACE_HPP

// opt-in stack trace object and support classes
#include <utility>
#include <vector>
#include <string>

namespace zaimoni {

class stacktrace {
private:
	typedef std::pair<const char*, std::vector<std::string> > frame;

	std::vector<frame> _x;
	stacktrace() = default;
	~stacktrace() = default;
public:
	stacktrace(const stacktrace& src) = delete;
	stacktrace(stacktrace&& src) = delete;
	stacktrace& operator=(const stacktrace& src) = delete;
	stacktrace& operator=(stacktrace && src) = delete;

	void push_back(const char* src) {
		_append(src);
		_x.push_back(frame(src, std::vector<std::string>()));
	};	// usually __PRETTY_FUNCTION__ macro
	void pop_back() {
		if (!_x.empty()) _x.pop_back();
		_summarize();
	};
	void log(const std::string& src) {
		_append(src);
		if (!_x.empty()) _x.back().second.push_back(src);
	}
	void log(std::string&& src) {
		_append(src);
		if (!_x.empty()) _x.back().second.push_back(std::move(src));
	}

	static stacktrace& get();
private:
	void _append(const char* src) const;
	void _append(const std::string& src) const { _append(src.c_str()); };
	void _summarize() const;
};

// \todo extract this to own header when needed elsewhere
template<class T, class U>
class ref_stack
{
private:
	T& _x;
public:
	ref_stack(T& x, U label) : _x(x) { _x.push_back(label); };
	~ref_stack() { _x.pop_back(); }
	ref_stack(const ref_stack& src) = delete;
	ref_stack(ref_stack&& src) = delete;
	ref_stack& operator=(const ref_stack& src) = delete;
	ref_stack& operator=(ref_stack&& src) = delete;
};


}


#endif
