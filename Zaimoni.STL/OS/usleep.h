/* usleep.h */

#ifndef ZAIMONI_STL_OS_USLEEP_H
#define ZAIMONI_STL_OS_USLEEP_H 1

#ifndef ZSTL_USLEEP_CODE
#include "../Compiler.h"
#ifdef ZAIMONI_PLATFORM_WIN32
#define ZSTL_USLEEP_CODE 2
#else /* ZAIMONI_PLATFORM_WIN32 */
#define ZSTL_USLEEP_CODE 1
#endif /* ZAIMONI_PLATFORM_WIN32 */
#endif

#if 1==ZSTL_USLEEP_CODE
#include <unistd.h>
#elif 2==ZSTL_USLEEP_CODE
#include <Windows.h>	// for Sleep

inline void usleep(int v) { Sleep(v / 1000); }
#endif

#endif	/* ZAIMONI_STL_OS_USLEEP_H */
