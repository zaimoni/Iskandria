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

	wrap& operator=(const std::shared_ptr<T>& src) { _x = src; return *this; };
	wrap& operator=(std::shared_ptr<T>&& src) { _x = src; return *this; };
	wrap& operator=(T* const src) { _x = decltype(_x)(src); return *this; };

	auto natural_dimensions() const { return _x->getLocalBounds(); }
};

}	// namespace css

#endif
