#ifndef TEXT_MENU_HPP
#define TEXT_MENU_HPP 1

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/Text.hpp>
#include <vector>
#include <functional>
#include <memory>

namespace isk {

class textmenu {
private:
	typedef std::tuple < std::vector<std::string>, sf::Event::KeyEvent, std::function<bool(void)>, std::vector<std::shared_ptr<sf::Text> > > menu_entry;

	// the natural triple here is:
	// text label (possibly multiple lines)
	// action keycode (framework leakage?)
	// action function
	// The last part must *NOT* reach the savefile as-is.  Simplest if this is not wrapped as a game object but instead handled by the input manager.
	std::vector<menu_entry> entries;
	// when installed to the input manager:
	// * show the bounding rectangle of the text if and only if if mouse is within the bounding rectangle of the text
	// * ultra-high z-index (above main game render)
	// * if the hotkey is pressed, or the mouse clicked within a bounding rectangle, that option's handler is executed
	// * there is a bounding rectangle if and only if there is a text label
public:
	textmenu() = default;
	ZAIMONI_DEFAULT_COPY_DESTROY_ASSIGN(textmenu);

	void add_entry(const std::vector<std::string>& label, const sf::Event::KeyEvent& hotkey, const std::function<bool(void)>& handler);
	void add_entry(const std::string& label, const sf::Event::KeyEvent& hotkey, const std::function<bool(void)>& handler) {
		std::vector<std::string> working;
		working.push_back(label);
		add_entry(working, hotkey, handler);
	}
	void add_entry(const sf::Event::KeyEvent& hotkey, const std::function<bool(void)>& handler) {
		add_entry(std::vector<std::string>(), hotkey, handler);
	};
};

}

#endif
