#ifndef SPRITE_HPP
#define SPRITE_HPP 1

#include <SFML/Graphics.hpp>
#include <memory>

namespace isk {

class Sprite {
private:
	std::shared_ptr<sf::Texture> texture;
	std::unique_ptr<sf::Sprite> sprite;
public:
	Sprite() = delete; // define this if we need it
	Sprite(const Sprite& src) = delete;
	Sprite(const std::shared_ptr<sf::Texture>& src) : texture(src), sprite(new sf::Sprite(*src.get())) { }
	Sprite(Sprite&& src) = default;
	~Sprite() = default;

	Sprite& operator=(const Sprite& src) = delete;
	Sprite& operator=(Sprite&& src) = default;

	// forwarders
	auto getLocalBounds() const { return sprite->getLocalBounds(); }
	void setPosition(float x, float y) { sprite->setPosition(x, y); }
	void setScale(float factorX, float factorY) { sprite->setScale(factorX, factorY); }

	// typecasts
	operator sf::Sprite& () { return *sprite; }
	operator const sf::Sprite& () const { return *sprite; }
	operator sf::Sprite* () { return sprite.get(); }
	operator sf::Sprite* const () const { return sprite.get(); }

	// NOTE: C/C++ -> of const pointer to nonconst data is not const
	sf::Sprite* operator->() const { return sprite.get(); }
};

}


#endif
