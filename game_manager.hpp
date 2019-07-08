// game_manager.hpp

#ifndef GAME_MANAGER_HPP
#define GAME_MANAGER_HPP 1

#include "singleton.on.hpp"

namespace isk {

// Cf. http://dragonfly.wpi.edu/book/
class GameManager {
ISK_SINGLETON_HEADER(GameManager);
private:
	bool game_over;
	bool is_paused;
	bool was_paused;
	int frame_time_ms;
public:
	void run();

	void set_gameOver(bool src=true) {game_over = src;}
	bool gameOver() const {return game_over;}

	void set_isPaused(bool src = true) { is_paused = src; }
	bool isPaused() const { return is_paused; }

	int frameTime_ms() const {return frame_time_ms;}	// XXX setter to be feature-complete

	static bool quit_handler();
};

}	// namespace isk

#include "singleton.off.hpp"

#endif

