// z_n.hpp

#ifndef Z_N_HPP
#define Z_N_HPP 1

#include "Zaimoni.STL/Logging.h"
#include "Zaimoni.STL/augment.STL/type_traits"

namespace zaimoni {
namespace math {

// modulo arithmetic classes
template<uintmax_t n>
class Z_
{
	ZAIMONI_STATIC_ASSERT(0<n);
private:
	typename min_unsigned<n>::type _x;
public:
	Z_() = default;
	Z_(uintmax_t src) : _x(src%n) {}
	explicit Z_(intmax_t src) : _x(0<=src ? src%n : (n-1)-norm(src)%n) {};
	ZAIMONI_DEFAULT_COPY_DESTROY_ASSIGN(Z_);

	operator uintmax_t() const {return _x;};
	explicit operator intmax_t() const {return n/2>=_x ? _x : -(intmax_t)((n-1)-_x);}
};

}	// namespace math
}	// namespace zaimoni


#endif
