#ifndef PTR2_HPP
#define PTR2_HPP

//#include <type_traits>
//#include <functional>
//#include <atomic>
//#include "zero.hpp"

// explicit reference counting handle.
namespace zaimoni {

template<class T>
class intrusive_ptr
{
private:
	T* x;
	uintptr_t count;
public:
	intrusive_ptr() noexcept : x(0), count(0) {}
	intrusive_ptr(T* src) noexcept : x(src), count(0) {}
	intrusive_ptr(const intrusive_ptr& src) = delete;
	intrusive_ptr(intrusive_ptr&& src) = delete;
	~intrusive_ptr() {
		delete x;
		x = 0;
		count = 0;
	}

	intrusive_ptr& operator=(const intrusive_ptr& src) = delete;
	intrusive_ptr& operator=(intrusive_ptr&& src) = delete;

	T* get() {
		if ((uintptr_t)(-1) > count) count++;
		return x;
	}

	// usually called from host class destructor
	void release() {
		if ((uintptr_t)(-1) > count && 0 < count) count--;
		if (0 == count) {
			delete x;
			x = 0;
		}
	}

	bool empty() const { return x; }
	explicit operator bool() const noexcept { return x; }
};

}

#endif
