// input_manager.hpp

#ifndef INPUT_MANAGER_HPP
#define INPUT_MANAGER_HPP 1

#include <vector>
#include "text_menu.hpp"
#include "singleton.on.hpp"

namespace isk {

// Cf. http://dragonfly.wpi.edu/book/

class InputManager
{
ISK_SINGLETON_HEADER(InputManager);
private:
	std::vector<textmenu> menus;
	bool cancel_menu;
public:
	void getInput();

	void install(const textmenu& src) { menus.push_back(src); }
	void install(textmenu&& src) { menus.push_back(std::move(src)); }

	void draw() const;

	static bool close_menu() { get().cancel_menu = true; return true; }
};

}	// namespace isk

#include "singleton.off.hpp"

#endif
