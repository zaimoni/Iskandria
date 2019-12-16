// angle.cpp

#include "angle.hpp"
#include "taylor.hpp"
#include "Zaimoni.STL/augment.STL/array"

static constexpr const typename interval_shim::interval::base_type _whole_circle = 10125.0;

zaimoni::circle::angle::angle(const angle& lb, const angle& ub)
: _theta(lb._theta.lower(), ub._theta.upper())
{
	if (_theta.lower() > _theta.upper()) {
		if (_whole_circle / 2 >= _theta.lower() && -_whole_circle / 2 <= _theta.upper()) {
			// wraparound likely
			const auto adjusted_ub = interval(_whole_circle) + _theta.upper();
			const auto adjusted_lb = interval(-_whole_circle) + _theta.lower();
			// use more precise adjusted version
			if (adjusted_ub.width() <= adjusted_lb.width() && _theta.lower() <= adjusted_ub.upper()) {
				_theta.assign(_theta.lower(), adjusted_ub.upper());
				return;
			} else if (adjusted_lb.lower() <= _theta.upper()) {
				_theta.assign(adjusted_lb.lower(), _theta.upper());
				return;
			}
		}
		throw std::runtime_error("denormalized angle");
	}
}


void zaimoni::circle::angle::_standard_form()
{
	if (is_whole_circle())
		{
		_theta.assign(-_whole_circle / 2, _whole_circle / 2);
		return;
		}

	// our preferred internal representation is in [-5062.5, 5062.5] but
	while (_whole_circle / 2 <= _theta.lower()) _theta -= _whole_circle;
	while (-_whole_circle / 2 >= _theta.upper()) _theta += _whole_circle;
}

static bool degree_whole_circle(zaimoni::circle::angle::interval& deg) {
	if (360.0 <= (zaimoni::circle::angle::interval(deg.upper()) - zaimoni::circle::angle::interval(deg.lower())).lower()) {
		deg.assign(-5062.5, 5062.5);
		return true;
	}
	return false;
}

// check for whole-circle in degrees
void zaimoni::circle::angle::_degree_to_standard_form()
{
	// First, check for the whole-circle special case.
	if (degree_whole_circle(_theta)) return;

	// normal form: -180<=lb<180
	static const interval scale_overflow(interval(HUGE_VAL) / 360.0);
	if (180 <= _theta.lower()) {
		do {
			int scale = 2;
			while ((scale + 1) * 180.0 <= _theta.lower() && scale < scale_overflow.lower()) scale *= 2;
			_theta -= scale * 180.0;
			if (degree_whole_circle(_theta)) return;
		} while (180 <= _theta.lower());
	} else if (-180 >= _theta.upper()) {
		do {
			int scale = 2;
			while ((scale + 1) * (-180.0) >= _theta.upper() && scale < scale_overflow.lower()) scale *= 2;
			_theta += scale * 180.0;
			if (degree_whole_circle(_theta)) return;
		} while (-180 >= _theta.upper());
	}
	// change to preferred internal representation
	(_theta*=225)/=8;
}

static bool radian_whole_circle(zaimoni::circle::angle::interval& rad) {
	if (interval_shim::pi.upper() <= (zaimoni::circle::angle::interval(rad.upper()) - zaimoni::circle::angle::interval(rad.lower())).lower()) {
		rad.assign(-5062.5, 5062.5);
		return true;
	}
	return false;
}

void zaimoni::circle::angle::_radian_to_standard_form()
{
	// First, check for the whole-circle special case.
	if (radian_whole_circle(_theta)) return;

	// normal form: -180<=lb<180
	static const interval scale_overflow(interval(HUGE_VAL) / (2*interval_shim::pi.upper()));
	if (interval_shim::pi.upper() <= _theta.lower()) {
		do {
			int scale = 2;
			while (((scale + 1) * interval_shim::pi).upper() <= _theta.lower() && scale < scale_overflow.lower()) scale *= 2;
			_theta -= scale * interval_shim::pi;
			if (radian_whole_circle(_theta)) return;
		} while (interval_shim::pi.upper() <= _theta.lower());
	}
	else if (-interval_shim::pi.upper() >= _theta.upper()) {
		do {
			int scale = 2;
			while (((scale + 1) * -interval_shim::pi).upper() <= _theta.lower() && scale < scale_overflow.lower()) scale *= 2;
			_theta += scale * interval_shim::pi;
			if (radian_whole_circle(_theta)) return;
		} while (-interval_shim::pi.upper() >= _theta.upper());
	}
	// change to preferred internal representation
	((_theta *= 1125) /= 2) /= interval_shim::pi;
}


void static enforce_circle(interval_shim::interval& _sin, interval_shim::interval& _cos)
{
	interval_shim::interval test(1.0);
	test -= square(_sin);
	test = sqrt(test);
	if (0.0>_cos.upper()) test = -test;
	else if (0.0>_cos.lower()) test.assign(-test.upper(),test.upper());	// XXX might be able to do better if 0.0<test.lower()

	assert(test.lower()<=_cos.upper());
	assert(test.upper()>=_cos.lower());
	if (_cos.lower()<test.lower() || _cos.upper()>test.upper()) _cos.assign((_cos.lower()<test.lower() ? test.lower() : _cos.lower()),(_cos.upper()>test.upper() ? test.upper() : _cos.upper()));
}

// not a proper < operator as the unit circle isn't ordered that way
// we may want to delegate to our interval's container_lt once it bootstraps
bool zaimoni::circle::angle::container_lt(const angle& rhs) const
{	// disjoint cases first
	if (_theta.upper() < rhs._theta.lower()) return true;
	if (rhs._theta.upper() < _theta.lower()) return false;
	// if not disjoint, go with lexical order
	if (_theta.lower() < rhs._theta.lower()) return true;
	if (rhs._theta.lower() < _theta.lower()) return false;
	if (_theta.upper() < rhs._theta.upper()) return true;
//	if (rhs._theta.upper() < _theta.upper()) return false;
	return false;
}


zaimoni::circle::angle zaimoni::circle::angle::operator+=(const angle& src)
{
	_theta += src._theta;
	_standard_form();
	return *this;
}

zaimoni::circle::angle zaimoni::circle::angle::operator-=(const angle& src)
{
	_theta -= src._theta;
	_standard_form();
	return *this;
}

zaimoni::circle::angle& zaimoni::circle::angle::operator*=(const interval& rhs)
{
	interval lb(_theta.lower());
	interval ub(_theta.upper());
	lb *= rhs;
	ub *= rhs;
	_theta.assign(lb.lower(), ub.upper());
	_standard_form();
	return *this;
}

zaimoni::circle::angle& zaimoni::circle::angle::operator*=(typename const_param<interval::base_type>::type rhs)
{
	interval lb(_theta.lower());
	interval ub(_theta.upper());
	lb *= rhs;
	ub *= rhs;
	_theta.assign(lb.lower(), ub.upper());
	_standard_form();
	return *this;
}

void zaimoni::circle::angle::self_scalBn(int scale)
{
	if (0 == scale || 0 == _theta) return;
	if (is_whole_circle()) return;	// \todo is it clear we shouldn't suddenly undegenerate?
	if (0 > scale) {
		_theta = scalBn(_theta, scale);
		return;	// will be in range, although underflow is a consideration
	}
	// remaining near standard form is an issue
	while (0 < scale) {
		_theta = scalBn(_theta, 1);
		_standard_form();
		if (is_whole_circle()) return;	// bail if we degenerate
		--scale;
	}
}

void zaimoni::circle::angle::_radian_sincos(interval radians, interval& _sin, interval& _cos)
{
	_sin = zaimoni::math::sin().eval(radians);	// using Taylor series as Boost compile-errors here
	_cos = zaimoni::math::cos().eval(radians);
//	\todo post-processing
	// since we know the domain is the real numbers, we have some additional specializations:
	// * range is [-1,1]
	// sin^2+cos^2 = 1
	assert(1.0>=_sin.lower());
	assert(1.0>=_cos.lower());
	assert(-1.0>=_sin.upper());
	assert(-1.0>=_cos.upper());
	_sin.self_intersect(sin_cos_range_for_real_domain);
	_cos.self_intersect(sin_cos_range_for_real_domain);

	enforce_circle(_sin,_cos);
	enforce_circle(_cos,_sin);
}

void zaimoni::circle::angle::_apply_180_degrees_shift(interval x, interval& _sin, interval& _cos)
{
	interval tmp_sin;
	interval tmp_cos;
	_sincos(x, tmp_sin, tmp_cos);
	_sin.self_union(-tmp_sin);
	_cos.self_union(-tmp_cos);
}

void zaimoni::circle::angle::_apply_sector(interval x, interval& _sin, interval& _cos)
{
	interval tmp_sin;
	interval tmp_cos;
	_sincos(x, tmp_sin, tmp_cos);
	_sin.self_union(tmp_sin);
	_cos.self_union(tmp_cos);
}

void zaimoni::circle::angle::_apply_y_reflect(interval x, interval& _sin, interval& _cos)
{
	interval tmp_sin;
	interval tmp_cos;
	_sincos(x, tmp_sin, tmp_cos);
	_sin.self_union(tmp_sin);
	_cos.self_union(-tmp_cos);
}

void zaimoni::circle::angle::_apply_x_reflect(interval x, interval& _sin, interval& _cos)
{
	interval tmp_sin;
	interval tmp_cos;
	_sincos(x, tmp_sin, tmp_cos);
	_sin.self_union(-tmp_sin);
	_cos.self_union(tmp_cos);
}

// x is in the internal representation
void zaimoni::circle::angle::_sincos(interval x, interval& _sin, interval& _cos)
{
	// Quadrant II wrap
	if (_whole_circle/2 < x.upper()) {	// spans x-axis
		assert(_whole_circle / 2 > x.lower());	// assume normalized
		interval Q3(_whole_circle / 2, x.upper());
		interval Q2(x.lower(), _whole_circle / 2);
		_sincos(Q2, _sin, _cos);
		_apply_180_degrees_shift(Q3 -= _whole_circle / 2, _sin, _cos);
		return;
	} else if (_whole_circle / 2 == x.upper()) {	// ends on x-axis
		if (_whole_circle / 4 >= x.lower()) {
			_sin.assign(0, 1);
			_cos.assign(-1, 0);
			if (_whole_circle / 4 > x.lower()) {
				x.assign(x.lower(), _whole_circle / 4);
				_apply_sector(x, _sin, _cos);
			}
			return;
		}
		interval Q1(0, (_whole_circle / 2 - interval(x.lower())).upper());
		_apply_y_reflect(Q1, _sin, _cos);
		return;
	}

	// Quadrant III wrap
	if (-_whole_circle / 2 > x.lower()) {	// spans x-axis
		assert(-_whole_circle / 2 < x.upper());	// assume normalized
		interval Q3(-_whole_circle / 2, x.upper());
		interval Q2(x.lower(), -_whole_circle / 2);
		_sincos(Q3, _sin, _cos);
		_apply_180_degrees_shift(Q2 += _whole_circle / 2, _sin, _cos);
		return;
	} else if (-_whole_circle / 2 == x.lower()) {	// ends on x-axis
		if (-_whole_circle / 4 <= x.upper()) {
			_sin.assign(0, -1);
			_cos.assign(-1, 0);
			if (-_whole_circle / 4 < x.upper()) {
				x.assign(-_whole_circle / 4, x.upper());
				_apply_sector(x, _sin, _cos);
			}
			return;
		}
		_sincos(-_whole_circle / 2 - x, _sin, _cos);
		_cos.self_negate();
		return;
	}
	// domain is now (-_whole_circle / 2, _whole_circle / 2)

	// we want that alternating series to alternate
	if (0 > x.lower()) {
		if (0 >= x.upper()) {
			_sincos(-x, _sin, _cos);
			_sin.self_negate();
			return;
		}
		interval Q1Q2(0, -x.lower());
		x.assign(0, x.upper());
		_sincos(x, _sin, _cos);
		_apply_x_reflect(Q1Q2, _sin, _cos);
		return;
	}
	// domain is now [0, _whole_circle / 2)
	if (_whole_circle / 4 < x.upper()) {
		if (_whole_circle / 4 <= x.lower()) {
			_sincos(_whole_circle / 2 - x, _sin, _cos);
			_cos.self_negate();
			return;
		}
		_sincos(interval(x.lower(), _whole_circle / 4), _sin, _cos);
		_apply_y_reflect(interval((_whole_circle / 2 - x).lower(), _whole_circle / 4), _sin, _cos);
		return;
	} else if (_whole_circle / 4 == x.upper()) {
		if (0 == x.lower()) {
			_sin.assign(0, 1);
			_cos.assign(0, 1);
			return;
		}
	}
	// domain is now [0, _whole_circle / 4]
	if (_whole_circle / 8 <= x.lower()) {
		_sincos(_whole_circle / 8 - x, _sin, _cos);
		swap(_sin, _cos);
		return;
	} else if (_whole_circle / 8 < x.upper()) {
		_sincos(interval((_whole_circle / 8 - x).lower(), _whole_circle / 8), _sin, _cos);
		swap(_sin, _cos);
		x.assign(x.lower(), _whole_circle / 8);
		_apply_sector(x, _sin, _cos);
		return;
	}
	// domain is now [0, _whole_circle / 8]
	if (x.lower() == x.upper()) {
		_internal_sincos(x.lower(), _sin, _cos);
		return;
	}
//	go to radians
	_radian_sincos(((x*2.0)*interval_shim::pi)/1125.0, _sin, _cos);
}

static const auto SQRT3_2 = interval_shim::SQRT3 / 2;	// these belong in interval_shim
static const auto SQRT2_2 = interval_shim::SQRT2 / 2;
// also can have entries for 18 degrees and 36 degrees, from the pentagon construction
// { reference angle, {sin, cos} }
static const std::pair<interval_shim::interval::base_type, std::pair<interval_shim::interval, interval_shim::interval> > ref_trig[] = {
	{0, {0, 1}},	// 0 degrees
	{_whole_circle / 12, {0.5, SQRT3_2}},	// 30 degrees
	{_whole_circle / 8, {SQRT2_2, SQRT2_2}}	// 45 degrees
};

// can't actually do final stage, zaimoni::retype<zaimoni::circle::angle>: private constructor
static const auto ref_angles = zaimoni::math::reflect_about_front(
	zaimoni::math::reflect_about_back(
	zaimoni::math::reflect_about_back(
	zaimoni::math::restrict_to_axis<STATIC_SIZE(ref_trig), 0>(ref_trig))));

void zaimoni::circle::angle::_internal_sincos(typename const_param<interval::base_type>::type x, interval& _sin, interval& _cos)
{
	assert(0 <= x && _whole_circle / 8 >= x);
	auto i = STATIC_SIZE(ref_trig);
	do {
		auto& tmp = ref_trig[--i];
		if (x == tmp.first) {
			_sin = tmp.second.first;
			_cos = tmp.second.second;
			return;
		} else if (x > tmp.first) break;
	} while (0 < i);

	_radian_sincos(((x * 2.0) * interval_shim::pi) / 1125.0, _sin, _cos);
	_sin.self_intersect(interval(ref_trig[i].second.first.lower(), ref_trig[i+1].second.first.upper()));
	_cos.self_intersect(interval(ref_trig[i+1].second.second.lower(), ref_trig[i].second.second.upper()));
}

std::vector<zaimoni::circle::angle> zaimoni::circle::angle::contains_ref_angles() const
{
	std::vector<angle> ret;
	for (const auto& x : ref_angles) if (_theta.contains(x)) ret.push_back(angle(x));
	return std::move(ret);
}

size_t zaimoni::circle::angle::contains_ref_angles(angle* dest) const
{
	assert(dest);
	size_t n = 0;
	for (const auto& x : ref_angles) if (_theta.contains(x)) {
		*(dest++) = angle(x);
		n++;
	};
	return n;
}


void zaimoni::circle::angle::sincos(interval& _sin, interval& _cos) const
{
	if (is_whole_circle())
		{
		_sin = sin_cos_range_for_real_domain;
		_cos = sin_cos_range_for_real_domain;
		return;
		}
	_sincos(_theta,_sin,_cos);
}

#ifdef TEST_APP2
// fast compile test
// g++ -std=c++11 -oangle.exe -Os -DTEST_APP2 -D__STDC_LIMIT_MACROS angle.cpp taylor.cpp
// If doing INFORM-based debugging
// g++ -std=c++11 -oangle.exe -Os -DTEST_APP2 -D__STDC_LIMIT_MACROS angle.cpp taylor.cpp -Llib/host.isk -lz_log_adapter -lz_stdio_log -lz_format_util

// console-mode application
#define STRING_LITERAL_TO_STDOUT(A) fwrite(A,sizeof(A)-1,1,stdout)
#define C_STRING_TO_STDOUT(A) fwrite(A,strlen(A),1,stdout)
#define STL_STRING_TO_STDOUT(A) fwrite((A).data(),(A).size(),1,stdout)
#define STL_PTR_STRING_TO_STDOUT(A) fwrite((A)->data(),(A)->size(),1,stdout)

int main(int argc, char* argv[])
{
	STRING_LITERAL_TO_STDOUT("starting main\n");

	zaimoni::circle::angle test_deg(typename zaimoni::circle::angle::degree(0));
	zaimoni::circle::angle test_rad(typename zaimoni::circle::angle::radian(0));

	STRING_LITERAL_TO_STDOUT("raw reference angles\n");
	for (auto x : ref_angles) INFORM(x);

	assert(zaimoni::circle::angle::ref_angle_maxsize == ref_angles.size());

	STRING_LITERAL_TO_STDOUT("tests finished\n");

	return 0;
}
#endif
