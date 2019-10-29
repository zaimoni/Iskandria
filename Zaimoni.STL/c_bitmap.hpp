#ifndef ZAIMONI_STL_C_BITMAP_HPP
#define ZAIMONI_STL_C_BITMAP_HPP
/* (C)2019 Kenneth Boyd, license: LICENSE_BOOST.txt */

#include <stddef.h>
#include <limits.h>
#include <type_traits>

namespace zaimoni {

	template<size_t n>
	struct bitmap
	{
		static_assert(sizeof(unsigned long long)* CHAR_BIT >= n, "use std::bitmap or std::vector<bool> instead");
		static_assert(0 < n, "zero-bit bitmap is pointless");
		typedef typename std::conditional<sizeof(unsigned char) * CHAR_BIT >= n, unsigned char,
			typename std::conditional<sizeof(unsigned short) * CHAR_BIT >= n, unsigned short,
			typename std::conditional<sizeof(unsigned int) * CHAR_BIT >= n, unsigned int,
			typename std::conditional<sizeof(unsigned long) * CHAR_BIT >= n, unsigned long, unsigned long long>::type>::type>::type>::type type;
	};

}

#endif
