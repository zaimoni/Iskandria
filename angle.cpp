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

// check for whole-circle in degrees
bool zaimoni::circle::angle::_deg_to_whole_circle()
{
	boost::numeric::interval<double> tmp(_theta.upper());
	tmp -= _theta.lower();
	if (360<=tmp.lower())
		{
		_theta.assign(-5062.5,5062.5);
		return true;
		}
	return false;
}

void zaimoni::circle::angle::_to_standard_form()
{
	// First, check for the whole-circle special case.
	if (_deg_to_whole_circle()) return;

	// normal form: -180<=lb<180
	while(180<=_theta.lower())
		{
		int scale = 2;
		while((scale+1)*180.0<=_theta.lower() && scale<(HUGE_VAL/360.0)) scale*=2;
		_theta -= scale*180.0;
		if (_deg_to_whole_circle()) return;
		}
	// normal form: -180<ub<=180
	while(-180>=_theta.upper())
		{
		int scale = 2;
		while((scale+1)*(-180.0)>=_theta.upper() && scale<(HUGE_VAL/360.0)) scale*=2;
		_theta += scale*180.0;
		if (_deg_to_whole_circle()) return;
		}
	// change to preferred internal representation
	(_theta*=225)/=8;
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
		_sincos(interval(x.lower()-5062.5,x.upper()-5062.5),_sin,_cos);
		_sin.assign(-_sin.upper(),-_sin.lower());
		_cos.assign(-_cos.upper(),-_cos.lower());
		return;
		}
	if (-5062.5 <= x.lower() && -2531.25>=x.upper())
		{	// quadrant III -> quadrant I
		_sincos(interval(x.lower()+5062.5,x.upper()+5062.5),_sin,_cos);
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

	zaimoni::circle::angle test(0);;

	STRING_LITERAL_TO_STDOUT("tests finished\n");

	return 0;
}
#endif
