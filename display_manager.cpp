// display_manager.cpp

#include "display_manager.hpp"

#include <new>

#include "singleton.on.hpp"

#include "Zaimoni.STL/Logging.h"

/*
8 // De f a u l t s f o r SFML window .
9 const int WINDOW_HORIZONTAL_PIXELS_DEFAULT = 1024;
10 const int WINDOW_VERTICAL_PIXELS_DEFAULT = 768;
15 const std :: string WINDOW_TITLE_DEFAULT = ” Dr a g o n f l y ”;
11 const int WINDOW_HORIZONTAL_CHARS_DEFAULT = 80;
12 const int WINDOW_VERTICAL_CHARS_DEFAULT = 24;

13 const int WINDOW_STYLE_DEFAULT = sf :: Style :: Titlebar ;
14 const sf :: Color WINDOW_BACKGROUND_COLOR_DEFAULT = sf :: Color :: Black;
16 const std :: string FONT_FILE_DEFAULT = ” df-f o n t . t t f ”;
*/

namespace isk {

ISK_SINGLETON_BODY(DisplayManager)

DisplayManager::DisplayManager()
:	_background(sf::Color::Black),
	_width_pixels(DESIGN_WIDTH),
	_height_pixels(DESIGN_HEIGHT),
	_width_chars(80),
	_height_chars(24),
	_font(new sf::Font()),
	_css_root(new css::box_dynamic(true))
{
	// \todo load starting dimensions from configuration?
	_window = decltype(_window)(new sf::RenderWindow(sf::VideoMode(1024,768), "Iskandria")),
	_css_root->min_width(DESIGN_WIDTH);
	_css_root->max_width(DESIGN_WIDTH);
	_css_root->min_height(DESIGN_HEIGHT);
	_css_root->max_height(DESIGN_HEIGHT);
//	two parts to configuring...system font location, and system font
	if (!_font->loadFromFile("fonts\\unifont.ttf")) throw std::bad_alloc();	// Unifont: rasterized
//	if (!_font->loadFromFile("c:\\Windows\\Fonts\\cour.ttf")) throw std::bad_alloc();	// Courier on a default Windows system install
}

void DisplayManager::resize(int w, int h) {
	_width_pixels = w;	// SFML: this isn't changing the coordinates being reported to us (oops), just the physical pixels on screen
	_height_pixels = h;
	_css_root->min_width(w);
	_css_root->max_width(w);
	_css_root->min_height(h);
	_css_root->max_height(h);
	// \todo any other triggered calculations e.g. character-based stats
	// \todo schedule but do not actually reflow
}

void DisplayManager::swapBuffers()
{
	_window->display();
	_window->clear(_background);
}


}	// namespace isk

#ifdef TEST_APP
// fast compile test
// g++ -std=c++11 -otest.exe -Os -DTEST_APP -DSFML_STATIC display_manager.cpp -lsfml-graphics-s -lsfml-window-s -lsfml-system-s
// g++ -std=c++11 -otest.exe -Os -DTEST_APP display_manager.cpp -lsfml-graphics -lsfml-window -lsfml-system
int main(int argc, char* argv[])
{
	isk::DisplayManager::get();
	return 0;
}
#endif
