// cyclic_fn.hpp

#ifndef CYCLIC_FN_HPP
#define CYCLIC_FN_HPP 1

#include "z_n.hpp"

namespace zaimoni {
namespace math {

namespace mod_n {

template<uintmax_t n,class T>
class cyclic_fn_enumerated
{
private:
	T _x[n];
//	mutable unsigned char _bitmap;
public:
	cyclic_fn_enumerated(const T* src) {memmove(_x,src,n*sizeof(T));};
	ZAIMONI_DEFAULT_COPY_DESTROY_ASSIGN(cyclic_fn_enumerated);

	T operator()(Z_<n> i) const {return _x[i];}

#if 0
	bool is_even() const;
	bool is_nonzero() const;

	bool is_odd() const;	// operation trait: unary - on range

	// operation trait: total ordering < on range
	bool is_alternating() const;	// non-strictly.. we allow zero as that's what a Taylor series is interested in
	bool is_nonnegative() const;
	bool is_nonpositive() const;
#endif
};

}
}	// namespace math
}	// namespace zaimoni

#endif
