// int_range.hpp

#ifndef INT_RANGE_HPP
#define INT_RANGE_HPP 1

#include "Zaimoni.STL/Logging.h"
#include "Zaimoni.STL/augment.STL/type_traits"
#include <utility>

#include "Zaimoni.STL/iterator.on.hpp"
#include <iterator>

namespace zaimoni {
namespace math {

// Cf Python 2/3 xrange/range
template<class T>
class int_range : public std::pair<T,T>
{
public:
	static_assert(std::is_integral<T>::value);
	typedef std::pair<T,T> super;

	// container core types
	typedef T value_type;
    typedef T& reference;
    typedef T* pointer;
    typedef const T& const_reference;
    typedef ptrdiff_t difference_type;
    typedef typename std::make_unsigned<T>::type size_type;

	class const_iterator
	{
public:
	typedef typename int_range::difference_type difference_type;	// these five aligned with container
	typedef typename int_range::size_type size_type;
	typedef const typename int_range::value_type value_type;
	typedef typename int_range::const_reference reference;
	typedef typename int_range::pointer pointer;
	typedef std::random_access_iterator_tag iterator_category;
private:
	const int_range* _src;
	T _i;

	bool can_dereference() const {return _src && _src->lower()<=_i && _src->upper()>=_i;}
public:
	const_iterator(const int_range* src = 0, T origin = 0) : _src(src),_i(origin) {};
	ZAIMONI_DEFAULT_COPY_DESTROY_ASSIGN(const_iterator);


	bool operator==(const const_iterator& rhs) const {return _src==rhs._src && _i==rhs._i;}

    reference operator*() const {
		assert(can_dereference());
		return _i;
	};
    const_iterator& operator++() {	// prefix
		if (_src->upper() >= _i) _i++;
		return *this;
	};
    const_iterator operator++(int) {	// postfix
		const_iterator ret(*this);
		if (_src->upper() >= _i) _i++;
		return ret;
	};

	// bidirectional
    const_iterator& operator--() {	// prefix
		if (_src->lower() < _i) _i--;
		return *this;
	};
    const_iterator operator--(int) {	// postfix
		const_iterator ret(*this);
		if (_src->lower() < _i) _i--;
		return ret;
	};

	// random access
    const_iterator& operator+=(difference_type n) {
		if (0<n) {
			assert((size_type)(_src->upper())-(size_type)(_i)+(size_type)(1) >= size_type(n));
			_i += n;
		} else if (0>n) {
			assert((size_type)(_i)-size_type(_src->lower()) >= (size_type)(-n));
			_i += n;
		}
		return *this;
	};

    const_iterator& operator-=(difference_type n) {
		if (0<n) {
			assert((size_type)(_i)-size_type(_src->lower()) >= (size_type)(n));
			_i -= n;
		} else if (0>n) {
			assert((size_type)(_src->upper())-(size_type)(_i)+(size_type)(1) >= size_type(-n));
			_i -= n;
		}
		return *this;
	};
	difference_type operator-(const const_iterator& rhs) {
		assert(_src==rhs._src);
		return (difference_type)((size_type)(_i)-(size_type)(rhs._i));
	}

    reference operator[](size_type n) const {
		assert(_src->size() >= n || 0==_src->size());
		return (*_src)[n];
	}

    bool operator<(const const_iterator& rhs) const {
		assert(_src==rhs._src);
		return _i < rhs._i;
	}

	ZAIMONI_ITER_DECLARE(const_iterator)
	};

	typedef const_iterator iterator;
	// at least bidirectional
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

	int_range() = default;
	int_range(T lower, T upper) : super(lower,upper) {};
	ZAIMONI_DEFAULT_COPY_DESTROY_ASSIGN(int_range);

	T& lower() {return this->first;};
	typename return_copy<T>::type lower() const {return this->first;};
	T& upper() {return this->second;};
	typename return_copy<T>::type upper() const {return this->second;};

	// container core API
	size_t size() const {
		if (lower()>upper()) return 0;
		return (size_type)(upper())-(size_type)(lower())+1;
	}
 	size_t max_size() const {return (size_type)(-1);}
	bool empty() const {return lower()>upper();} 	// XXX size 0 could simply be wraparound
//	void swap(int_range& rhs);	// check whether std::pair will catch us

	bool operator==(const int_range& rhs) const
	{
		if (empty()) return rhs.empty();
		if (rhs.empty()) return false;
		return *static_cast<const super*>(this)==rhs;	// use base definition if no unusual circumstances
	}

	iterator begin() { return iterator(this,this->lower()); };
	iterator end() { return iterator(this,this->upper()+1); }
	const_iterator cbegin() const { return const_iterator(this,this->lower()); }
	const_iterator cend() const { return const_iterator(this,this->upper()+1); }
	const_iterator begin() const { return cbegin(); }
	const_iterator end() const { return cend(); }
//	T& front() { return *begin(); }
//	T& back() { return *(--end()); }
//	const T& front() const { return *begin(); }
//	const T& back() const { return *(--end()); }
	typename return_copy<T>::type front() const { return this->first; }
	typename return_copy<T>::type back() const { return this->second; }

	// reversible container API
	reverse_iterator rbegin() { return reverse_iterator(end()); };
	iterator rend() { return reverse_iterator(begin()); }
	const_reverse_iterator crbegin() const { return const_reverse_iterator(cend()); }
	const_reverse_iterator crend() const { return const_reverse_iterator(cbegin()); }
	const_reverse_iterator rbegin() const { return crbegin(); }
	const_reverse_iterator rend() const { return crend(); }

	// random access
	T operator[](size_type n) const {
		assert(!empty());
		assert(size()>n || 0==size());
		return (T)((size_type)(lower())+n);
	}

	// we know what to do with these anyway
	void pop_front() { assert(!empty()); this->first++; }
	void pop_back() { assert(!empty()); this->second--; }
};

template<class T>
void swap(int_range<T>& lhs, int_range<T>& rhs) { lhs.swap(rhs); }

// lexicographical compare
// we don't want these to exist unless needed, as e.g. complex numerals don't have a meaningful operator <
template<class T>
bool operator<(const int_range<T>& lhs, const int_range<T>& rhs)
{
	if (lhs.empty()) return !rhs.empty();
	if (rhs.empty()) return false;
	return static_cast<const typename int_range<T>::super&>(lhs)<rhs;
}

template<class T>
bool operator>(const int_range<T>& lhs, const int_range<T>& rhs) { return rhs<lhs; }

template<class T>
bool operator<=(const int_range<T>& lhs, const int_range<T>& rhs) { return !(rhs<lhs); }

template<class T>
bool operator>=(const int_range<T>& lhs, const int_range<T>& rhs) { return !(lhs<rhs); }

}	// namespace math
}	// namespace zaimoni

#undef ZAIMONI_ITER_DECLARE

#endif
