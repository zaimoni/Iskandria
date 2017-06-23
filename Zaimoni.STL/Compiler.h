/* Compiler.h */
/* cross-compiler compatiblity header */
/* (C)2009,2010 Kenneth Boyd, license: MIT.txt */

#ifndef ZAIMONI_COMPILER_H
#define ZAIMONI_COMPILER_H 1

/* C standard headers */
#include <stdint.h>
#ifndef __cplusplus
#include <stdbool.h>
#endif

/* This macro turns on trying to compile as ISO, rather than with the memory manager extensions */
#ifdef ZAIMONI_FORCE_ISO
#define ZAIMONI_LEN_WITH_NULL(A) ((A)+1)
#define ZAIMONI_NULL_TERMINATE(A) A = '\0'
#endif

/* delayed expansion of preprocessing operators */
#define STRINGIZE(A) #A
#define DEEP_STRINGIZE(A) STRINGIZE(A)
#define CONCATENATE(A,B) A ## B
#define DEEP_CONCATENATE(A,B) CONCATENATE(A,B)

/* static assertions */
#if __cplusplus >= 201103L
#define ZAIMONI_STATIC_ASSERT(A) static_assert(A,#A)
#elif __STDC_VERSION__ >= 201112l
#define ZAIMONI_STATIC_ASSERT(A) _Static_assert(A,#A)
#else
/* using MSVC approach */
#define ZAIMONI_STATIC_ASSERT(A) typedef char DEEP_CONCATENATE(static_assert_,__LINE__)[(A) ? 1 : -1]
#endif

/* size of a static array */
#define STATIC_SIZE(A) (sizeof(A)/sizeof(*A))

/* some macros to help with aggregate initialization */
#define DICT_STRUCT(A) { (A), sizeof(A)-1 }
#define DICT2_STRUCT(A,B) { (A), sizeof(A)-1, (B) }

/* platform config copied from Boost */
/* would prefer to use BOOST_PLATFORM, but strings aren't allowed in preprocessor tests */
/* for now, detect Mac OS X by MACOSX pendng proper documentation */
#if defined(linux) || defined(__linux) || defined(__linux__)
#	define ZAIMONI_PLATFORM_LINUX 1
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)
#	define ZAIMONI_PLATFORM_BSD	1
#elif defined(sun) || defined(__sun)
#	define ZAIMONI_PLATFORM_SOLARIS	1
#elif defined(__sgi)
#	define ZAIMONI_PLATFORM_IRIX	1
#elif defined(__hpux)
#	define ZAIMONI_PLATFORM_HPUX	1
#elif defined(__CYGWIN__)
#	define ZAIMONI_PLATFORM_CYGWIN	1
#elif defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#	define ZAIMONI_PLATFORM_WIN32	1
#elif defined(__BEOS__)
#	define ZAIMONI_PLATFORM_BEOS	1
#elif defined(macintosh) || defined(__APPLE__) || defined(__APPLE_CC__)
#	define ZAIMONI_PLATFORM_MACOS	1
#elif defined(__IBMCPP__) || defined(_AIX)
#	define ZAIMONI_PLATFORM_AIX	1
#elif defined(__amigaos__)
#	define ZAIMONI_PLATFORM_AMIGA	1
/* XOPEN implies POSIX implies UNIX */
#elif defined(_XOPEN_SOURCE)
#	define ZAIMONI_PLATFORM_GENERIC_XOPEN	1
#elif defined(_POSIX_SOURCE)
#	define ZAIMONI_PLATFORM_GENERIC_POSIX	1
#elif defined(unix)  || defined(__unix)
#	define ZAIMONI_PLATFORM_GENERIC_UNIX	1
#endif

/* filesystem weirdness */
#define ZAIMONI_TARGET_FILESYSTEM_POSIX 1
#define ZAIMONI_TARGET_FILESYSTEM_WINDOWS 2
#ifndef ZAIMONI_TARGET_FILESYSTEM
#	if defined(ZAIMONI_PLATFORM_WIN32)
#		define ZAIMONI_TARGET_FILESYSTEM ZAIMONI_TARGET_FILESYSTEM_WINDOWS
#	else
#		define ZAIMONI_TARGET_FILESYSTEM ZAIMONI_TARGET_FILESYSTEM_POSIX
#	endif
#endif
/* z_realpath macro requires unistd.h to be included first */
#if ZAIMONI_TARGET_FILESYSTEM==ZAIMONI_TARGET_FILESYSTEM_WINDOWS
#	define ZAIMONI_PATH_SEP "\\"
#	define ZAIMONI_PATH_SEP_CHAR '\\'
#	define z_realpath(TARGET,SRC) _fullpath(TARGET,SRC,sizeof(TARGET))
#elif ZAIMONI_TARGET_FILESYSTEM==ZAIMONI_TARGET_FILESYSTEM_POSIX
#	define ZAIMONI_PATH_SEP "/"
#	define ZAIMONI_PATH_SEP_CHAR '/'
#	define z_realpath(TARGET,SRC) realpath(TARGET,SRC)
#endif

/* hooks for properties of the libraries */
/* undefine this if C/C++ library sqrt() is inaccurate [use _root(x,2) in med_alg.hpp to replace] */
#define ZAIMONI_ALLOW_SQRT 1

/* hooks for C/C++ adapter code */
#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C extern
#endif

/* hooks for compiler extensions
   CONST_C_FUNC: like PURE_C_FUNC, but also cannot access global memory (through pointers or global variables)
   PURE_C_FUNC: no effects except return value, return value depends only on parameters and global variables
   PTR_RESTRICT: C99 restrict semantics (pointer may be assumed not to be aliased) */
#define NO_RETURN
#define CONST_C_FUNC
#define PURE_C_FUNC
#define ALL_PTR_NONNULL
#define PTR_NONNULL(A)
#define PTR_RESTRICT

/* GCC */
#ifdef __GNUC__
#define THROW_COMPETENT 1
#undef NO_RETURN
#undef CONST_C_FUNC
#undef PURE_C_FUNC
#undef ALL_PTR_NONNULL
#undef PTR_NONNULL
#undef PTR_RESTRICT
#define NO_RETURN __attribute__ ((noreturn))
#define CONST_C_FUNC __attribute__ ((const))
#define PURE_C_FUNC __attribute__ ((pure))
#define ALL_PTR_NONNULL __attribute__ ((nonnull))
#define PTR_NONNULL(A) __attribute__ ((nonnull A))
#define PTR_RESTRICT __restrict__
#endif

/* other compiler blocks */

/* THROW_COMPETENT: define if the compiler understands throw () as a member function modifier
   controls definition of THROW(A) macro */
#undef THROW
#ifdef THROW_COMPETENT
#define THROW(A) throw (A)
#undef THROW_COMPETENT
#else
#define THROW(A)
#endif

#if 0
/* #ifdef __cplusplus */
/* Danny Kelev: testing POD-struct
 * http://www.devx.com/cplus/10MinuteSolution/24908/0/page/3
 * compiler error when not POD, so unsuitable for Boost
 * use this to determine whether to override boost::is_pod<T> for a class */

namespace zaimoni {

template<class T>
struct POD_test
{
 POD_test(){constraints();} /* forces compile-time evaluation  */
private:
 static void constraints()
 {
  union{ T t;} u;
 }
};

template<typename T>
struct CPU_info
{
	static const unsigned int num = 1U;
	static bool little_endian() {return 1U==reinterpret_cast<unsigned char*>(&num)[0];};
};

}
#endif

#endif	/* ZAIMONI_COMPILER_HPP */
