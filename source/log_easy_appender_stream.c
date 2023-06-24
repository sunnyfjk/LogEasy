//
// Created by fjk on 2023/6/23.
//

#include "log_easy_appender_stream.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

struct stream_ctx_t {
    FILE *file;
    char path[128];
};

static inline void *log_easy_open(const char *path, int flags) {

    struct stream_ctx_t *sc = calloc(sizeof(*sc), 1);
    if (sc == NULL)
        goto error_create_stream_ctx;

    strncpy(sc->path, path, sizeof(path));

    if (!strncmp(path, "stderr", sizeof(path))) {
        sc->file = stderr;
    } else if (!strncmp(path, "stdout", sizeof(path))) {
        sc->file = stdout;
    } else {
        sc->file = fopen(path, "a+");
        if (sc->file == NULL)
            goto error_log_easy_open_stream;
    }

    return sc;
    error_log_easy_open_stream:
    free(sc);
    error_create_stream_ctx:
    return NULL;
}

static inline void log_easy_close(void *ctx) {
    struct stream_ctx_t *sc = ctx;
    if (!(strncmp(sc->path, "stderr", sizeof(sc->path)) || strncmp(sc->path, "stdout", sizeof(sc->path)))) {
        fclose(sc->file);
    }
    free(sc);
}

static inline void log_easy_vprintf(void *ctx, const char *fmt, va_list ap) {
    struct stream_ctx_t *sc = ctx;
    struct timeval val;
    struct tm tm;
    time_t tv_sec = 0;
    gettimeofday(&val, NULL);
    tv_sec = val.tv_sec;
    gmtime_r(&tv_sec, &tm);


    fprintf(sc->file, "[%04d-%02d-%02d %02d:%02d:%02d.%03ld]", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
            tm.tm_hour, tm.tm_min, tm.tm_sec,
            val.tv_usec / 1000);

    vfprintf(sc->file,fmt,ap);
}

static inline void log_easy_raw_vprintf(void *ctx, const char *fmt, va_list ap){
    struct stream_ctx_t *sc = ctx;
    vfprintf(sc->file,fmt,ap);
}

struct log_easy_appender_t log_easy_appender_stream = {
        .name={"stream"},
        .flags=O_APPEND | O_RDWR | O_CREAT,
        .log_easy_open = log_easy_open,
        .log_easy_close = log_easy_close,
        .log_easy_vprintf = log_easy_vprintf,
        .log_easy_raw_vprintf = log_easy_raw_vprintf,
};