// Euclidean.hpp

#ifndef EUCLIDEAN_HPP
#define EUCLIDEAN_HPP

#include <vector>
#include <type_traits>

namespace zaimoni {
namespace math {
namespace Euclidean {

// the dot product is very easy to botch numerically.  Put a default wrong version here and deal with accuracy later.

// this version assumes ::size() and operator[]
// would also like an iterator version
template<class T1, class T2>
typename std::enable_if<std::is_same<typename std::remove_cv<typename T1::value_type>::type, 
                                     typename std::remove_cv<typename T2::value_type>::type>::value,
                        typename std::remove_cv<typename T1::value_type>::type >::type dot(const T1& lhs, const T2& rhs)
{
	assert(lhs.size()==rhs.size());
	assert(0<lhs.size());

	size_t i = lhs.size();
	std::vector<typename std::remove_cv<typename T1::value_type>::type> accumulator(i);
	typename T1::value_type zero(0);
	while(0<i)
		{
		--i;
		// XXX need to handle other exceptional conditions by type; e.g. double NaN or infinity
		if (zero == lhs[i] || zero == rhs[i]) continue;
		// XXX need to handle overflow by type
		accumulator.push_back(lhs[i]*rhs[i]);
		}

	if (accumulator.empty()) return 0;
	// XXX need to be overflow-aware when doing this sum, etc.
	while(2<=accumulator.size()) {
		accumulator[0] += accumulator.back();
		accumulator.pop_back();
	}
	return accumulator.front();
}


}	// namespace Euclidean

// useful specializations
// N=0: hack for L-infinity metric
// N=1: L1
// N=2: L2 (Euclidean) distance
// unfortunately, specializing template functions is problematic.
#define NO_CV(A) typename std::remove_cv<A>::type
#define NO_CV_SAME(A,B) std::is_same<NO_CV(A), NO_CV(B)>::value
#define ONLY_IF_NO_CV_SAME(A,B) typename std::enable_if<NO_CV_SAME(A,B),NO_CV(A)>::type

template<size_t N,class T1, class T2>
typename std::enable_if<(2<N), ONLY_IF_NO_CV_SAME(typename T1::value_type,typename T2::value_type)>::type Lesbegue(const T1& lhs, const T2& rhs)
{
	assert(lhs.size()==rhs.size());
	assert(0<lhs.size());

	size_t i = lhs.size();
	std::vector<typename std::remove_cv<typename T1::value_type>::type> accumulator(i);
	typename T1::value_type zero(0);
	while(0<i)
		{
		--i;
		// XXX need to handle other exceptional conditions by type; e.g. double NaN or infinity
		typename std::remove_cv<typename T1::value_type>::type tmp(rhs[i]-lhs[i]);
		if (zero == tmp) continue;
		if (1==N%2 && zero > tmp) tmp = -tmp;
		// XXX need to handle overflow by type
		accumulator.push_back(pow(tmp,N));
		}

	if (accumulator.empty()) return 0;
	// XXX need to be overflow-aware when doing this sum, etc.
	while(2<=accumulator.size()) {
		accumulator[0] += accumulator.back();
		accumulator.pop_back();
	}
	return pow(accumulator.front(),1.0/N);
}

template<size_t N,class T1, class T2>
typename std::enable_if<(2==N), ONLY_IF_NO_CV_SAME(typename T1::value_type,typename T2::value_type)>::type Lesbegue(const T1& lhs, const T2& rhs)
{
	assert(lhs.size()==rhs.size());
	assert(0<lhs.size());

	size_t i = lhs.size();
	std::vector<typename std::remove_cv<typename T1::value_type>::type> accumulator(i);
	typename T1::value_type zero(0);
	while(0<i)
		{
		--i;
		// XXX need to handle other exceptional conditions by type; e.g. double NaN or infinity
		typename std::remove_cv<typename T1::value_type>::type tmp(rhs[i]-lhs[i]);
		if (zero == tmp) continue;
		// XXX need to handle overflow by type
		accumulator.push_back(square(tmp));
		}

	if (accumulator.empty()) return 0;
	// XXX need to be overflow-aware when doing this sum, etc.
	while(2<=accumulator.size()) {
		accumulator[0] += accumulator.back();
		accumulator.pop_back();
	}
	return sqrt(accumulator.front());
};

template<size_t N,class T1, class T2>
typename std::enable_if<(1==N), ONLY_IF_NO_CV_SAME(typename T1::value_type,typename T2::value_type)>::type Lesbegue(const T1& lhs, const T2& rhs)
{
	assert(lhs.size()==rhs.size());
	assert(0<lhs.size());

	size_t i = lhs.size();
	std::vector<typename std::remove_cv<typename T1::value_type>::type> accumulator(i);
	typename T1::value_type zero(0);
	while(0<i)
		{
		--i;
		// XXX need to handle other exceptional conditions by type; e.g. double NaN or infinity
		typename std::remove_cv<typename T1::value_type>::type tmp(rhs[i]-lhs[i]);
		if (zero == tmp) continue;
		if (zero > tmp) tmp = -tmp;
		// XXX need to handle overflow by type
		accumulator.push_back(tmp);
		}

	if (accumulator.empty()) return 0;
	// XXX need to be overflow-aware when doing this sum, etc.
	while(2<=accumulator.size()) {
		accumulator[0] += accumulator.back();
		accumulator.pop_back();
	}
	return accumulator.front();
};

template<size_t N,class T1, class T2>
typename std::enable_if<(0==N), ONLY_IF_NO_CV_SAME(typename T1::value_type,typename T2::value_type)>::type Lesbegue(const T1& lhs, const T2& rhs)
{
	assert(lhs.size()==rhs.size());
	assert(0<lhs.size());

	size_t i = lhs.size();
	typename T1::value_type zero(0);
	typename std::remove_cv<typename T1::value_type>::type ub(0);
	while(0<i)
		{
		--i;
		// XXX need to handle other exceptional conditions by type; e.g. double NaN or infinity
		typename std::remove_cv<typename T1::value_type>::type tmp(rhs[i]-lhs[i]);
		if (zero == tmp) continue;
		if (1==N%2 && zero > tmp) tmp = -tmp;
		if (ub < tmp) ub = tmp;
		}
	return ub;
};

template<int negative_coords,class T1, class T2>
typename std::enable_if<std::is_same<typename std::remove_cv<typename T1::value_type>::type, 
                                     typename std::remove_cv<typename T2::value_type>::type>::value,
                        typename std::remove_cv<typename T1::value_type>::type >::type Minkowski(const T1& lhs, const T2& rhs)
{
	assert(lhs.size()==rhs.size());
	assert(0<lhs.size());

	size_t i = lhs.size();
	int negate = negative_coords;
	std::vector<typename std::remove_cv<typename T1::value_type>::type> accumulator(i);
	typename T1::value_type zero(0);
	while(0<i)
		{
		--i;
		--negate;
		// XXX need to handle other exceptional conditions by type; e.g. double NaN or infinity
		typename std::remove_cv<typename T1::value_type>::type tmp(rhs[i]-lhs[i]);
		if (zero == tmp) continue;
		// XXX need to handle overflow by type
		accumulator.push_back(0<=negate ? -square(tmp) : square(tmp));
		}

	if (accumulator.empty()) return 0;
	// XXX need to be overflow-aware when doing this sum, etc.
	while(2<=accumulator.size()) {
		accumulator[0] += accumulator.back();
		accumulator.pop_back();
	}
	return sqrt(accumulator.front());
}

}	// namespace math
}	// namespace zaimoni

#endif

