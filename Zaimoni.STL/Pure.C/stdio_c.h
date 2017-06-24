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


#ifdef __cplusplus
extern "C" {
#endif

long get_filelength(FILE* src);
uintmax_t read_uintmax(uintmax_t ub,FILE* src);
void write_uintmax(uintmax_t ub,uintmax_t src,FILE* dest);

#ifdef __cplusplus
}	/* end extern "C" */
#endif

#endif
