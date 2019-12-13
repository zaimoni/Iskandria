// mutex.hpp
// hides the OS-specific details of a basic mutex
// (C)2009,2019 Kenneth Boyd, license: LICENSE_BOOST.txt

#ifndef ZAIMONI_STL_OS_MUTEX_HPP
#define ZAIMONI_STL_OS_MUTEX_HPP 1

namespace zaimoni {
namespace OS {

// this must be implemented in an OS-specific way
class mutex
{
private:
	void* const _raw_mutex;	// pointer to actual OS representation of the mutex

	mutex(const mutex& src) = delete;	// non copyable
	mutex(mutex&& src) = delete;
	void operator=(const mutex& src) = delete;
	void operator=(mutex&& src) = delete;	
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

	scoped_lock(const scoped_lock& src) = delete;	// non-copyable
	scoped_lock(scoped_lock&& src) = delete;
	void operator=(const scoped_lock& src) = delete;
	void operator=(scoped_lock&& src) = delete;
public:
	scoped_lock(mutex& src) : _mutex(src) {_mutex.lock();};
	~scoped_lock() {_mutex.unlock();};
};

}	// namespace OS
}	// namespace zaimoni

#endif
