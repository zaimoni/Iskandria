/* stdio_c.c */

#include "stdio_c.h"

#include "comptest.h"
#if ZAIMONI_HAVE_MICROSOFT_IO_H
#include <io.h>
#endif

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

