// coord_chart.hpp

#ifndef COORD_CHART_HPP
#define COORD_CHART_HPP 1

#include <limits.h>

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
public:
	typedef std::pair<ISK_INTERVAL<double>, vector<zaimoni::circle::angle,N-1> > coord_type;
	// typename std::enable_if<std::is_same<boost::numeric::interval<double>, decltype(*T)> , void>		// doesn't work
	// typename std::enable_if<std::is_same<boost::numeric::interval<double>, decltype(T[])> , void>	// doesn't work
	template<class T> static void to_cartesian(const coord_type& src, T*  dest)	// just do array destinations for now
	{	// general idea
		// x: rcos(theta)sin(phi1)...sin(phin)
		// y: rsin(theta)sin(phi1)...sin(phin)
		// z: rcos(phi1)sin(phi2)...sin(phin)
		// above convention (in the source code below) is that in a calculus textbook i.e. phi 0...180 degrees.
		// Alternate convention: -90 degrees to 90 degrees (south pole to north pole); transform is geocentric phi = 90 degrees - calculus phi.
		// neither of these are a canonical geodetic coordinate system: https://en.wikipedia.org/wiki/Reference_ellipsoid
		ISK_INTERVAL<double> _sin;
		ISK_INTERVAL<double> _cos;
		ISK_INTERVAL<double> tmp(src.first);

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

// to interoperate with standard references, we want to deal with geodetic and geocentic latitude/longitude
// geodetic: phi is normal to reference ellipsoid
// geocentric: phi normal to the geometric center
// theta is taken to be either -180 to 180 degrees (earth/sun/moon), or 0 to 360 degrees (everything else).  
// As we use an angle class modeling the 1-dimensional circle S1, this only affects textual display.
// phi is taken to be -90 to 90 degrees north to south.
template<size_t N>
class geocentric_vector
{
	ZAIMONI_STATIC_ASSERT(2<=N);
public:
	typedef std::pair<ISK_INTERVAL<double>, vector<zaimoni::circle::angle,N-1> > coord_type;
	// typename std::enable_if<std::is_same<boost::numeric::interval<double>, decltype(*T)> , void>		// doesn't work
	// typename std::enable_if<std::is_same<boost::numeric::interval<double>, decltype(T[])> , void>	// doesn't work
	template<class T> static void to_cartesian(const coord_type& src, T*  dest)	// just do array destinations for now
	{	// general idea
		// x: rcos(theta)cos(phi1)...cos(phin)
		// y: rsin(theta)cos(phi1)...cos(phin)
		// z: rsin(phi1)sin(phi2)...sin(phin)
		ISK_INTERVAL<double> _sin;
		ISK_INTERVAL<double> _cos;
		ISK_INTERVAL<double> tmp(src.first);

		size_t i = N-1;
		while(1< --i)
			{
			src.second[i].sincos(_sin,_cos);
			dest[i+1] = tmp*_sin;
			tmp *= _cos;
			};
		src.second[0].sincos(_sin,_cos);
		dest[1] = tmp*_sin;
		dest[0] = tmp*_cos;
	}
	// need extra overloads of above taking reference ellipsoids, for geodetic coordinates
	template<class T> static void from_cartesian(const T& src, coord_type& dest);
};

}	// namespace math
}	// namespace zaimoni

#endif
