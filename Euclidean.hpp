// Euclidean.hpp

#ifndef EUCLIDEAN_HPP
#define EUCLIDEAN_HPP

#include <stddef.h>
#include <vector>
#include "overprecise.hpp"

namespace zaimoni {
namespace math {
namespace Euclidean {

// let's try this again now that slice_array.hpp has iterators.
// what we would like is: if the "value types have the same interval-arithmeatic type, then allow the dot product, etc.
#define ONLY_IF_NUMERICALLY_COMPATIBLE(A,B)	\
typename std::enable_if<	\
	std::is_same<	\
		typename interval_type<A>::type,	\
		typename interval_type<B>::type	\
	>::value,	\
	typename interval_type<A>::type	\
>::type

#if 0
template<class T1, class T2>
ONLY_IF_NUMERICALLY_COMPATIBLE(typename T1::value_type,typename T2::value_type) dot(const T1& lhs, const T2& rhs)
{
	assert(lhs.size()==rhs.size());
	assert(0<lhs.size());
	std::vector<typename interval_type<typename T1::value_type>::type> accumulator;
	std::vector<typename std::remove_cv<typename T1::value_type>::type> lhs_terms;
	std::vector<typename std::remove_cv<typename T2::value_type>::type> rhs_terms;

	{	// scoping brace
	const auto lhs_end = lhs.end();
	const auto rhs_end = rhs.end();
	auto lhs_iter = lhs.begin();
	auto rhs_iter = rhs.begin();
	while(lhs_iter<lhs_end)
		{
		typename std::remove_cv<typename T1::value_type>::type lhs_term = *lhs_iter++;
		typename std::remove_cv<typename T2::value_type>::type rhs_term = *rhs_iter++;
		if (rearrange_product(lhs_term,rhs_term))	// may throw runtime errors which would indicate need to do something else
			{	// evaluated
			if (typename std::remove_cv<typename T1::value_type>::type(0)==lhs_term) continue;		// ignore additive identity 0
			accumulator.push_back(lhs_term);
//			if (1<accumulator.size()) incremental_rearrange_sum(accumulator);
			continue;
			}
		else{
			lhs_terms.push_back(lhs_term);
			rhs_terms.push_back(rhs_term);
//			incremental_rearrange_dot(lhs_term,rhs_term,accumulator);
			}
		}
	}	// end scoping brace
	while(!lhs_terms.empty())
		{
		accumulator.push_back(lossy<typename interval_type<typename T1::value_type>::type::base_type>::product(lhs_terms.back(),rhs_terms.back()));
		lhs_terms.pop_back();
		rhs_terms.pop_back();
//		if (1<accumulator.size()) incremental_rearrange_sum(accumulator);
		}
	if (accumulator.empty()) return 0;	// zero-term sum is usually defined as zero
	// XXX replace this with something reasonable
	while(1<accumulator.size())
		{
		accumulator.front() += accumulator.back();
		accumulator.pop_back();
		}
	return accumulator.front();
}
#endif

#define NO_CV(A) typename std::remove_cv<A>::type
#define NO_CV_SAME(A,B) std::is_same<NO_CV(A), NO_CV(B)>::value
#define ONLY_IF_NO_CV_SAME(A,B) typename std::enable_if<NO_CV_SAME(A,B),NO_CV(A)>::type

// the dot product is very easy to botch numerically.  Put a default wrong version here and deal with accuracy later.

#if 2
// this version assumes ::size() and operator[]
// would also like an iterator version
template<class T1, class T2>
ONLY_IF_NO_CV_SAME(typename T1::value_type,typename T2::value_type) dot(const T1& lhs, const T2& rhs)
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
#endif

}	// namespace Euclidean

// useful specializations
// N=0: hack for L-infinity metric
// N=1: L1
// N=2: L2 (Euclidean) distance
// unfortunately, specializing template functions is problematic.
// Cf. http://www.gotw.ca/publications/mill17.htm
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
}

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
}

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
}

template<int negative_coords,class T1, class T2>
ONLY_IF_NO_CV_SAME(typename T1::value_type,typename T2::value_type) Minkowski(const T1& lhs, const T2& rhs)
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

#undef NO_CV
#undef NO_CV_SAME
#undef ONLY_IF_NO_CV_SAME
#undef ONLY_IF_NUMERICALLY_COMPATIBLE

}	// namespace math
}	// namespace zaimoni

#endif

