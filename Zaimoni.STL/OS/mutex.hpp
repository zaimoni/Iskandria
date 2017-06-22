// mutex.hpp
// hides the OS-specific details of a basic mutex
// (C)2009 Kenneth Boyd, license: MIT.txt

#ifndef ZAIMONI_STL_OS_MUTEX_HPP
#define ZAIMONI_STL_OS_MUTEX_HPP 1

namespace zaimoni {
namespace OS {

// this must be implemented in an OS-specific way
class mutex
{
private:
	void* const _raw_mutex;	// pointer to actual OS representation of the mutex

	mutex(const mutex& src);			// non copyable
	void operator=(const mutex& src);
public:
	mutex();
	~mutex();

	void lock();
	bool unlock();
};

class scoped_lock
{
private:
	mutex& _mutex;
	scoped_lock(const scoped_lock& src);	// non-copyable
	void operator=(const scoped_lock& src);
public:
	scoped_lock(mutex& src) : _mutex(src) {_mutex.lock();};
	~scoped_lock() {_mutex.unlock();};
};

}	// namespace OS
}	// namespace zaimoni

#endif
