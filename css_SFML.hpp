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

	auto natural_dimensions() const { return _x->getLocalBounds(); }
};

}	// namespace css

#endif
