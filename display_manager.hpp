// display_manager.hpp

#ifndef DISPLAY_MANAGER_HPP
#define DISPLAY_MANAGER_HPP 1

#include <SFML/Graphics.hpp>
#include "cssbox.hpp"

#include "singleton.on.hpp"

namespace isk {

// Cf. http://dragonfly.wpi.edu/book/

// yes, the SFML library is bleeding through the interface.  Refactor when a second well-tested library is license-viable.

// SFML has a concept of "subwindow".

class DisplayManager
{
ISK_SINGLETON_HEADER(DisplayManager);
private:
	sf::Color _background;
	int _width_pixels;
	int _height_pixels;
	// we should compute the pixel width from a sample character, then store that for charWidth() and calculate width_char().
	int _width_chars;
	std::shared_ptr<sf::Font> _font;
	std::shared_ptr<sf::RenderWindow> _window;
	std::shared_ptr<css::box_dynamic> _css_root;
public:
	enum {
		DESIGN_WIDTH = 1024,
		DESIGN_HEIGHT = 768,
		DESIGN_HEIGHT_CHAR = DESIGN_HEIGHT/(DESIGN_HEIGHT/24)
	};

	void swapBuffers();
	auto getFont() const { return _font; };
	auto getWindow() const { return _window; };
	int width_pixel() const { return _width_pixels;};
	int height_pixel() const { return _height_pixels;};
	void resize(int w, int h);

	// makes physical pixels look logical
	std::pair<float, float> inverseScale() const { return std::pair<float, float>((float)DESIGN_WIDTH / _width_pixels, (float)DESIGN_HEIGHT / _height_pixels); };
	// makes logical pixels look physical
	std::pair<float, float> scale() const { return std::pair<float, float>(_width_pixels/(float)DESIGN_WIDTH, _height_pixels/(float)DESIGN_HEIGHT); };

	void append(std::shared_ptr<css::box> src) { _css_root->append(src); };
	void remove(std::shared_ptr<css::box> src) { _css_root->remove(src); };
	void draw() {
		_css_root->recalc();
		_css_root->screen_coords(std::pair<int, int>(0, 0));
		_css_root->draw();
	}

	// character support; likely to need all of this for subwindows as well (should be driven by font size)
	int width_char() const { return _width_chars;};
	int height_char() const { return DESIGN_HEIGHT_CHAR;};

//	int drawCh(int x, int y, char ch, sf::Color color);

	float charWidth() const { return (float)(_width_pixels)/(float)(_width_chars);};
	float charHeight() const { return (float)(_height_pixels)/ DESIGN_HEIGHT_CHAR;};
};

#include "singleton.off.hpp"

}	// namespace isk

#endif
