#include "interval_shim.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "test_driver.h"

int main(int argc, char* argv[])
{	// parse options
	char buf[100];

	STRING_LITERAL_TO_STDOUT("starting main\n");

	const interval_shim::interval ref_pi(M_PI, nextafter(M_PI, 4));	// CRC Handbook says floating point representation of M_PI is "low"; this brackets
	INFORM(interval_shim::pi);
	INFORM(ref_pi);
	INFORM(interval_shim::pi.lower() == ref_pi.lower() ? "true" : "false");
	INFORM(interval_shim::pi.upper() == ref_pi.upper() ? "true" : "false");

	STRING_LITERAL_TO_STDOUT("tests finished\n");
}


