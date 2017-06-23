// display_manager.hpp

#ifndef DISPLAY_MANAGER_HPP
#define DISPLAY_MANAGER_HPP 1

#include <SFML/Graphics.hpp>

#include "singleton.on.hpp"

namespace isk {

// Cf. http://dragonfly.wpi.edu/book/

// yes, the library is bleeding through the interface.  Refactor when a second well-tested library is license-viable.

class DisplayManager
{
ISK_SINGLETON_HEADER(DisplayManager);
private:
	sf::Font* _font;
	sf::RenderWindow* _window;
	sf::Color _background;
	int _width_pixels;
	int _height_pixels;
	int _width_chars;
	int _height_chars;
public:
	void swapBuffers();
	sf::RenderWindow* getWindow() const { return _window; };
	int width_pixel() const { return _width_pixels;};
	int height_pixel() const { return _height_pixels;};

	int width_char() const { return _width_chars;};
	int height_char() const { return _height_chars;};

//	int drawCh(int x, int y, char ch, sf::Color color);

	float charWidth() const { return (float)(_width_pixels)/(float)(_width_chars);};
	float charHeight() const { return (float)(_height_pixels)/(float)(_height_chars);};
};

#include "singleton.off.hpp"

}	// namespace isk

#endif
