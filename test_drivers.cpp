#include "input_manager.hpp"
#include "display_manager.hpp"

// VAPORWARE: isometric view test driver, with a boardable AA gun
static bool AA_chessboard()
{
	isk::InputManager::close_menu();	// request closing our invoking menu only

	// \todo load required grid tiles
	// for now, exercise the CGI image generation
	auto w_floor = isk::DisplayManager::get().getImage("cgi:floor:#CCC");
	auto g_floor = isk::DisplayManager::get().getImage("cgi:floor:#999999");
	auto w_wall_front = isk::DisplayManager::get().getImage("cgi:lwall:#F00");
	auto w_wall_back = isk::DisplayManager::get().getImage("cgi:rwall:#00FF00");
	auto n_wall_front = isk::DisplayManager::get().getImage("cgi:rwall:#0000FF");
	auto n_wall_back = isk::DisplayManager::get().getImage("cgi:lwall:#FFFF00");

	// \todo set up map data mockup and camera viewpoint
	// \todo install command processing menu
	auto terminate_menu = [=]() mutable {	// must capture by value so that std::weak_ptrs don't wink out prematurely
		w_floor.reset();
		g_floor.reset();
		w_wall_front.reset();
		w_wall_back.reset();
		n_wall_front.reset();
		n_wall_back.reset();
		return isk::InputManager::close_menus();	// close everything at/above our menu (fails loop cycling)
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

