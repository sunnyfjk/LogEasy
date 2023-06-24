//
// Created by fjk on 2023/6/23.
//

#ifndef MMFRAMEWORK_CATEGORY_H
#define MMFRAMEWORK_CATEGORY_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <appender.h>

void *log_easy_category_find(const char *name);

int log_easy_category_register(const char *name,const char *path,struct log_easy_appender_t *a);

void log_easy_category_unregister(const char *name);

void log_easy_category_deregister(void);

void log_easy_category_printf(void *ctx,const char *fmt,...);

void log_easy_category_vprintf(void *ctx,const char *fmt,va_list);

void log_easy_category_raw_printf(void *ctx,const char *fmt,...);

void log_easy_category_raw_vprintf(void *ctx,const char *fmt,va_list);

#endif //MMFRAMEWORK_CATEGORY_H
