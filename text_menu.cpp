#include "text_menu.hpp"

namespace isk {

void textmenu::add_entry(const std::vector<std::string>& label, const sf::Event::KeyEvent& hotkey, const std::function<bool(void)>& handler)
{
	static const sf::Event::TextEvent nop = { 0 };
	entries.push_back(menu_entry(label, hotkey, nop, handler));
}

}
