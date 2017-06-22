/* logging.h */

#ifndef ZAIMONI_STL_PURE_C_LOGGING_H
#define ZAIMONI_STL_PURE_C_LOGGING_H

#ifdef __cplusplus
extern "C" {
#endif

int start_logfile(const char* const filename);
int continue_logfile(const char* const filename);
int end_logfile(void);
int resume_logging(void);
void suspend_logging(void);
int is_logfile_active(void);
int is_logfile_at_all(void);

/* add_log postpends a \n afterwards; inc_log doesn't */
/* both will shut down logging completely on an error */
void add_log(const char* const x);
void add_log_substring(const char* const x,size_t x_len);
void inc_log(const char* const x);
void inc_log_substring(const char* const x,size_t x_len);

/* remove noise from debugging (somewhat) */
long get_checkpoint(void);
void revert_checkpoint(long x);

#ifdef __cplusplus
}
#endif

#endif
