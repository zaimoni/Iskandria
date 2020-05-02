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
	zaimoni::math::vector<int, 2> _pixel_dim;
	// UI clipping reserves: mockup (consider inferring from fixed-positioning elements)
	zaimoni::gdi::box<zaimoni::math::vector<int, 2> > _clip_margin_logical;

	// we should compute the pixel width from a sample character, then store that for charWidth() and calculate width_char().
	int _width_chars;
	std::shared_ptr<sf::Font> _font;
	std::shared_ptr<sf::RenderWindow> _window;
	std::shared_ptr<css::box_dynamic> _css_root;

	// graphics management
	std::map<std::string, std::weak_ptr<sf::Image> > _image_cache;	// in main RAM
	std::map<std::string, std::weak_ptr<sf::Texture> > _texture_cache;	// in graphics card
public:
	enum {
		DESIGN_WIDTH = 1024,
		DESIGN_HEIGHT = 768,
		DESIGN_HEIGHT_CHAR = DESIGN_HEIGHT / (DESIGN_HEIGHT / 24),

		ISOMETRIC_TRIANGLE_HEIGHT = 13,
		ISOMETRIC_TRIANGLE_WIDTH  = 15,
		ISOMETRIC_HEX_HEIGHT = 2 * ISOMETRIC_TRIANGLE_HEIGHT + 1,
		ISOMETRIC_HEX_WIDTH  = 2 * ISOMETRIC_TRIANGLE_WIDTH
	};

	void swapBuffers();
	auto getFont() const { return _font; };
	auto getWindow() const { return _window; };
	int width_pixel() const { return _pixel_dim[0];};
	int height_pixel() const { return _pixel_dim[1];};
	void resize(int w, int h);
	auto clip_rect_logical() const {
		return decltype(_clip_margin_logical)(_clip_margin_logical.tl_c(), zaimoni::make<decltype(_clip_margin_logical.br_c())>(DESIGN_WIDTH, DESIGN_HEIGHT) - _clip_margin_logical.tl_c());
	}

	// makes physical pixels look logical
	std::pair<float, float> inverseScale() const { return std::pair<float, float>((float)DESIGN_WIDTH / _pixel_dim[0], (float)DESIGN_HEIGHT / _pixel_dim[1]); };
	// makes logical pixels look physical
	std::pair<float, float> scale() const { return std::pair<float, float>(_pixel_dim[0]/(float)DESIGN_WIDTH, _pixel_dim[1] /(float)DESIGN_HEIGHT); };

	void append(std::shared_ptr<css::box>& src) { _css_root->append(src); };
	void remove(std::shared_ptr<css::box>& src) { _css_root->remove(src); };
	void draw();

	// character support; likely to need all of this for subwindows as well (should be driven by font size)
	int width_char() const { return _width_chars;};
	int height_char() const { return DESIGN_HEIGHT_CHAR;};

//	int drawCh(int x, int y, char ch, sf::Color color);

	float charWidth() const { return (float)(_pixel_dim[0])/(float)(_width_chars);};
	float charHeight() const { return (float)(_pixel_dim[1])/ DESIGN_HEIGHT_CHAR;};

	std::shared_ptr<sf::Image> getImage(const std::string& src);
};

#include "singleton.off.hpp"

}	// namespace isk

#endif
