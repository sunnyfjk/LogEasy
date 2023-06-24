//
// Created by fjk on 2023/6/24.
//

#ifndef MMFRAMEWORK_LOG_EASY_CLASS_H
#define MMFRAMEWORK_LOG_EASY_CLASS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <list.h>
#include <pthread.h>

struct appender_class_t{
    struct list_head node;
    char appender_class_type[128];
    char appender_class_path[128];
    pthread_mutex_t lock;
    int quote;
    void *ctx;
    void *(*appender_class_open)(char *path);
    void (*appender_class_close)(void *ctx);
    void (*appender_class_vprintf)(void *ctx, const char *format, va_list va);
    void (*appender_class_raw_vprintf)(void *ctx, const char *format, va_list va);
};

struct category_class_t{
    struct list_head node;
    char category_class_type[128];
    char appender_class_type[128];
    struct appender_class_t *appender_class;
};

struct log_easy_class_t{
    char log_easy_class_type[128];
    struct list_head node;
    struct list_head category_class_head;
    struct list_head appender_class_head;
#define LOG_EASY_LOCK
#if defined(LOG_EASY_LOCK)
    pthread_mutex_t lock;
#endif
    int (*log_easy_class_match)(struct appender_class_t *ac,struct category_class_t *cc);
};

int appender_class_regitser(struct log_easy_class_t *le,struct appender_class_t *appender_class);

void appender_class_unregister(struct log_easy_class_t *le,struct appender_class_t *appender_class);

int category_class_register(struct log_easy_class_t *le,struct category_class_t *category_class);

void category_class_unregister(struct log_easy_class_t *le,struct category_class_t *category_class);

int log_easy_class_register(struct log_easy_class_t *le);

void log_easy_class_unregister(struct log_easy_class_t *le);

struct log_easy_class_t *log_easy_class_find(const char *name);

void log_easy_class_vprintf(struct log_easy_class_t *le,const char *type,const char *fmt,va_list ap);

void log_easy_class_raw_vprintf(struct log_easy_class_t *le,const char *type,const char *fmt,va_list ap);

#endif //MMFRAMEWORK_LOG_EASY_CLASS_H
