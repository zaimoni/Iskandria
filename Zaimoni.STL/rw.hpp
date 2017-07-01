// rw.hpp

#ifndef RW_HPP
#define RW_HPP 1

namespace zaimoni {

// specialize as needed
template<class T>
struct rw_mode
{
	enum {
	// 0: not specified
	// 1: fixed-size: begin/size()
	// 2: fixed-size: operator[]/size()
	// --- unimplemented
	// 3: variable-size: begin/end/size()
	// 4: variable-size: operator[]/size()
		group_write = 0,
	// 0: not specified
	// 1: fixed-size; begin/size()
	// 2: fixed-size: operator[]
	// --- unimplemented
		group_read = 0
	};
};

}	// namespace zaimoni

#endif
