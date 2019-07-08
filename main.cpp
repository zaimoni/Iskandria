// main.cpp

#include "game_manager.hpp"
#include "input_manager.hpp"
#include "gridspace.hpp"
#include "minkowski.hpp"
#include "agent.hpp"
#include "craft.hpp"
#include "wrap.hpp"
#include "text_menu.hpp"

int main(int argc, char* argv[])
{
	// Order of world registration of game objects, is their order in the savefile.
	isk::Wrap<iskandria::grid::cartesian_2d>::world_setup();
	isk::Wrap<iskandria::minkowski>::world_setup();
	isk::Wrap<iskandria::agent>::world_setup();	// only legitimate source of PC control, try not to have ECS Systems acting before this point
	isk::Wrap<iskandria::craft>::world_setup();

	// configure the game start menu and install to the input manager (possibly natural singleton?)  function target here
	isk::textmenu start_game;
	sf::Event::KeyEvent hotkey = { sf::Keyboard::Key::Q, false, false, false, false };
	start_game.add_entry("Q)uit", hotkey, isk::GameManager::quit_handler);
	hotkey.shift = true;
	start_game.add_entry(hotkey, isk::GameManager::quit_handler);

	isk::InputManager::get().install(start_game);

	// start the game engine
	isk::GameManager::get().run();
	return 0;
}

