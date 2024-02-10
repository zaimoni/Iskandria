#include "clock.hpp"
#include <chrono>

namespace zaimoni {

const unsigned int Clock::scale = 1000; // how many units per second

unsigned long Clock::n_seconds()
{
	using clock = std::chrono::steady_clock;
	static clock::time_point startTime = clock::now();

	return std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - startTime).count();
}

}
