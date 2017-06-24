/* stdio_c.c */

#include "stdio_c.h"

#include "comptest.h"
#if ZAIMONI_HAVE_MICROSOFT_IO_H
#include <io.h>
#endif

#include "../Logging.h"

long get_filelength(FILE* src)
{
#if ZAIMONI_HAVE_MICROSOFT_IO_H
	return _filelength(_fileno(src));
#else
	// Failing over to implementation-defined extension of ISO C.
	fpos_t current_pos;
	if (fgetpos(src,&current_pos)) return -1;
	if (fseek(src,0,SEEK_END)) return -1;	// ISO does not guarantee this works.
	long src_len = ftell(src);
	if (fsetpos(src,&current_pos)) return -1;
	return src_len;
#endif	
}

#define CONDITIONAL_READ(T)	\
	if ((T)(-1)>=ub)	\
		{	\
		T tmp;	\
		if (1!=fread(&tmp, sizeof(tmp), 1, src)) _fatal_code("failed to read " #T,3);	\
		return tmp;	\
		}

uintmax_t read_uintmax(uintmax_t ub,FILE* src)
{
	CONDITIONAL_READ(unsigned char);
	CONDITIONAL_READ(unsigned short);
	CONDITIONAL_READ(unsigned int);
	CONDITIONAL_READ(unsigned long);
	/* if ((unsigned long long)(-1) >= ub) */
		{
		uintmax_t tmp;
		if (1!=fread(&tmp, sizeof(tmp), 1, src)) _fatal_code("failed to read uintmax_t",3);
		return tmp;
		}
}

#undef CONDITIONAL_READ

#define CONDITIONAL_WRITE(T)	\
	if ((T)(-1)>=ub)	\
		{	\
		T tmp = src;	\
		if (1!=fwrite(&tmp, sizeof(tmp), 1, dest)) _fatal_code("failed to write " #T,3);	\
		return;	\
		}

void write_uintmax(uintmax_t ub,uintmax_t src,FILE* dest)
{
	CONDITIONAL_WRITE(unsigned char);
	CONDITIONAL_WRITE(unsigned short);
	CONDITIONAL_WRITE(unsigned int);
	CONDITIONAL_WRITE(unsigned long);
	/* if ((unsigned long long)(-1) >= ub) */
		{
		if (1!=fwrite(&src, sizeof(src), 1, dest)) _fatal_code("failed to write uintmax_t",3);
	/*	return;	*/
		}
}

