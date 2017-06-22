/* logging.c */

#include <stdio.h>
#include <string.h>
#include "logging.h"

static FILE* Logfile = NULL;
static FILE* master = NULL;

int start_logfile(const char* const filename)
{
	if (NULL!=master) return 0;
	master = fopen(filename,"wb");
	if (NULL==master) return 0;
	Logfile = master;
	return 1;
}

int continue_logfile(const char* const filename)
{
	if (NULL!=master) return 0;
	master = fopen(filename,"wb+");
	if (NULL==master) return 0;
	fseek(master,0,SEEK_END);
	Logfile = master;
	return 1;
}

int end_logfile(void)
{
	if (NULL==master) return 0;
	Logfile = NULL;
	fclose(master);	/* ignore the error code: exactly what are we supposed to do with an unsuccessfully closed file? */
	master = NULL;
	return 1;
}

int resume_logging(void)
{
	if (NULL==Logfile && NULL!=master)
		{
		Logfile = master;
		return 1;
		}
	return 0;
}

void suspend_logging(void)
{
	Logfile = NULL;
}

int is_logfile_active(void)
{
	return NULL!=Logfile;
}

int is_logfile_at_all(void)
{
	return NULL!=master;
}

/* if there is an error, we probably don't want to keep on using the file in these two */
void add_log(const char* const x)
{
	if (NULL==Logfile || NULL==x || '\0'== *x) return;	
	if (	!fwrite(x,strlen(x),1,Logfile)
		||	!fwrite("\n",1,1,Logfile)
		|| 	fflush(Logfile))
		end_logfile();
}

void add_log_substring(const char* const x,size_t x_len)
{
	if (NULL==Logfile || NULL==x || '\0'== *x || 0==x_len) return;
	if (	!fwrite(x,x_len,1,Logfile)
		||	!fwrite("\n",1,1,Logfile)
		|| 	fflush(Logfile))
		end_logfile();
}

void inc_log(const char* const x)
{
	if (NULL==Logfile || NULL==x || '\0'== *x) return;
	if (	!fwrite(x,strlen(x),1,Logfile)
		|| 	fflush(Logfile))
		end_logfile();
}

void inc_log_substring(const char* const x,size_t x_len)
{
	if (NULL==Logfile || NULL==x || '\0'== *x || 0==x_len) return;
	if (	!fwrite(x,x_len,1,Logfile)
		|| 	fflush(Logfile))
		end_logfile();
}

/* remove noise from debugging (somewhat) */
long get_checkpoint(void)
{
	if (NULL==Logfile) return -1;
	return ftell(Logfile);
}

void revert_checkpoint(long x)
{
	if (0<=x && NULL!=Logfile)
		{
		if (fseek(Logfile,x,SEEK_SET))
			end_logfile();
		};
}

