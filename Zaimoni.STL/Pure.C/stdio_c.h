/* stdio_c.h */
#ifndef STDIO_C_H
#define STDIO_C_H 1

#include <stdio.h>
#include <stdint.h>

/* C strings to stdout; include stdio.h before using these */
/* including cstdio ok if not on a deathstation */
#define STRING_LITERAL_TO_STDOUT(A) fwrite(A,sizeof(A)-1,1,stdout)
#define C_STRING_TO_STDOUT(A) fwrite(A,strlen(A),1,stdout)
#ifdef __cplusplus
#define STL_PTR_STRING_TO_STDOUT(A) fwrite((A)->data(),(A)->size(),1,stdout)
#endif

/* link agaist something providing _fatal_code for these */
#define ZAIMONI_FWRITE_OR_DIE(T,src,dest) { if (1!=fwrite(&src, sizeof(src), 1, dest)) _fatal_code("failed to write " #T,3); }
#define ZAIMONI_FREAD_OR_DIE(T,dest,src) { if (1!=fread(&dest, sizeof(dest), 1, src)) _fatal_code("failed to read " #T,3); }


#ifdef __cplusplus
extern "C" {
#endif

long get_filelength(FILE* src);
uintmax_t read_uintmax(uintmax_t ub,FILE* src);
void write_uintmax(uintmax_t ub,uintmax_t src,FILE* dest);

#ifdef __cplusplus
}	/* end extern "C" */
#endif

/* XXX have some natural C++ support here */
#ifdef __cplusplus
#include <limits.h>
#include <type_traits>

namespace zaimoni {

#define ZAIMONI_RW(A,B)	\
template<class T>	\
typename std::enable_if<std::is_same<A, T>::value, T>::type read(FILE* src)	\
{	\
	return (T)read_uintmax(B, src);	\
}	\
	\
template<class T>	\
typename std::enable_if<std::is_same<A, T>::value, T>::type read(FILE* src, uintmax_t ub)	\
{	\
	return (T)read_uintmax(ub, src);	\
}	\
	\
template<class T>	\
typename std::enable_if<std::is_same<A, T>::value, void>::type write(const T& src, FILE* dest)	\
{	\
	return write_uintmax(B, src, dest);	\
}	\
	\
template<class T>	\
typename std::enable_if<std::is_same<A, T>::value, void>::type write(const T& src, FILE* dest, uintmax_t ub)	\
{	\
	return write_uintmax(ub, src, dest);	\
}

ZAIMONI_RW(unsigned char, UCHAR_MAX)
ZAIMONI_RW(unsigned short, USHRT_MAX)
ZAIMONI_RW(unsigned int, UINT_MAX)
ZAIMONI_RW(unsigned long, ULONG_MAX)
ZAIMONI_RW(uintmax_t, UINTMAX_MAX)

#undef ZAIMONI_RW

}	// namespace zaimoni

#endif

#endif
