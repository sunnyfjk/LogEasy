//
// Created by fjk on 2023/6/23.
//

#ifndef MMFRAMEWORK_APPENDER_H
#define MMFRAMEWORK_APPENDER_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <pthread.h>
struct log_easy_appender_t{
    char name[128];
    int flags;
    void *(*log_easy_open)(const char *path, int flags);
    void (*log_easy_close)(void *ctx);
    void (*log_easy_vprintf)(void *ctx,const char *fmt,va_list ap);
    void (*log_easy_raw_vprintf)(void *ctx,const char *fmt,va_list ap);
};

int log_easy_appender_register(struct log_easy_appender_t *a,size_t count);

void log_easy_appender_unregister(struct log_easy_appender_t *a);

struct log_easy_appender_t *log_easy_appender_find(const char *name);

#endif //MMFRAMEWORK_APPENDER_H
