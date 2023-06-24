//
// Created by fjk on 2023/6/24.
//

#ifndef MMFRAMEWORK_LOG_EASY_STREAM_CLASS_H
#define MMFRAMEWORK_LOG_EASY_STREAM_CLASS_H

#include "log_easy_class.h"

int appender_stream_regitser(struct appender_class_t *appender_class);

void appender_stream_unregister(struct appender_class_t *appender_class);

int category_stream_register(struct category_class_t *category_class);

void category_stream_unregister(struct category_class_t *category_class);

int log_easy_stream_init(void);

void log_easy_stream_exit(void);

void log_easy_stream_vprintf(const char *type,const char *fmt,va_list ap);

void log_easy_stream_raw_vprintf(const char *type,const char *fmt,va_list ap);

#endif //MMFRAMEWORK_LOG_EASY_STREAM_CLASS_H
