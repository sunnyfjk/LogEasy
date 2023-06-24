//
// Created by fjk on 2023/6/24.
//

#ifndef MMFRAMEWORK_LOG_EASY_H
#define MMFRAMEWORK_LOG_EASY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void log_easy_init();

void log_easy_exit();

void *log_easy_find(const char *name);

void log_easy_printf(void *ctx,const char *type,const char *fmt,...);

void log_easy_vprintf(void *ctx,const char *type,const char *fmt, va_list args);

void log_easy_raw_printf(void *ctx,const char *type,const char *fmt,...);

void log_easy_raw_vprintf(void *ctx,const char *type,const char *fmt, va_list args);

#endif //MMFRAMEWORK_LOG_EASY_H
