#include "log.h"

#define PRINT_TO_FP(buf, len, pattern)                                    \
    do {                                                                  \
        char buf[len];                                                    \
        buf[strftime(buf, sizeof(buf), pattern, ev->time)] = '\0';        \
        fprintf(                                                          \
            ev->fp, "%s %-5s %s:%d: ",                                    \
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


static void simple_print(log_Event *ev) {
    PRINT_TO_FP(buf, 16, "%H:%M:%S");
}


static void default_print(log_Event *ev) {
    PRINT_TO_FP(buf, 64, "%Y-%m-%d %H:%M:%S");
}


const char* log_level_string(int level) {
    return level_strings[level];
}



void log_set_level(int level) {
    L.level = level;
}


void log_set_quiet(BOOL enable) {
    L.quiet = enable;
}


static void init_event(log_Event *ev, void *fp) {
    if (!ev->time) {
        time_t t = time(NULL);
        ev->time = localtime(&t);
    }
    ev->fp = fp;
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
        FILE *fp = fopen("./log.txt", "a+");
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
