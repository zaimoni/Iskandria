// conventions.hpp

#ifndef CONVENTIONS_HPP
#define CONVENTIONS_HPP

#include <boost/numeric/interval.hpp>

namespace iskandria {

// The general relativistic invariant: the time experienced by a clock that thinks the object is *always* at rest.
typedef boost::numeric::interval<double> proper_time;

typedef boost::numeric::interval<double> rest_mass;

}

#endif
