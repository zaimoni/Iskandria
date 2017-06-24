// angle.hpp

#ifndef ANGLE_HPP
#define ANGLE_HPP 1

#include <boost/numeric/interval.hpp>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace zaimoni {
namespace circle {

// Object here is to make the periodicity of trigonometric functions exactly represented
// 360 degrees is 360*60*60 seconds: 2^7 is a factor
// so representing 360 degrees as 10125 is fine; scaling is *225/8

class angle {
private:
	boost::numeric::interval<double> _theta;
public:
	explicit angle(boost::numeric::interval<double> degrees)
	: _theta(degrees) {_to_standard_form();};
	angle(const angle& src) : _theta(src._theta) {};
	// default destructor ok
	// default assignment probably ok

	bool is_whole_circle() const {return 10125<=_theta.upper()-_theta.lower();};
	boost::numeric::interval<double> degrees() const {return (_theta*8.0)/225.0;}
//	XOPEN standard provides M_PI, not ISO C
//	boost::numeric::interval<double> radians() const {return (((_theta*8)/225)/180)*M_PI;}
	boost::numeric::interval<double> radians() const {return ((_theta*2.0)*M_PI)/1125.0;}
private:
	void _standard_form();
	bool _deg_to_whole_circle();
	void _to_standard_form();
};

}	// namespace circle
}	// namespace zaimoni

#endif
