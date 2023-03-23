#ifndef LOG__H_
#define LOG__H_

#include <stdio.h>
#include <stdarg.h>
#include <time.h>

// =====================mode switch===========================
// #define LOG_PATH "xxx\\xxx"
#ifndef LOG_PATH
    #include <ShlObj.h>
#endif

// if define PRINT_TO_CONSOLE, print log to stderr.
// if define SIMPLE_PRINT_FORMAT, print log in simple format.
#define PRINT_TO_FILE
#ifndef PRINT_TO_FILE
    #define PRINT_TO_CONSOLE
#endif

#ifndef DEFAULT_PRINT_FORMAT
    #define SIMPLE_PRINT_FORMAT
#endif
// ===========================================================

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

void log_set_level(int level);
void log_set_quiet(BOOL enable);
void log_log(int level, const char *file, int line, const char *fmt, ...);

#define PRINT_TO_FP(buf, len, pattern)                                    \
    do {                                                                  \
        char buf[len];                                                    \
        buf[strftime(buf, sizeof(buf), pattern, ev->time)] = '\0';        \
        fprintf(                                                          \
            ev->fp, "%s %-5s %s:%-5d: ",                                  \
            buf, level_strings[ev->level], ev->file, ev->line);           \
        vfprintf(ev->fp, ev->fmt, ev->ap);                                \
        fprintf(ev->fp, "\n");                                            \
        fflush(ev->fp); } while(0)


static struct {
    void *fp;
    int level;
    BOOL quiet;
} L;


static const char *level_strings[] = {
    "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

#ifdef SIMPLE_PRINT_FORMAT

    static void simple_print(log_Event *ev) {
        PRINT_TO_FP(buf, 16, "%H:%M:%S");
    }

#else

    static void default_print(log_Event *ev) {
        PRINT_TO_FP(buf, 64, "%Y-%m-%d %H:%M:%S");
    }

#endif

void log_set_level(int level) {
    L.level = level;
}


void log_set_quiet(BOOL enable) {
    L.quiet = enable;
}


static void init_event(log_Event *ev, void *fp) {
    if (!ev->time) {
        time_t t = time(0);
        ev->time = localtime(&t);
    }
    ev->fp = fp;
}

void GetSuffix(wchar_t *module_name) {
    int cur = 0, pos = 0;
    while(module_name[cur] != '\0') {
        if(module_name[cur] == '\\') {
            pos = cur;
        }
        ++cur;
    }
    if( !pos ) return;
    for(cur = 0, ++pos; module_name[pos] != '\0'; ++cur, ++pos) {
        module_name[cur] = module_name[pos];
    }
    module_name[cur] = '\0';
}

void log_log(int level, const char *file, int line, const char *fmt, ...) {
    log_Event ev = {
        .fmt   = fmt,
        .file  = file,
        .line  = line,
        .level = level,
    };

    if (!L.quiet && level >= L.level) {

    #ifdef PRINT_TO_CONSOLE
        init_event(&ev, stderr);
    #else
        #ifdef LOG_PATH
            FILE *fp = fopen(LOG_PATH, "a+");
        #else
            wchar_t path[MAX_PATH] = { 0 };
            wchar_t module_name[MAX_PATH] = { 0 };
            SHGetSpecialFolderPathW(0, path, CSIDL_DESKTOP, 0);
            GetModuleFileNameW(0, module_name, MAX_PATH);
            GetSuffix(module_name);
            wcscat(path, L"\\");
            wcscat(path, module_name);
            wcscat(path, L"_log.dat");
            FILE *fp = _wfopen(path, L"a+");
        #endif

        if( !fp ) {
            return;
        }
        init_event(&ev, fp);
    #endif

        va_start(ev.ap, fmt);

    #ifdef SIMPLE_PRINT_FORMAT
        simple_print(&ev);
    #else
        default_print(&ev);
    #endif

        va_end(ev.ap);
    #ifndef PRINT_TO_CONSOLE
        fclose(ev.fp);
    #endif
    }
}


#endif