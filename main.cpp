// main.cpp

#include "game_manager.hpp"
#include "gridspace.hpp"
#include "minkowski.hpp"
#include "agent.hpp"
#include "craft.hpp"
#include "wrap.hpp"
#include "text_menu.hpp"

std::function<bool(void)> test_drivers();	// defined in test_drivers.cpp

int main(int argc, char* argv[])
{
	// Order of world registration of game objects, is their order in the savefile.
	isk::Wrap<iskandria::grid::cartesian<2> >::world_setup();
	isk::Wrap<iskandria::grid::cartesian<3> >::world_setup();
	isk::Wrap<iskandria::minkowski>::world_setup();
	isk::Wrap<iskandria::agent>::world_setup();	// only legitimate source of PC control, try not to have ECS Systems acting before this point
	isk::Wrap<iskandria::craft>::world_setup();

	// start the game engine
	isk::GameManager::get().run();
	return 0;
}

