#ifndef TEXT_MENU_HPP
#define TEXT_MENU_HPP 1

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/Text.hpp>
#include <vector>
#include <functional>

namespace iskandria {

class textmenu {
private:
//	static std::vector<std::function<bool(char)> > _handlers;

	// the natural triple here is:
	// text label (possibly multiple lines)
	// action keycode (framework leakage?)
	// action function
	// The last part must *NOT* reach the savefile as-is.
	std::vector<
		std::tuple<std::vector<std::string>, sf::Event::KeyEvent, std::function<bool(void)>, std::vector<std::unique_ptr<sf::Text> > >
	> entries;
	// when installed to the input manager:
	// * show the bounding rectangle of the text if and only if if mouse is within the bounding rectangle of the text
	// * ultra-high z-index (above main game render)
	// * if the hotkey is pressed, or the mouse clicked within a bounding rectangle, that option's handler is executed
	// * there is a bounding rectangle if and only if there is a text label
public:
	textmenu() = default;
	ZAIMONI_DEFAULT_COPY_DESTROY_ASSIGN(textmenu);

	bool add_entry(const std::vector<std::string>& label, const sf::Event::KeyEvent& hotkey, const std::function<bool(void)>& handler);
};

}

#endif
