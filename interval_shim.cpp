#include "interval_shim.hpp"

#ifndef CONSTANTS_ISK_INTERVAL
#include <cmath>	// do not need augmentations here
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

const interval_shim::interval interval_shim::pi(M_PI, nextafter(M_PI, 4));	// CRC Handbook says floating point representation of M_PI is "low"; this brackets

