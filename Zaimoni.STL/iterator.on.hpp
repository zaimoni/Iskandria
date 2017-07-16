// iterator.on.hpp

#undef ZAIMONI_ITER_DECLARE
#define ZAIMONI_ITER_DECLARE(CLASS)	\
	bool operator!=(const CLASS& rhs) const {return !(*this==rhs);}	\
    CLASS operator+(difference_type n) const {	\
		CLASS ret(*this);	\
		return ret += n;	\
	}	\
	friend CLASS operator+(difference_type n, const CLASS& src) {	\
		CLASS ret(src);	\
		return ret += n;	\
	};	\
    CLASS operator-(difference_type n) const {	\
		CLASS ret(*this);	\
		return ret -= n;	\
	}	\
    bool operator>(const CLASS& rhs) const { return rhs< *this; }	\
    bool operator<=(const CLASS& rhs) const { return !(rhs < *this); }	\
    bool operator>=(const CLASS& rhs) const { return !(*this < rhs); }

