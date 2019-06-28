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
intmax_t read_intmax(intmax_t ub,FILE* src);
void write_intmax(intmax_t ub,intmax_t src,FILE* dest);

#ifdef __cplusplus
}	/* end extern "C" */
#endif

/*
XXX have some natural C++ support here for binary file I/O 
template<class T> T zaimoni::read(FILE* src);
template<class T> void zaimoni::write(T src, FILE* dest)
etc.
*/
#ifdef __cplusplus
#include <limits.h>
#include <type_traits>
#include "../rw.hpp"

namespace zaimoni {

#define ZAIMONI_RW(A,B)	\
template<class T>	\
typename std::enable_if<std::is_same<A, typename std::remove_reference<T>::type>::value, typename std::remove_reference<T>::type>::type read(FILE* src)	\
{	\
	return (typename std::remove_reference<T>::type)read_uintmax(B, src);	\
}	\
	\
template<class T>	\
typename std::enable_if<std::is_same<A, typename std::remove_reference<T>::type>::value, typename std::remove_reference<T>::type>::type read(FILE* src, uintmax_t ub)	\
{	\
	return (typename std::remove_reference<T>::type)read_uintmax(ub, src);	\
}	\
	\
template<class T>	\
typename std::enable_if<std::is_same<A, typename std::remove_reference<T>::type>::value, void>::type write(const T& src, FILE* dest)	\
{	\
	return write_uintmax(B, src, dest);	\
}	\
	\
template<class T>	\
typename std::enable_if<std::is_same<A, typename std::remove_reference<T>::type>::value, void>::type write(const T& src, FILE* dest, uintmax_t ub)	\
{	\
	return write_uintmax(ub, src, dest);	\
}

ZAIMONI_RW(unsigned char, UCHAR_MAX)
ZAIMONI_RW(unsigned short, USHRT_MAX)
ZAIMONI_RW(unsigned int, UINT_MAX)
ZAIMONI_RW(unsigned long, ULONG_MAX)
ZAIMONI_RW(uintmax_t, UINTMAX_MAX)

#undef ZAIMONI_RW

#define ZAIMONI_RW(A,B)	\
template<class T>	\
typename std::enable_if<std::is_same<A, typename std::remove_reference<T>::type>::value, typename std::remove_reference<T>::type>::type read(FILE* src)	\
{	\
	return (typename std::remove_reference<T>::type)read_intmax(B, src);	\
}	\
	\
template<class T>	\
typename std::enable_if<std::is_same<A, typename std::remove_reference<T>::type>::value, typename std::remove_reference<T>::type>::type read(FILE* src, intmax_t ub)	\
{	\
	return (typename std::remove_reference<T>::type)read_intmax(ub, src);	\
}	\
	\
template<class T>	\
typename std::enable_if<std::is_same<A, typename std::remove_reference<T>::type>::value, void>::type write(const T& src, FILE* dest)	\
{	\
	return write_intmax(B, src, dest);	\
}	\
	\
template<class T>	\
typename std::enable_if<std::is_same<A, typename std::remove_reference<T>::type>::value, void>::type write(const T& src, FILE* dest, intmax_t ub)	\
{	\
	return write_intmax(ub, src, dest);	\
}

ZAIMONI_RW(signed char, SCHAR_MAX)
ZAIMONI_RW(signed short, SHRT_MAX)
ZAIMONI_RW(signed int, INT_MAX)
ZAIMONI_RW(signed long, LONG_MAX)
ZAIMONI_RW(intmax_t, INTMAX_MAX)

#undef ZAIMONI_RW

template<class T>
void read_iterator_count(T iter, size_t i, FILE* src)
{
	while(0<i)
		{
		*(iter++) = read<decltype(*iter)>(src);		
		--i;
		}
}

template<class T>
typename std::enable_if<1==zaimoni::rw_mode<T>::group_read , T>::type read(FILE* src)
{
	T dest;
	read_iterator_count(dest.begin(),dest.size(), src);
	return dest;
}

template<class T>
typename std::enable_if<1==zaimoni::rw_mode<T>::group_read , void>::type read(T& dest, FILE* src)
{
	read_iterator_count(dest.begin(),dest.size(), src);
}

template<class T>
typename std::enable_if<2==zaimoni::rw_mode<T>::group_read , void>::type read(T& dest, FILE* src)
{
	const size_t ub = dest.size();
	size_t i = 0;
	while(i < ub) dest[i++] = read<decltype(dest[i])>(src);
}

template<class T>
typename std::enable_if<3 == zaimoni::rw_mode<T>::group_read, T>::type read(FILE* src)
{
	return T(src);
}

template<class T>
void write_iterator_count(T iter, size_t i, FILE* dest)
{
	while(0<i)
		{
		write(*(iter++),dest);
		--i;
		}
}

template<class T>
typename std::enable_if<1==zaimoni::rw_mode<T>::group_write , void>::type write(const T& src, FILE* dest)
{
	write_iterator_count(src.begin(),src.size(),dest);
}

template<class T>
typename std::enable_if<2==zaimoni::rw_mode<T>::group_write , void>::type write(const T& src, FILE* dest)
{
	const size_t ub = src.size();
	size_t i = 0;
	while(i < ub) write(src[i++],dest);
}

template<class T>
typename std::enable_if<3 == zaimoni::rw_mode<T>::group_write, void>::type write(const T& src, FILE* dest)
{
	src.save(dest);
}

}	// namespace zaimoni

#endif

#endif
