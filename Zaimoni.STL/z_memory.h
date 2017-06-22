/* z_memory.h */
/* external header for Zaimoni.com memory manager */
/* (C)2009,2015 Kenneth Boyd, license: MIT.txt */

#ifndef ZAIMONI_STL_Z_MEMORY_H
#define ZAIMONI_STL_Z_MEMORY_H 1

#ifdef ZAIMONI_FORCE_ISO
#error Zaimoni.STL/z_memory.h is for knowingly using the ISO-violating extensions of the memory manager.
#endif
#ifndef USE_ZAIMONI_MEMORY_MANAGER
#define USE_ZAIMONI_MEMORY_MANAGER 1
#endif

#include <stdlib.h>

/*
 * this memory manager uses sizeof(intptr_t) NUL characters as guard bytes
 * so we can go non-ANSI safely
 */
#define ZAIMONI_LEN_WITH_NULL(A) (A)
#define ZAIMONI_NULL_TERMINATE(A)

/*
 * our realloc implements realloc(x,0) is NULL
 */
#undef ZAIMONI_REALLOC_TO_ZERO_IS_NULL
#define ZAIMONI_REALLOC_TO_ZERO_IS_NULL 1

/*
 * defined in memory.cpp: controls Microsoft bypass for realloc.
 * Set to something non-zero in your program entry point, *immediately*.
 */
extern size_t AppRunning;	

#ifdef __cplusplus
extern "C"
#endif
inline size_t __cdecl _msize(const void* memblock)
{	/* FORMALLY CORRECT: Kenneth Boyd, 9/15/1999 */
#ifdef __cplusplus
	return reinterpret_cast<const size_t*>(reinterpret_cast<const char*>(memblock)-sizeof(size_t))[0];
#else
	return ((const size_t*)((const char*)(memblock)-sizeof(size_t)))[0];
#endif
}

#ifdef __cplusplus
namespace zaimoni {
template <class T> inline size_t
ArraySize(T* memblock)
{	/* FORMALLY CORRECT: Kenneth Boyd, 12/08/2004 */
	return _msize(memblock)/sizeof(T);
}

template <class T> inline size_t
SafeArraySize(T* memblock)
{	/* FORMALLY CORRECT: Kenneth Boyd, 11/2/2005 */
	return (memblock) ? _msize(memblock)/sizeof(T) : 0;
}

template <class T> inline size_t
ArraySize(const T* memblock)
{	/* FORMALLY CORRECT: Kenneth Boyd, 12/08/2004 */
	return _msize(const_cast<T*>(memblock))/sizeof(T);
}

template <class T> inline size_t
SafeArraySize(const T* memblock)
{	/* FORMALLY CORRECT: Kenneth Boyd, 11/2/2005 */
	return (memblock) ? _msize(const_cast<T*>(memblock))/sizeof(T) : 0;
}

}	/* namespace zaimoni */
#endif


/*!
 * checks that memory block in question is not only dynamically allocated, but is safe to free/realloc
 */
#ifdef __cplusplus
extern "C"
#endif
int _memory_block_start_valid(const void* x);

/*! 
 * checks that no guard bytes have been overwritten anywhere
 * this exposes a post-condition that triggers a memory-manager crash
 * really should be bool/_Bool return value
 * 
 * \return int 1 if ok, 0 if memory overwrites detected
 */
#ifdef __cplusplus
extern "C"
#endif
int _no_obvious_overwrites(void);

#ifdef __cplusplus

#ifndef ZAIMONI_STL_IN_MEMORY_CPP
/*
 * Sufficiently archaic compilers do not enforce the recent prohibition of not
 * inlining replacement new/delete operators.
 */
#ifdef __GNUC__
/* MingW32: 3.4.5 OK, 4.2.1 not.  Haven't checked 2.95.3. */
#if __GNUC__==3
#define ZAIMONI_STL_ARCHAISM_INLINE_DELETE 1
#endif
#endif /* __GNU__ */

#ifdef ZAIMONI_STL_ARCHAISM_INLINE_DELETE
/*
 * These must come before #include <new> to prevent compiler errors,
 * but don't show these to memory.cpp
 */
inline void operator delete(void* Target) throw ()
{/* FORMALLY CORRECT: 4/16/98, Kenneth Boyd */ free(Target);}

inline void operator delete[](void* Target) throw()
{/* FORMALLY CORRECT: 9/27/2005, Kenneth Boyd */ free(Target);}
#endif /* end ZAIMONI_STL_ARCHAISM_INLINE_DELETE */
#undef ZAIMONI_STL_ARCHAISM_INLINE_DELETE
#endif /* end ZAIMONI_STL_IN_MEMORY_CPP */

/* 
 * Now, trivial destructor and assignment alone are sufficient to make 
 * REALLOC safe for shrinking
 */
#include <new>

extern std::new_handler ZaimoniNewHandler; /* new handler for the custom memory manager */

#endif	/* end __cplusplus */
#endif	/* end ZAIMONI_STL_Z_MEMORY_H */

