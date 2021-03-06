// input_manager.cpp

#include "input_manager.hpp"
#include "display_manager.hpp"
#include "game_manager.hpp"
#include "singleton.on.hpp"

std::function<bool(void)> test_drivers();	// defined in test_drivers.cpp

namespace isk {

ISK_SINGLETON_BODY(InputManager)

static const textmenu& start_menu()
{
	static textmenu* oaoo = 0;
	if (!oaoo) {
		oaoo = new isk::textmenu(true);
		auto tests = test_drivers();

		sf::Event::KeyEvent hotkey = { sf::Keyboard::Key::T, false, false, false, false };
		oaoo->add_entry("T)est", hotkey, tests);	// eventually provides access to various test drivers
		hotkey.shift = true;
		oaoo->add_entry(hotkey, tests);

		hotkey = { sf::Keyboard::Key::Q, false, false, false, false };
		oaoo->add_entry("Q)uit", hotkey, isk::GameManager::quit_handler);
		hotkey.shift = true;
		oaoo->add_entry(hotkey, isk::GameManager::quit_handler);
	}
	return *oaoo;
}

static void force_start_menu(InputManager& x) { x.install(start_menu()); }

InputManager::InputManager()
{
	DisplayManager::get().getWindow()->setKeyRepeatEnabled(false);
	force_start_menu(*this);
}

InputManager::~InputManager()
{
	DisplayManager::get().getWindow()->setKeyRepeatEnabled(true);
}

void InputManager::getInput()
{
	sf::Event e;

	// general idea is that we have to register "control scheme"/PC agent pairs
	// for each control scheme, we have commands directed at the UI (e.g., context-sensitive help)
	// and commands directed at the PC (e.g., move commands)

	while(DisplayManager::get().getWindow()->pollEvent(e)) {
		switch(e.type)
		{
		case sf::Event::Closed:
			GameManager::quit_handler();
			return;
		case sf::Event::Resized:
			DisplayManager::get().resize(e.size.width, e.size.height);
			// \todo request reflowing
			// note that this does *not* change the coordinates SFML reports to us, we have to convert to physical pixels from logical pixels
			break;
//		case sf::Event::LostFocus: break;         
//		case sf::Event::GainedFocus: break;       
//		case sf::Event::TextEntered: break;	// implement this once we know what the KeyReleased one does
//		case sf::Event::KeyPressed: break;	// usually ignore this one
		case sf::Event::KeyReleased:
			{
			size_t ub = menus.size();
			while (0 < ub) {
				cancel_menu = false;
				cancel_menus = false;
				if (menus[--ub].handle(e.key)) {
					if (cancel_menus) {
						while(menus.size() > ub) menus.pop_back();	// this menu is done
					} else if (cancel_menu) menus.erase(menus.begin() + ub);
					ub = 0;	// done
					if (GameManager::get().gameOver()) return;	// hard-stop
					// \todo do menus need a modal option that lets them block lower menus, or is this subsumed by pause?
				}
				if (GameManager::get().isPaused()) break;	// the unpause menu will be the very top menu
			}
			if (menus.empty()) force_start_menu(*this);
			}
			break;
//		case sf::Event::MouseWheelMoved: break;
//		case sf::Event::MouseWheelScrolled: break;
//		case sf::Event::MouseButtonPressed: break;
//		case sf::Event::MouseButtonReleased: break;
//		case sf::Event::MouseMoved: break;
//		case sf::Event::MouseEntered: break;
//		case sf::Event::MouseLeft: break;
//		case sf::Event::JoystickButtonPressed: break;
//		case sf::Event::JoystickButtonReleased: break;
//		case sf::Event::JoystickMoved: break;
//		case sf::Event::JoystickConnected: break;
//		case sf::Event::JoystickDisconnected: break;
//		case sf::Event::TouchBegan: break;
//		case sf::Event::TouchMoved: break;
//		case sf::Event::TouchEnded: break;
//		case sf::Event::SensorChanged: break;
		default: continue;	// something we don't handle
		}
	}
}

void InputManager::draw() const
{
	for (auto& x : menus) x.draw();
}

}	// namespace isk

#ifdef TEST_APP2
// fast compile test
// g++ -std=c++11 -otest.exe -Os -DTEST_APP2 -DSFML_STATIC input_manager.cpp display_manager.cpp -lsfml-graphics-s -lsfml-window-s -lsfml-system-s
// g++ -std=c++11 -otest.exe -Os -DTEST_APP2 input_manager.cpp display_manager.cpp -lsfml-graphics -lsfml-window -lsfml-system
int main(int argc, char* argv[])
{
	isk::InputManager::get();
	return 0;
}
#endif
