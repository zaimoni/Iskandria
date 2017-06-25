// slice.hpp

#ifndef SLICE_HPP
#define SLICE_HPP 1

#include "Zaimoni.STL/Logging.h"

namespace zaimoni {

// closely follow std::slice (note non-required index member function, and absence of default constructor)
class slice
{
private:
	size_t _start;
	size_t _size;
	size_t _stride;
public:
	slice(size_t start,size_t length,size_t stride) : 
		_start(start),_size((assert(0<length),length)),_stride((assert(0<stride),assert((size_t)(-1)/length>=stride),assert((size_t)(-1)-start>=(length*stride)),stride))
		{};

	size_t start() const {return _start;};
	size_t size() const {return _size;};
	size_t stride() const {return _stride;};

	size_t index(size_t n) const {
		assert(size()>n);
		return _start+n*_stride;
	}
};

// like STL, reference semantics on underlying array
template<class T>
class slice_array
{
private:
	T* _src;
	zaimoni::slice _filter;
public:
	typedef T value_type;

	slice_array(T* src,const zaimoni::slice& filter) : _src(src),_filter(filter) {};
	slice_array(const slice_array& src) : _src(src._src),_filter(src._filter) {};

	T& operator[](size_t n) {return _src[_filter.index(n)];}
	const T& operator[](size_t n) const {return _src[_filter.index(n)];}
	size_t size() const {return _filter.size();};
};

}	// namespace zaimoni

#endif
