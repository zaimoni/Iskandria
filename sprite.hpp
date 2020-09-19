#ifndef SPRITE_HPP
#define SPRITE_HPP 1

#include <SFML/Graphics.hpp>
#include "Zaimoni.STL/ptr2.hpp"

namespace isk {

class Sprite {
private:
	zaimoni::intrusive_ptr<sf::Texture>* texture;	// must agree with DisplayManager's texture cache
	sf::Sprite* ptr;
public:
	Sprite() = delete; // define this if we need it
	Sprite(const Sprite& src) = delete;
	Sprite(zaimoni::intrusive_ptr<sf::Texture>* src) : texture(src),ptr(new sf::Sprite(*src->get())) {

	}
	Sprite(Sprite&& src) noexcept : texture(src.texture), ptr(src.x) {
		ptr = 0;
		texture = 0;
	}
	~Sprite() {
		if (ptr) {
			delete ptr;
			x = 0;
		}
		if (texture) {
			texture->release();
			texture = 0;
		}
	}

	Sprite& operator=(const Sprite& src) = delete;
	Sprite& operator=(Sprite&& src) noexcept : x(src.x), texture(src.texture) {
		x = 0;
		texture = 0;
	}

	// typecasts
	operator sf::Sprite*& () { return ptr; }
	operator sf::Sprite* const& () const { return ptr; }

	// NOTE: C/C++ -> of const pointer to nonconst data is not const
	sf::Sprite* operator->() const { return ptr; }
};

}


#endif
