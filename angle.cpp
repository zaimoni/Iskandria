// angle.cpp

#include "angle.hpp"

#include "taylor.hpp"

void zaimoni::circle::angle::_standard_form()
{
	if (is_whole_circle())
		{
		_theta.assign(-5062.5,5062.5);
		return;
		}

	// our preferred internal representation is in [-5062.5, 5062.5] but
	while( 10125 < _theta.upper()) _theta -= 10125;
	while(-10125 > _theta.lower()) _theta += 10125;
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

void zaimoni::circle::angle::_radian_sincos(interval radians, interval& _sin, interval& _cos)
{
	_sin = zaimoni::math::sin().template eval(radians);	// using Taylor series as Boost compile-errors here
	_cos = zaimoni::math::cos().template eval(radians);
//	\todo post-processing
	// since we know the domain is the real numbers, we have some additional specializations:
	// * range is [-1,1]
	// sin^2+cos^2 = 1
	assert(1.0>=_sin.lower());
	assert(1.0>=_cos.lower());
	assert(-1.0>=_sin.upper());
	assert(-1.0>=_cos.upper());
	if (-1.0>_sin.lower() || 1.0<_sin.upper()) _sin.assign((-1.0>_sin.lower() ? -1.0 : _sin.lower()),(1.0<_sin.upper() ? 1.0 : _sin.upper()));
	if (-1.0>_cos.lower() || 1.0<_cos.upper()) _cos.assign((-1.0>_cos.lower() ? -1.0 : _cos.lower()),(1.0<_cos.upper() ? 1.0 : _cos.upper()));

	enforce_circle(_sin,_cos);
	enforce_circle(_cos,_sin);
}

// x is in the internal representation
void zaimoni::circle::angle::_sincos(interval x, interval& _sin, interval& _cos)
{
//	\todo preprocessing
	if (5062.5 >= x.upper() && 2531.25<=x.lower())
		{	// quadrant II -> quadrant IV
		x -= 5062.5;
		_sincos(x,_sin,_cos);
		_sin.assign(-_sin.upper(),-_sin.lower());
		_cos.assign(-_cos.upper(),-_cos.lower());
		return;
		}
	if (-5062.5 <= x.lower() && -2531.25>=x.upper())
		{	// quadrant III -> quadrant I
		x += 5062.5;
		_sincos(x,_sin,_cos);
		_sin.assign(-_sin.upper(),-_sin.lower());
		_cos.assign(-_cos.upper(),-_cos.lower());
		return;
		}
//	go to radians
	_radian_sincos(((x*2.0)*interval_shim::pi)/1125.0, _sin, _cos);
}

void zaimoni::circle::angle::sincos(interval& _sin, interval& _cos) const
{
	if (is_whole_circle())
		{
		_sin.assign(-1.0,1.0);
		_cos.assign(-1.0,1.0);
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

	STRING_LITERAL_TO_STDOUT("tests finished\n");

	return 0;
}
#endif
