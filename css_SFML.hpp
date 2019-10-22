#ifndef CSS_SFML_HPP
#define CSS_SFML_HPP 1

#include "cssbox.hpp"
#include "display_manager.hpp"
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>

namespace css {

template<class T>
class wrap : public box
{
private:
	std::shared_ptr<T> _x;
public:
	wrap() = default;
	wrap(const wrap& src) = default;
	wrap(wrap&& src) = default;
	~wrap() = default;
	wrap& operator=(const wrap& src) = default;
	wrap& operator=(wrap&& src) = default;

	wrap& operator=(const std::shared_ptr<T>& src) { _x = src; assign_bootstrap(); return *this; };
	wrap& operator=(std::shared_ptr<T>&& src) { _x = src; assign_bootstrap(); return *this; };
	wrap& operator=(T* const src) { _x = decltype(_x)(src); assign_bootstrap(); return *this; };

	auto natural_dimensions() const { return _x->getLocalBounds(); }
	auto screen_dimensions() const { return _x->getGlobalBounds(); }

	virtual void draw() const {
		if (_x) isk::DisplayManager::get().getWindow()->draw(*_x);
	}

	virtual void screen_coords(std::pair<int, int> logical_origin) {
		box::screen_coords(logical_origin);
		_x->setPosition(_screen.first, _screen.second);
	}
private:
	int assign_bootstrap_code() const {
		if ((_auto & (1ULL << HEIGHT)) && (_auto & (1ULL << WIDTH))) {
			if ((_auto_recalc & (1ULL << HEIGHT)) || (_auto_recalc & (1ULL << WIDTH))) return 1;
		}
		return 0;
	}
	void physical_width_height() {
		auto natural = _x->getLocalBounds();
		if (0 <= natural.width) _width((int)(natural.width + 0.5));
		if (0 <= natural.height) _height((int)(natural.height + 0.5));
		_auto_recalc &= ~((1ULL << HEIGHT) | (1ULL << WIDTH));
	}
	void assign_bootstrap() {
		if (!_x) return;	// reject NULL;
		if ((_auto & (1ULL << HEIGHT)) && (_auto & (1ULL << WIDTH))) physical_width_height();
		else {
			// XXX should also handle auto, fixed and fixed, auto here
			// but must delegate fixed,fixed
			// probably should check for min/max width,height here as well
		}
	}

	virtual bool flush() { return !_x; }

	virtual int need_recalc() const {
		if (!_x) return 0;	// reject NULL;
		int ret = assign_bootstrap_code();
		if (0 < ret) return ret;
		return 0;
	}
	virtual void _recalc(int code) {
		if (0 >= code) return;	// do not process errors or no-op
		switch (code)	// arguably should have a private enum for legibility.  Backfit it when it's clear what's going on.
		{
		case 1:
			physical_width_height();
			return;
		default: return;	// not handled is like error
		}
	};
};

}	// namespace css

#endif
