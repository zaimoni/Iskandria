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
	int frame_time_ms;
public:
	void run();

	void set_gameOver(bool src=true) {game_over = src;}
	bool gameOver() const {return game_over;}

	int frameTime_ms() const {return frame_time_ms;}	// XXX setter to be feature-complete
};

}	// namespace isk

#include "singleton.off.hpp"

#endif

