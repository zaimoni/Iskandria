// game_manager.cpp

#include "game_manager.hpp"

#include "world_manager.hpp"
#include "display_manager.hpp"
#include "input_manager.hpp"

#include "Zaimoni.STL/Pure.C/format_util.h"
#include "Zaimoni.STL/Pure.C/logging.h"
#include "Zaimoni.STL/OS/Clock.hpp"
#include "Zaimoni.STL/OS/usleep.h"

#include "singleton.on.hpp"

namespace isk {

ISK_SINGLETON_BODY(GameManager)

// Windows: holding off on timeBeginPeriod(1), timeEndPeriod(1) calls
GameManager::GameManager()
:	game_over(false),
	is_paused(false),
	was_paused(false),
  	frame_time_ms(1000/30)
{
	start_logfile("stderr.txt");	// XXX configuration target
}

GameManager::~GameManager()
{
	end_logfile();
}

// mockup
void GameManager::run()
{
	WorldManager& cosmos = WorldManager::get();		// these destructors need to run late
	DisplayManager& gui = DisplayManager::get();
	InputManager& io = InputManager::get();

	// C main() or its OS-specific analog is responsible for
	// * Configuring savefile format, etc.
	// * Configuring the input manager for New game/load game/save game

	char buf[11];
	int i = 10;
	zaimoni::Clock t0;
	zaimoni::Clock t1(t0);
	long delta = t1.split();
	while(!game_over)
		{	// XXX mockup for a fast-twitch game
		t1.delta();
		was_paused = is_paused;
		io.getInput();						// has to be able to unset is_paused
		if (!is_paused) cosmos.update();	// has to be able to set is_paused
		if (!is_paused || was_paused != is_paused) {
			cosmos.draw();
			gui.swapBuffers();
		}
		delta = t1.split();
		if (delta<frameTime_ms()) {
			usleep(1000*(frameTime_ms()-delta));
			t1.delta();
		}
		if (0>= --i) game_over = true;
		}
	inc_log("total runtime: ");
	inc_log(z_umaxtoa(t0.delta(),buf,10));
	add_log(" ms");	// assumes Windows
}

}	// namespace isk

#ifdef TEST_DRIVER
// fast compile test
// g++ -std=c++11 -otest.exe -Os -DTEST_DRIVER game_manager.cpp world_manager.cpp input_manager.cpp display_manager.cpp -Llib/host.isk  -lz_logging -lz_format_util -lz_clock  -lsfml-graphics -lsfml-window -lsfml-system -lwinmm 
int main(int argc, char* argv[])
{
	isk::GameManager::get().run();
	return 0;
}
#endif
