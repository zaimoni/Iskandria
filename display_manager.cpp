// display_manager.cpp

#include "display_manager.hpp"
#include "matrix.hpp"
#include "gridtile.hpp"

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

DisplayManager::~DisplayManager()
{
	_css_root->disconnect();
}

void DisplayManager::draw() {
	_css_root->recalc();
#if POINT_IS_Z_VECTOR
	_css_root->screen_coords(css::box::point(0));
#else
	_css_root->screen_coords(css::box::point(0, 0));
#endif
	_css_root->draw();
}

void DisplayManager::resize(int w, int h) {
	_width_pixels = w;	// SFML: this isn't changing the coordinates being reported to us (oops), just the physical pixels on screen
	_height_pixels = h;
	_css_root->force_size(w, h);
	// \todo any other triggered calculations e.g. character-based stats
}

void DisplayManager::swapBuffers()
{
	_window->display();
	_window->clear(_background);
}

// reference data: pixels that belong to an isosceles triangle of height x-15, width y-13
static constexpr const std::array<unsigned char,7> hex_triangle_steps = { 2,2,2,3,2,2,2 };

template<class T, size_t N>
constexpr std::array<T, N> triangle_sum(const std::array<T, N>& src)
{
	std::array<T, N> ret(src);
	int ub = N - 1;
	while (0 <= --ub) {
		auto i = ub;
		while (N > ++i) ret[i] += ret[ub];
	}
	return ret;
}

static constexpr auto hex_triangle_scanline = triangle_sum(hex_triangle_steps);

template<class T, size_t N>
constexpr size_t triangle_pixel_count(const std::array<T, N>& src) {
	size_t ret = src[N - 1];
	int ub = N - 1;
	while (0 <= --ub) ret += 2 * src[ub];
	return ret;
}

static auto hex_triangle_pixels()
{
	std::array<zaimoni::math::vector<unsigned char, 2>, triangle_pixel_count(hex_triangle_scanline)> ret;
	zaimoni::make<zaimoni::math::vector<unsigned char, 2> > factory;
	size_t ub = 0;
	int y = 0;
	do	{
		size_t j = DisplayManager::ISOMETRIC_TRIANGLE_WIDTH -hex_triangle_scanline[y];
		do ret[ub++] = factory(j, y);
		while (DisplayManager::ISOMETRIC_TRIANGLE_WIDTH > ++j);
	} while(++y < hex_triangle_scanline.size());
	--y;
	while(0 <= --y) {
		size_t j = DisplayManager::ISOMETRIC_TRIANGLE_WIDTH - hex_triangle_scanline[y];
		do ret[ub++] = factory(j, (DisplayManager::ISOMETRIC_TRIANGLE_HEIGHT-1)-y);
		while (DisplayManager::ISOMETRIC_TRIANGLE_WIDTH > ++j);
	}
	return ret;
}

static auto _ref_hex_triangle_pixels(hex_triangle_pixels());

const char* const hex_digits = "0123456789ABCDEF";

/// <returns>color, length of match for color parsing</returns>
static std::pair<sf::Color,size_t> parse_css_color(const std::string& src)
{
	const size_t srclen = src.size();
	if (4 <= srclen) {
		if (9 >= srclen) {
			unsigned char buffer[8];
			size_t ub = 0;
			const char* tmp = 0;
			char x =  0;
			if ('#' == src.front()) {	// one of the hex encodings; we do extended RGBA as well as RGB
				while (8 > ub && srclen > ub + 1) if (tmp = strchr(hex_digits, (x = src[ub + 1]))) buffer[ub++] = *tmp - x;
				if (ub + 1 == srclen) {
					switch (ub) {
					case 3: return std::pair(sf::Color(17*buffer[0], 17*buffer[1], 17*buffer[2]), 4);
					case 4: return std::pair(sf::Color(17 * buffer[0], 17 * buffer[1], 17 * buffer[2], 17*buffer[3]), 5);
					case 6: return std::pair(sf::Color(16 * buffer[0]+buffer[1], 16 * buffer[2]+buffer[3], 16 * buffer[4]+buffer[5]), 7);
					case 8: return std::pair(sf::Color(16 * buffer[0] + buffer[1], 16 * buffer[2] + buffer[3], 16 * buffer[4] + buffer[5], 16 * buffer[6] + buffer[7]), 9);
					}
				}
				return std::pair(sf::Color::Transparent, 0); // no-match
			}
		}
		// \todo CSS-standard rgb color
	}
	return std::pair(sf::Color::Transparent,0); // no-match
}

template<class T>
static void draw_monochrome_STL_mask(sf::Image& dest, const zaimoni::math::vector<int, 2>& origin, T& STL_mask, const sf::Color& src)
{
	if (!src.a) return;
	const auto dims(dest.getSize());
	for (const auto& offset : STL_mask) {
		const auto target = origin + offset;
		if (dims.x > target[0] && 0 <= target[0] && dims.y > target[1] && 0 <= target[1]) dest.setPixel(target[0], target[1], src);
	}
}

zaimoni::math::vector<int, 2> triangle_v_reflect(const zaimoni::math::vector<int, 2>& src)
{
	auto ret(src);
	iskandria::grid::exact_reflect((DisplayManager::ISOMETRIC_TRIANGLE_WIDTH - 1) / 2, (DisplayManager::ISOMETRIC_TRIANGLE_HEIGHT - 1) / 2, (0== DisplayManager::ISOMETRIC_TRIANGLE_WIDTH%2)+2*(0 == DisplayManager::ISOMETRIC_TRIANGLE_HEIGHT % 2), iskandria::compass::N, ret[0], ret[1]);
	return ret;
}

template<class T, class U>
static void draw_monochrome_STL_mask(sf::Image& dest, const zaimoni::math::vector<int, 2>& origin, U xform, T& STL_mask, const sf::Color& src)
{
	if (!src.a) return;
	const auto dims(dest.getSize());
	for (const auto& offset : STL_mask) {
		const auto target = xform(origin + offset);
		if (dims.x > target[0] && 0 <= target[0] && dims.y > target[1] && 0 <= target[1]) dest.setPixel(target[0], target[1], src);
	}
}

bool CGI_load(std::shared_ptr<sf::Image>& x, const std::string& src)
{
	static const std::string floor("floor:");
	static const std::string lwall("lwall:");
	static const std::string rwall("rwall:");
	zaimoni::make<zaimoni::math::vector<int, 2> > factory;

	if (floor == src.substr(0, 6)) {
		auto c1(parse_css_color(src.substr(6)));
		if (0 < c1.second) {
			x->create(DisplayManager::ISOMETRIC_HEX_WIDTH, DisplayManager::ISOMETRIC_HEX_HEIGHT, sf::Color::Transparent);
			draw_monochrome_STL_mask(*x, factory(0, DisplayManager::ISOMETRIC_HEX_HEIGHT - DisplayManager::ISOMETRIC_TRIANGLE_HEIGHT), _ref_hex_triangle_pixels, c1.first);
			draw_monochrome_STL_mask(*x, factory(DisplayManager::ISOMETRIC_TRIANGLE_WIDTH, DisplayManager::ISOMETRIC_HEX_HEIGHT - DisplayManager::ISOMETRIC_TRIANGLE_HEIGHT), triangle_v_reflect, _ref_hex_triangle_pixels, c1.first);
			return true;
		}
	}
	else if (lwall == src.substr(0, 6)) {
		auto c1(parse_css_color(src.substr(6)));
		if (0 < c1.second) {
			x->create(DisplayManager::ISOMETRIC_HEX_WIDTH, DisplayManager::ISOMETRIC_HEX_HEIGHT, sf::Color::Transparent);
			draw_monochrome_STL_mask(*x, factory(0, 0), _ref_hex_triangle_pixels, c1.first);
			draw_monochrome_STL_mask(*x, factory(0, DisplayManager::ISOMETRIC_TRIANGLE_HEIGHT / 2 + 1), triangle_v_reflect, _ref_hex_triangle_pixels, c1.first);
			return true;
		}
	}
	else if (rwall == src.substr(0, 6)) {
		auto c1(parse_css_color(src.substr(6)));
		if (0 < c1.second) {
			x->create(DisplayManager::ISOMETRIC_HEX_WIDTH, DisplayManager::ISOMETRIC_HEX_HEIGHT, sf::Color::Transparent);
			draw_monochrome_STL_mask(*x, factory(DisplayManager::ISOMETRIC_TRIANGLE_WIDTH, DisplayManager::ISOMETRIC_TRIANGLE_HEIGHT / 2 + 1), _ref_hex_triangle_pixels, c1.first);
			draw_monochrome_STL_mask(*x, factory(DisplayManager::ISOMETRIC_TRIANGLE_WIDTH, 0), triangle_v_reflect, _ref_hex_triangle_pixels, c1.first);
			return true;
		}
	}
	return false;
}

std::shared_ptr<sf::Image> DisplayManager::getImage(const std::string& src)
{
	static const std::string cgi("cgi:");

	if (1 == _image_cache.count(src)) return _image_cache[src];	// 0: already loaded
	std::shared_ptr<sf::Image> x(new sf::Image());
	// 1) CGI options
	if (cgi == src.substr(0, 4) && CGI_load(x, src.substr(4))) {
		_image_cache[src] = x;
		return x;
	}
	// 2) relative file path
	if (x->loadFromFile(src)) {
		_image_cache[src] = x;
		return x;
	}
	return 0;
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
