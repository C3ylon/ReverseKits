#include "log.h"

#define MAX_CALLBACKS 32

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

typedef struct {
    log_LogFn fn;
    void *fp;
    int level;
} Callback;

static struct {
    void *fp;
    log_LockFn lock;
    int level;
    bool quiet;
    Callback callbacks[MAX_CALLBACKS];
} L;


static const char *level_strings[] = {
    "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};


static void stdout_callback(log_Event *ev) {
    PRINT_TO_FP(buf, 16, "%H:%M:%S");
}


static void file_callback(log_Event *ev) {
    PRINT_TO_FP(buf, 64, "%Y-%m-%d %H:%M:%S");
}


static void lock(void)   {
    if (L.lock) { L.lock(true, L.fp); }
}


static void unlock(void) {
    if (L.lock) { L.lock(false, L.fp); }
}


const char* log_level_string(int level) {
    return level_strings[level];
}


void log_set_lock(log_LockFn fn, void *fp) {
    L.lock = fn;
    L.fp = fp;
}


void log_set_level(int level) {
    L.level = level;
}


void log_set_quiet(bool enable) {
    L.quiet = enable;
}


int log_add_callback(log_LogFn fn, void *fp, int level) {
    for (int i = 0; i < MAX_CALLBACKS; i++) {
        if (!L.callbacks[i].fn) {
        L.callbacks[i] = (Callback) { fn, fp, level };
        return 0;
        }
    }
    return -1;
}


int log_add_fp(FILE *fp, int level) {
    return log_add_callback(file_callback, fp, level);
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

    lock();

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
        stdout_callback(&ev);
        va_end(ev.ap);
      #ifndef PRINT_TO_CONSOLE
        fclose(ev.fp);
      #endif
    }

    for (int i = 0; i < MAX_CALLBACKS && L.callbacks[i].fn; i++) {
        Callback *cb = &L.callbacks[i];
        if (level >= cb->level) {
        init_event(&ev, cb->fp);
        va_start(ev.ap, fmt);
        cb->fn(&ev);
        va_end(ev.ap);
        }
    }

    unlock();
}
