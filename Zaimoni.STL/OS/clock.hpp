// clock.hpp

#ifndef ZAIMONI_OS_CLOCK_HPP
#define ZAIMONI_OS_CLOCK_HPP 1

namespace zaimoni {

// cf. http://dragonfly.wpi.edu/book/
class Clock {
private :
	unsigned long m_previous_time;

	static unsigned long n_seconds();	// wraps OS-specific implementation
public:
	static const unsigned int scale;	// how many units per second
	Clock() : m_previous_time(n_seconds()) {};

	long delta() {
		unsigned long tmp = m_previous_time;
		return (m_previous_time =  n_seconds())-tmp;
	};
	long split() const {
		return n_seconds()-m_previous_time;
	};
};

}	// end namespace zaimoni

#endif


