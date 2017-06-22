// game_manager.cpp

#include "game_manager.hpp"

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
	char buf[11];
	int i = 10;
	zaimoni::Clock t0;
	zaimoni::Clock t1(t0);
	long delta = t1.split();
	while(!game_over)
		{	// XXX mockup for a fast-twitch game
		delta = t1.delta();
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

#ifdef TEST_APP
// fast compile test
// g++ -otest.exe -DTEST_APP game_manager.cpp -Llib/host.isk  -lz_logging -lz_format_util -lz_clock -lwinmm
int main(int argc, char* argv[])
{
	isk::GameManager::get().run();
	return 0;
}
#endif
