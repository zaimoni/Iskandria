// slice.hpp

#ifndef SLICE_HPP
#define SLICE_HPP 1

#include "Zaimoni.STL/iterator_array_size.hpp"

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
	// container core types
	typedef T value_type;
    typedef T& reference;
    typedef T* pointer;
    typedef const T& const_reference;
    typedef ptrdiff_t difference_type;
    typedef size_t size_type;

	typedef iterator_array_size<slice_array<T> > iterator;
	typedef const_iterator_array_size<slice_array<T> > const_iterator;
	// at least bidirectional
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

	slice_array(T* src,const zaimoni::slice& filter) : _src((assert(src),src)),_filter(filter) {};
	ZAIMONI_DEFAULT_COPY_DESTROY_ASSIGN(slice_array);

	// container core API
	size_t size() const {return _filter.size();}
 	size_t max_size() const {return ((size_t)(-1)/2)/sizeof(T);}
	bool empty() const {return 0 >= _filter.size();} 
	void swap(slice_array& rhs) {
		swap(_src,rhs._src);
		swap(_filter,rhs._filter);
	}

	// don't want to pull in <algorithm> for these
	bool operator==(const slice_array& rhs) const
	{
		if (empty()) return rhs.empty();
		if (rhs.empty()) return false;
		if (size()!=rhs.size()) return false;
		const auto lhs_end = cend();
		const auto rhs_end = rhs.cend();
		if (lhs_end==rhs_end) return true;	// same container
		auto lhs_iter = cbegin();
		auto rhs_iter = rhs.cbegin();
		while(lhs_iter < lhs_end) {
			if (*lhs_iter!=*rhs_iter) return false;
			lhs_iter++;
			rhs_iter++;
		}
		return true;
	}
	bool operator!=(const slice_array& rhs) const { return !(*this==rhs); }

	iterator begin() { return iterator(this); };
	iterator end() { return iterator(this,_filter.size()); }
	const_iterator cbegin() const { return const_iterator(this); }
	const_iterator cend() const { return const_iterator(this,_filter.size()); }
	const_iterator begin() const { return cbegin(); }
	const_iterator end() const { return cend(); }
//	T& front() { return *begin(); }
//	T& back() { return *(--end()); }
//	const T& front() const { return *begin(); }
//	const T& back() const { return *(--end()); }
	T& front() { return _src[_filter.index(0)]; }
	T& back() { return _src[_filter.index(_filter.size()-1)]; }
	const T& front() const { return _src[_filter.index(0)]; }
	const T& back() const { return _src[_filter.index(_filter.size()-1)]; }

	// reversible container API
	reverse_iterator rbegin() { return reverse_iterator(this); };
	iterator rend() { return reverse_iterator(this,_filter.size()); }
	const_reverse_iterator crbegin() const { return const_reverse_iterator(this); }
	const_reverse_iterator crend() const { return const_reverse_iterator(this,_filter.size()); }
	const_reverse_iterator rbegin() const { return crbegin(); }
	const_reverse_iterator rend() const { return crend(); }

	// random access API
	T& operator[](size_t n) {return _src[_filter.index(n)];}
	const T& operator[](size_t n) const {return _src[_filter.index(n)];}
};

template<class T>
void swap(slice_array<T>& lhs, slice_array<T>& rhs) { lhs.swap(rhs); }

// lexicographical compare
// we don't want these to exist unless needed, as e.g. complex numerals don't have a meaningful operator <
template<class T>
bool operator<(const slice_array<T>& lhs, const slice_array<T>& rhs)
{
	if (lhs.empty()) return !rhs.empty();
	if (rhs.empty()) return false;
	const auto lhs_end = lhs.cend();
	const auto rhs_end = rhs.cend();
	if (lhs_end==rhs_end) return false;	// same container
	auto lhs_iter = lhs.cbegin();
	auto rhs_iter = rhs.cbegin();
	while(lhs_iter < lhs_end && rhs_iter < rhs_end) {
		if (*lhs_iter>=*rhs_iter) return false;
		lhs_iter++;
		rhs_iter++;
	}
	return lhs_iter==lhs_end;
}

template<class T>
bool operator>(const slice_array<T>& lhs, const slice_array<T>& rhs) { return rhs<lhs; }

template<class T>
bool operator<=(const slice_array<T>& lhs, const slice_array<T>& rhs) { return !(rhs<lhs); }

template<class T>
bool operator>=(const slice_array<T>& lhs, const slice_array<T>& rhs) { return !(lhs<rhs); }

}	// namespace zaimoni

#endif
