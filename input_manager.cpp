// input_manager.cpp

#include "input_manager.hpp"

#include "display_manager.hpp"

#include "singleton.on.hpp"

namespace isk {

ISK_SINGLETON_BODY(InputManager)

InputManager::InputManager()
{
	DisplayManager::get().getWindow()->setKeyRepeatEnabled(false);
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
//		case sf::Event::Closed: break;
//		case sf::Event::Resized: break;
//		case sf::Event::LostFocus: break;         
//		case sf::Event::GainedFocus: break;       
//		case sf::Event::TextEntered: break;     
//		case sf::Event::KeyPressed: break;     
//		case sf::Event::KeyReleased: break;
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