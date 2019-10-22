#include "text_menu.hpp"
#include "display_manager.hpp"
#include "css_SFML.hpp"
#include "Zaimoni.STL/Logging.h"

namespace isk {

void textmenu::add_entry(const std::vector<std::string>& label, const sf::Event::KeyEvent& hotkey, const std::function<bool(void)>& handler)
{
	static const sf::Event::TextEvent nop = { 0 };
	entries.push_back(menu_entry(label, hotkey, nop, handler));
}

void textmenu::draw() const
{
	if (!_gui_top) {
		// bootstrap this
		// once this is working we'll want a non-default destructor (to remove this from the display)
		std::unique_ptr<css::box_dynamic> working(new css::box_dynamic());
		for (const auto& entry : entries) {
			const auto& lines = std::get<0>(entry);
			if (lines.empty()) continue;	// can't draw a pure hotkey
			for (const auto& line : lines) {
				if (line.empty()) continue;
				std::unique_ptr<css::wrap<sf::Text> > span(new css::wrap<sf::Text>());
				*span = new sf::Text(line, *DisplayManager::get().getFont());
				// CSS styles desired: clear:left (implicit line break) on first text node generated for each line
				// would like to use HTML for italics/bold/... but that requires parsing
				working->append(std::shared_ptr<css::box>(span.release()));
			}
		}
		// center this: non-default full-auto margins
		working->set_auto(css::box::LEFT);
		working->set_auto(css::box::TOP);
		working->set_auto(css::box::RIGHT);
		working->set_auto(css::box::BOTTOM);
		_gui_top = std::shared_ptr<css::box_dynamic>(working.release());
		DisplayManager::get().append(_gui_top);
	}
}

}
