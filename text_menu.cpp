#include "text_menu.hpp"
#include "display_manager.hpp"
#include "css_SFML.hpp"
#include "Zaimoni.STL/Logging.h"

namespace isk {

static void _remove_gui(std::shared_ptr<css::box>& src)
{
	if (src) DisplayManager::get().remove(src);
}

textmenu::~textmenu()
{
	_remove_gui(_gui_top);
}

bool textmenu::handle(const sf::Event::KeyEvent& hotkey)
{
	for (const auto& entry : entries) {
		if (hotkey != std::get<1>(entry)) continue;
		auto handler(std::get<3>(entry));
		if (remove_self_after_handling) _remove_gui(_gui_top);	// installing menus can invalidate the iterator
		return handler();
	}
	return false;
}

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
				*span = new sf::Text(line, *DisplayManager::get().getFont(), DisplayManager::get().charHeight());
				span->set_clear(css::box::CF_LEFT);
				// CSS styles desired: clear:left (implicit line break) on first text node generated for each line
				// would like to use HTML for italics/bold/... but that requires parsing
				working->append(span.release());
			}
		}
		// center this: non-default full-auto margins
		working->set_auto(css::box::LEFT);
		working->set_auto(css::box::TOP);
		working->set_auto(css::box::RIGHT);
		working->set_auto(css::box::BOTTOM);
		_gui_top = decltype(_gui_top)(working.release());
		DisplayManager::get().append(_gui_top);
	}
}

}
