#ifndef CSS_SFML_HPP
#define CSS_SFML_HPP 1

#include "cssbox.hpp"
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
private:
	void assign_bootstrap() {
		if (!_x) return;	// reject NULL;
		if ((_auto & (1ULL << HEIGHT)) && (_auto & (1ULL << WIDTH))) {
			auto natural = _x->getLocalBounds();
			if (0 <= natural.width) _width((int)(natural.width + 0.5));
			if (0 <= natural.height) _height((int)(natural.height + 0.5));
		} else {
			// XXX should also handle auto, fixed and fixed, auto here
			// but must delegate fixed,fixed
			// probably should check for min/max width,height here as well
		}
	}
};

}	// namespace css

#endif
