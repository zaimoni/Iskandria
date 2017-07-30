// coord_chart.hpp

#ifndef COORD_CHART_HPP
#define COORD_CHART_HPP 1

#include <type_traits>
#include <limits.h>
#include <stdint.h>

#include "Zaimoni.STL/Compiler.h"

#include "Euclidean.hpp"
#include "matrix.hpp"
#include "angle.hpp"

namespace zaimoni {
namespace math {

// XXX we probably don't want to commit as one header file -- dependencies questionable.  One file for vectors, one file for tuples

// primary coordinate systems are;
// * topological: no real distance function.  machine representation integer,  Need UI translator function(s), and possibly a legal geometry checker.
// ** this just needs compile-time calculation of the coordinate type.
// * Cartesian: parametrize on distance functions, Euclidean is L2.  Linfinity has diagonal moves same cost as orthogonal moves.
// * Minkowski space and relatives: L2-like, but trailing cartesian axes are - rather than + signature.

// Adapters are
// * oblique coordinates: 2-d adapter.  60 degrees is hexagonal coordinates
// * spherical cooardinates: n-d, at least 2 dimensions; first dimension is real radius, later dimensions are angles.

template<class T,size_t N>
struct Cartesian_vector
{
	ZAIMONI_STATIC_ASSERT(0<N);
	typedef zaimoni::math::vector<T,N> coord_type;
	static auto metric(const coord_type& lhs, const coord_type& rhs) {return zaimoni::math::Lesbegue<2>(lhs,rhs);}
};

template<class T,size_t N,size_t negative_coord=1>
struct Minkowski_vector
{
	ZAIMONI_STATIC_ASSERT(0<N);
	ZAIMONI_STATIC_ASSERT(0<negative_coord);
	typedef zaimoni::math::vector<T,N> coord_type;
	static auto metric(const coord_type& lhs, const coord_type& rhs) {return zaimoni::math::Minkowski<negative_coord>(lhs,rhs);}
};

// adapter
template<size_t N>
class spherical_vector
{
	ZAIMONI_STATIC_ASSERT(2<=N);
	typedef std::pair<boost::numeric::interval<double>, vector<zaimoni::circle::angle,N-1> > coord_type;
	// typename std::enable_if<std::is_same<boost::numeric::interval<double>, decltype(*T)> , void>		// doesn't work
	// typename std::enable_if<std::is_same<boost::numeric::interval<double>, decltype(T[])> , void>	// doesn't work
	template<class T> static void to_cartesian(const coord_type& src, T*  dest)	// just do array destinations for now
	{	// general idea
		// x: rcos(theta)sin(phi1)...sin(phin)
		// y: rsin(theta)sin(phi1)...sin(phin)
		// z: rcos(phi1)sin(phi2)...sin(phin)
		boost::numeric::interval<double> _sin;
		boost::numeric::interval<double> _cos;
		boost::numeric::interval<double> tmp(src.first);

		size_t i = N-1;
		while(1< --i)
			{
			src.second[i].sincos(_sin,_cos);
			dest[i+1] = tmp*_cos;
			tmp *= _sin;
			};
		src.second[0].sincos(_sin,_cos);
		dest[1] = tmp*_sin;
		dest[0] = tmp*_cos;
	}
	template<class T> static void from_cartesian(const T& src, coord_type& dest);
};


}	// namespace math
}	// namespace zaimoni

#endif
