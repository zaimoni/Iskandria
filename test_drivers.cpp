#include "input_manager.hpp"

// VAPORWARE: isometric view test driver, with a boardable AA gun
static bool AA_chessboard()
{
	isk::InputManager::close_menu();	// request closing our invoking menu only

	// \todo load required grid tiles
	// \todo set up map data mockup and camera viewpoint
	// \todo install command processing menu
	auto terminate_menu = [=]() {	// must capture by value so that std::weak_ptrs don't wink out prematurely
		return isk::InputManager::close_menus();	// close everything at/above our menu
	};

	isk::textmenu AA_map_controls;
	sf::Event::KeyEvent hotkey = { sf::Keyboard::Key::Escape, false, false, false, false };
	AA_map_controls.add_entry("Esc)ape", hotkey, terminate_menu);	// remove this once we have real content
	AA_map_controls.add_entry(hotkey, terminate_menu);
	isk::InputManager::get().install(AA_map_controls);
	return true;
}

std::function<bool(void)> test_drivers()	// VAPORWARE referenced in main.cpp
{	// VAPORWARE: installs a menu indexing test driver options when run.
	return AA_chessboard;
}

