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

#ifdef TEST_APP
// fast compile test
// g++ -std=c++11 -otest.exe -Os -DTEST_APP angle.cpp
int main(int argc, char* argv[])
{
	zaimoni::circle::angle test(0);;
	return 0;
}
#endif
