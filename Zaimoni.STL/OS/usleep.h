/* usleep.h */

#ifndef ZAIMONI_STL_OS_USLEEP_H
#define ZAIMONI_STL_OS_USLEEP_H 1

#include "../Compiler.h"

#ifdef ZAIMONI_PLATFORM_WIN32
/* Windows: emulate with Sleep */

#ifdef __cplusplus
extern "C" {
#endif
__stdcall void Sleep(unsigned long);	/* don't pull in all of the Windows headers */
#ifdef __cplusplus
}
#endif
inline void usleep(int v) {Sleep(v / 1000);}

#else /* ZAIMONI_PLATFORM_WIN32 */

/* assume POSIX-ish; usleep is declared in unistd.h */
#include <unistd.h>

#endif /* ZAIMONI_PLATFORM_WIN32 */

#endif	/* ZAIMONI_STL_OS_USLEEP_H */
