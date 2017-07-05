// angle.cpp

#include "angle.hpp"

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

#if 0
void zaimoni::circle::angle::_radian_sincos(boost::numeric::interval<double> radians, boost::numeric::interval<double>& _sin, boost::numeric::interval<double>& _cos)
{
	_sin = sin(radians);	// Boost fails on these two lines.  Simulate with Taylor series?
	_cos = cos(radians);
//	\todo post-processing
}

// x is in the internal representation
void zaimoni::circle::angle::_sincos(boost::numeric::interval<double> x, boost::numeric::interval<double>& _sin, boost::numeric::interval<double>& _cos)
{
//	\todo preprocessing
	if (5062.5 >= x.upper() && 2531.25<=x.lower())
		{	// quadrant II -> quadrant IV
		_sincos(boost::numeric::interval<double>(x.lower()-5062.5,x.upper()-5062.5),_sin,_cos);
		_sin.assign(-_sin.upper(),-_sin.lower());
		_cos.assign(-_cos.upper(),-_cos.lower());
		return;
		}
	if (-5062.5 <= x.lower() && -2531.25>=x.upper())
		{	// quadrant III -> quadrant I
		_sincos(boost::numeric::interval<double>(x.lower()+5062.5,x.upper()+5062.5),_sin,_cos);
		_sin.assign(-_sin.upper(),-_sin.lower());
		_cos.assign(-_cos.upper(),-_cos.lower());
		return;
		}
//	go to radians
	_radian_sincos(((x*2.0)*M_PI)/1125.0, _sin, _cos);
}

void zaimoni::circle::angle::sincos(boost::numeric::interval<double>& _sin, boost::numeric::interval<double>& _cos)
{
	if (is_whole_circle())
		{
		_sin.assign(-1.0,1.0);
		_cos.assign(-1.0,1.0);
		return;
		}
	_sincos(_theta,_sin,_cos);
}
#endif

#ifdef TEST_APP
// fast compile test
// g++ -std=c++11 -otest.exe -Os -DTEST_APP angle.cpp
int main(int argc, char* argv[])
{
	zaimoni::circle::angle test(0);;
	return 0;
}
#endif
