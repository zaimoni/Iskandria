#include "input_manager.hpp"

// VAPORWARE: isometric view test driver, with a boardable AA gun
static bool AA_chessboard()
{
	isk::InputManager::close_menu();	// request closing our menu

	// \todo load required grid tiles
	// \todo set up map data mockup and camera viewpoint
	// \todo install command processing menu
	isk::textmenu AA_map_controls;
	sf::Event::KeyEvent hotkey = { sf::Keyboard::Key::Escape, false, false, false, false };
	AA_map_controls.add_entry("E)scape", hotkey, isk::InputManager::close_menu);	// remove this once we have real content
	AA_map_controls.add_entry(hotkey, isk::InputManager::close_menu);	// wrap this in lambda function responsible for cleanup
	isk::InputManager::get().install(AA_map_controls);
	return false;	// but not all menus
}

std::function<bool(void)> test_drivers()	// VAPORWARE referenced in main.cpp
{	// VAPORWARE: installs a menu indexing test driver options when run.
	return AA_chessboard;
}

