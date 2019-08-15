#include "css_SFML.hpp"

#ifdef TEST_APP3
// fast compile test
// g++ -std=c++14 -otest.exe -Os -DTEST_APP3 css_SFML.cpp cssbox.cpp -lsfml-graphics -lsfml-system
int main(int argc, char* argv[])
{
	css::wrap<sf::Text> text;
	css::wrap<sf::Sprite> sprite;

	// display manager boilerplate
	std::shared_ptr<sf::Font> _font(new sf::Font());
	//	two parts to configuring...system font location, and system font
	// 	for now, hardcode Courier on a default Windows system install
	if (!_font->loadFromFile("c:\\Windows\\Fonts\\cour.ttf")) throw std::bad_alloc();	// XXX

	text = new sf::Text("Q)uit", *_font);

	return 0;
}
#endif

