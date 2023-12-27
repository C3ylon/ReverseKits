#include "log.h"

#define PRINT_TO_FP(buf, len, pattern)                                    \
    do {                                                                  \
        char buf[len];                                                    \
        buf[strftime(buf, sizeof(buf), pattern, ev->time)] = '\0';        \
        fprintf(                                                          \
            /* ev->fp, "%s %-5s %s:%-5d: ", */                            \
            /* buf, level_strings[ev->level], ev->file, ev->line); */     \
            ev->fp, "%s %-5s : " ,                                        \
            buf, level_strings[ev->level]);                               \
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

#ifndef LOG_PATH
static wchar_t g_path[MAX_PATH] = { 0 };
static void InitPath() {
    if(g_path[0] != '\0')
        return ;
    wchar_t module_name[MAX_PATH] = { 0 };
    SHGetSpecialFolderPathW(0, g_path, CSIDL_DESKTOP, 0);
    GetModuleFileNameW(0, module_name, MAX_PATH);
    GetSuffix(module_name);
    wcscat(g_path, L"\\");
    wcscat(g_path, module_name);
    wcscat(g_path, L"_log.dat");
}
#endif

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
            InitPath();
            FILE *fp = _wfopen(g_path, L"a+");
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
