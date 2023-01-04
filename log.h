#ifndef LOG__H_
#define LOG__H_

#include <stdio.h>
#include <stdarg.h>
#include <time.h>


#ifndef TRUE
    #define TRUE 1
#endif

#ifndef FALSE
    #define FALSE 0
#endif

typedef int BOOL;

typedef struct {
    va_list ap;
    const char *fmt;
    const char *file;
    struct tm *time;
    void *fp;
    int line;
    int level;
} log_Event;


enum { LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL };

#define log_trace(...) log_log(LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...) log_log(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...)  log_log(LOG_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...)  log_log(LOG_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) log_log(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...) log_log(LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)

const char* log_level_string(int level);
void log_set_level(int level);
void log_set_quiet(BOOL enable);
void log_log(int level, const char *file, int line, const char *fmt, ...);

#endif