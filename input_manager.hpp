// input_manager.hpp

#ifndef INPUT_MANAGER_HPP
#define INPUT_MANAGER_HPP 1

#include "singleton.on.hpp"

namespace isk {

// Cf. http://dragonfly.wpi.edu/book/

class InputManager
{
ISK_SINGLETON_HEADER(InputManager);
private:
public:
	void getInput();
};

}	// namespace isk

#include "singleton.off.hpp"

#endif
