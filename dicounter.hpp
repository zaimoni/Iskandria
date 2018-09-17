// dicounter.hpp

#ifndef DICOUNTER_HPP
#define DICOUNTER_HPP

#include <utility>
#include "Zaimoni.STL/Logging.h"

// typically would use intmax_t, but would like to be able to fully represent a uintmax_t in either direction
struct dicounter : std::pair<uintmax_t,uintmax_t>
{
	dicounter() = default;
	ZAIMONI_DEFAULT_COPY_DESTROY_ASSIGN(dicounter);

	uintmax_t& positive() {return this->first;};
	uintmax_t positive() const {return this->first;};
	uintmax_t& negative() {return this->second;};
	uintmax_t negative() const {return this->second;};

#define ZAIMONI_ADD_DEF(CAPACITY,OP,POS,NEG)	\
	uintmax_t CAPACITY () const {return UINTMAX_MAX-POS;}	\
	void OP (uintmax_t& src) {	\
		if (0==src) return;	\
		if (0<NEG) {	\
			if (src <= NEG) {	\
				NEG -= src;	\
			} else {	\
				src -= NEG;	\
				NEG = 0;	\
				POS = src;	\
			}	\
			src = 0;	\
			return;	\
		}	\
		const uintmax_t test = UINTMAX_MAX-POS;	\
		if (test>=src) {	\
			POS += src;	\
			src = 0;	\
			return;	\
		} else if (0<test) {	\
			POS = UINTMAX_MAX;	\
			src -= test;	\
			return;	\
		}	\
	}	\
	void safe_##OP (uintmax_t src) {	\
		if (0==src) return;	\
		if (0<NEG) {	\
			if (src <= NEG) {	\
				NEG -= src;	\
			} else {	\
				src -= NEG;	\
				NEG = 0;	\
				POS = src;	\
			}	\
			return;	\
		}	\
		const uintmax_t test = UINTMAX_MAX-POS;	\
		assert(test>=src);	\
		POS += src;	\
		return;	\
	}

ZAIMONI_ADD_DEF(add_capacity,add,positive(),negative())
ZAIMONI_ADD_DEF(sub_capacity,sub,negative(),positive())

#undef ZAIMONI_ADD_DEF
};

#endif
