// Euclidean.hpp

#ifndef EUCLIDEAN_HPP
#define EUCLIDEAN_HPP

#include <stddef.h>
#include <vector>
#include "series_sum.hpp"

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

template<class T1, class T2>
ONLY_IF_NUMERICALLY_COMPATIBLE(typename T1::value_type,typename T2::value_type) dot(const T1& lhs, const T2& rhs)
{
	assert(lhs.size()==rhs.size());
	assert(0<lhs.size());
	series_sum<typename interval_type<typename T1::value_type>::type> accumulator;
//	series_sum<typename interval_type<typename T1::value_type>::type::base_type> accumulator_exact;
	std::vector<typename std::remove_cv<typename T1::value_type>::type> lhs_terms;
	std::vector<typename std::remove_cv<typename T2::value_type>::type> rhs_terms;
#ifdef ZAIMONI_USING_STACKTRACE
	zaimoni::ref_stack<zaimoni::stacktrace, const char*> log(zaimoni::stacktrace::get(), __PRETTY_FUNCTION__);
#endif

	{	// scoping brace
	const auto lhs_end = lhs.end();
	const auto rhs_end = rhs.end();
	auto lhs_iter = lhs.begin();
	auto rhs_iter = rhs.begin();
	while(lhs_iter<lhs_end)
		{
		typename std::remove_cv<typename T1::value_type>::type lhs_term = *lhs_iter++;
		typename std::remove_cv<typename T2::value_type>::type rhs_term = *rhs_iter++;
		// reject isnan now
		if (isNaN(lhs_term)) throw std::runtime_error("incoming NaN");
		if (isNaN(rhs_term)) throw std::runtime_error("incoming NaN");
		const int rearranged_product = trivial_product(lhs_term,rhs_term);
		switch(rearranged_product)
		{
		case -2:	// should have been trivial but failed
			goto defer_product;
		case -1:	swap(lhs_term,rhs_term);
		// intentional fall-through
		case 1:
			if (lhs_term == 0.0) continue;		// ignore additive identity 0

		}
		if (rearranged_product || rearrange_product(lhs_term,rhs_term))	// may throw runtime errors which would indicate need to do something else
			{	// evaluated
			if (lhs_term == 0.0) continue;		// ignore additive identity 0
//			if (lhs_term.lower()==lhs_term.upper())
//				{	// exact
//				accumulator_exact.push_back(lhs_term);
//				}
//			else{
				accumulator.push_back(lhs_term);
//				}
			continue;
			}
defer_product:
		lhs_terms.push_back(lhs_term);
		rhs_terms.push_back(rhs_term);
		}
	}	// end scoping brace
	while(!lhs_terms.empty())
		{
		accumulator.push_back(lossy<typename interval_type<typename T1::value_type>::type::base_type>::product(lhs_terms.back(),rhs_terms.back()));
		lhs_terms.pop_back();
		rhs_terms.pop_back();
		}
	return accumulator.eval();
}

#define NO_CV(A) typename std::remove_cv<A>::type
#define NO_CV_SAME(A,B) std::is_same<NO_CV(A), NO_CV(B)>::value
#define ONLY_IF_NO_CV_SAME(A,B) typename std::enable_if<NO_CV_SAME(A,B),NO_CV(A)>::type

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
	series_sum<typename std::remove_cv<typename T1::value_type>::type> accumulator(i);
	while(0<i)
		{
		--i;
		// XXX need to handle other exceptional conditions by type; e.g. double NaN or infinity
		typename std::remove_cv<typename T1::value_type>::type tmp(rhs[i]-lhs[i]);
		if (int_as<0,typename T1::value_type>() == tmp) continue;
		if (1==N%2 && int_as<0,typename T1::value_type>() > tmp) tmp = -tmp;
		// XXX need to handle overflow by type
		accumulator.push_back(pow(tmp,N));
		}
	return pow(accumulator.eval(),1.0/N);
}

template<size_t N,class T1, class T2>
typename std::enable_if<(2==N), ONLY_IF_NO_CV_SAME(typename T1::value_type,typename T2::value_type)>::type Lesbegue(const T1& lhs, const T2& rhs)
{
	assert(lhs.size()==rhs.size());
	assert(0<lhs.size());

	size_t i = lhs.size();
	series_sum<typename std::remove_cv<typename T1::value_type>::type> accumulator(i);
	while(0<i)
		{
		--i;
		// XXX need to handle other exceptional conditions by type; e.g. double NaN or infinity
		typename std::remove_cv<typename T1::value_type>::type tmp(rhs[i]-lhs[i]);
		if (int_as<0,typename T1::value_type>() == tmp) continue;
		// XXX need to handle overflow by type
		accumulator.push_back(square(tmp));
		}

	return sqrt(accumulator.eval());
}

template<size_t N,class T1, class T2>
typename std::enable_if<(1==N), ONLY_IF_NO_CV_SAME(typename T1::value_type,typename T2::value_type)>::type Lesbegue(const T1& lhs, const T2& rhs)
{
	assert(lhs.size()==rhs.size());
	assert(0<lhs.size());

	size_t i = lhs.size();
	series_sum<typename std::remove_cv<typename T1::value_type>::type> accumulator(i);
	while(0<i)
		{
		--i;
		// XXX need to handle other exceptional conditions by type; e.g. double NaN or infinity
		typename std::remove_cv<typename T1::value_type>::type tmp(rhs[i]-lhs[i]);
		if (int_as<0,typename T1::value_type>() == tmp) continue;
		if (int_as<0,typename T1::value_type>() > tmp) tmp = -tmp;
		// XXX need to handle overflow by type
		accumulator.push_back(tmp);
		}
	return accumulator.eval();
}

template<size_t N,class T1, class T2>
typename std::enable_if<(0==N), ONLY_IF_NO_CV_SAME(typename T1::value_type,typename T2::value_type)>::type Lesbegue(const T1& lhs, const T2& rhs)
{
	assert(lhs.size()==rhs.size());
	assert(0<lhs.size());

	size_t i = lhs.size();
	typename std::remove_cv<typename T1::value_type>::type ub(0);
	while(0<i)
		{
		--i;
		// XXX need to handle other exceptional conditions by type; e.g. double NaN or infinity
		typename std::remove_cv<typename T1::value_type>::type tmp(rhs[i]-lhs[i]);
		if (int_as<0,typename T1::value_type>() == tmp) continue;
		if (1==N%2 && int_as<0,typename T1::value_type>() > tmp) tmp = -tmp;
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
	series_sum<typename std::remove_cv<typename T1::value_type>::type> accumulator(i);
	while(0<i)
		{
		--i;
		--negate;
		// XXX need to handle other exceptional conditions by type; e.g. double NaN or infinity
		typename std::remove_cv<typename T1::value_type>::type tmp(rhs[i]-lhs[i]);
		if (int_as<0,typename T1::value_type>() == tmp) continue;
		// XXX need to handle overflow by type
		accumulator.push_back(0<=negate ? -square(tmp) : square(tmp));
		}
	return sqrt(accumulator.eval());
}

#undef NO_CV
#undef NO_CV_SAME
#undef ONLY_IF_NO_CV_SAME
#undef ONLY_IF_NUMERICALLY_COMPATIBLE

}	// namespace math
}	// namespace zaimoni

#endif

