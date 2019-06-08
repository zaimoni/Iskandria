#ifndef CONIC_HPP
#define CONIC_HPP

#include "Zaimoni.STL/Logging.h"
#include "interval_shim.hpp"

namespace zaimoni {
namespace math {

struct conic_tags
{
	// tag types
	struct parabola_t {};
	struct ellipse_t {};
	struct hyperbola_t {};

	// parameter types
	struct semi_major_axis_t {};
	struct semi_minor_axis_t {};
	struct eccentricity_t {};
	struct linear_eccentricity_t {};
	struct semi_latus_rectum_t {};
	struct focal_parameter_t {};
	struct flattening_t {};	// from reference ellipsoids: (a-b)/a

#define ZAIMONI_EMPTY_SINGLETON_ACCESSOR(A) static A##_t A() { static A##_t x; return x; }
	ZAIMONI_EMPTY_SINGLETON_ACCESSOR(parabola)
	ZAIMONI_EMPTY_SINGLETON_ACCESSOR(ellipse)
	ZAIMONI_EMPTY_SINGLETON_ACCESSOR(hyperbola)
	ZAIMONI_EMPTY_SINGLETON_ACCESSOR(semi_major_axis)
	ZAIMONI_EMPTY_SINGLETON_ACCESSOR(semi_minor_axis)
	ZAIMONI_EMPTY_SINGLETON_ACCESSOR(eccentricity)
	ZAIMONI_EMPTY_SINGLETON_ACCESSOR(linear_eccentricity)
	ZAIMONI_EMPTY_SINGLETON_ACCESSOR(semi_latus_rectum)
	ZAIMONI_EMPTY_SINGLETON_ACCESSOR(focal_parameter)
	ZAIMONI_EMPTY_SINGLETON_ACCESSOR(flattening)	// from reference ellipsoids: (a-b)/a
#undef ZAIMONI_EMPTY_SINGLETON_ACCESSOR
};

// T should be an interval type representing the real numbers R
class conic : public conic_tags
{
public:
	typedef interval_shim::interval interval;
private:
	interval _a;	// semi-major axis
	interval _b;	// semi-minor axis
	interval _c;	// linear eccentricity
	interval _e;	// linear eccentricity
	interval _l;	// semi-latus rectum
	interval _p;	// focal parameter
public:
	using conic_tags::parabola;
	// tag types
	using conic_tags::ellipse;
	using conic_tags::hyperbola;

	// parameter types
	using conic_tags::semi_major_axis;
	using conic_tags::semi_minor_axis;
	using conic_tags::eccentricity;
	using conic_tags::linear_eccentricity;
	using conic_tags::semi_latus_rectum;
	using conic_tags::focal_parameter;
	using conic_tags::flattening;	// from reference ellipsoids: (a-b)/a

	conic() = default;
	conic(const conic& src) = default;
	~conic() = default;
	conic& operator=(const conic& src) = default;

	explicit conic(const interval& radius)	// circle
	: _a((assert(0 <= radius.lower()),radius)), _b(radius), _c(0), _e(0), _l(radius), _p(interval::whole().upper()) {};

	conic(conic_tags::parabola_t, const interval& src, semi_major_axis_t)
	: _a((assert(0 <= src.lower()),src)),_b(0),_e(1),_c(_a),_l(2.0*src),_p(_l) {}	// linear eccentricity is the limit value

	conic(conic_tags::parabola_t, const interval& src, semi_latus_rectum_t)
	: _a((assert(0 <= src.lower()),src/2.0)),_b(0),_e(1),_c(_a),_l(src),_p(_l) {}

	conic(conic_tags::parabola_t, const interval& src, focal_parameter_t)
	: _a((assert(0 <= src.lower()), src / 2.0)),_b(0),_e(1),_c(_a),_l(src),_p(_l) {}

	conic(conic_tags::ellipse_t, const interval& src, const interval& src2);
	conic(conic_tags::hyperbola_t, const interval& src, const interval& src2);

	friend bool operator==(const conic& lhs, const conic& rhs);

	// accessors
	const interval& a() const { return _a; };
	const interval& b() const { return _b; };
	const interval& c() const { return _c; };
	const interval& e() const { return _e; };
	const interval& l() const { return _l; };
	const interval& p() const { return _p; };

	// geometry testers
	bool is_circle() const { return _e == 0.0; };
#if CONSTANTS_ISK_INTERVAL
	bool is_not_circle() const { return !_e.contains(0); };
#else
	bool is_not_circle() const { return 0<_e.lower(); };
#endif

	bool is_parabola() const { return _e == 1.0; };
#if CONSTANTS_ISK_INTERVAL
	bool is_not_parabola() const { return !_e.contains(1); };
#else
	bool is_not_parabola() const { return 1>_e.upper() || 1 < _e.lower(); };
#endif

	bool is_ellipse() const { return 0 < _e.lower() && 1 > _e.upper(); };
	bool is_not_ellipse() const { return 0 >= _e.upper() || 1 < _e.lower(); };

	bool is_hyperbola() const { return 1 < _e.lower(); };
	bool is_not_hyperbola() const { return 1 >= _e.upper(); };
private:
	void init_ellipse(const interval& major, const interval& minor);
	void init_hyperbola(const interval& major, const interval& minor);
};

inline bool operator!=(const conic& lhs, const conic& rhs) { return !(lhs == rhs); }

}	// namespace math
}	// namespace zaimoni

#endif
