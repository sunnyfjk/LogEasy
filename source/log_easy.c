//
// Created by fjk on 2023/6/24.
//
#include <stdarg.h>
#include "log_easy_stream_class.h"
#include "log_easy_class.h"
#include "log_easy_config.h"
#include "log_easy.h"

static int init=0;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void log_easy_init(void){
    pthread_mutex_lock(&lock);
    if(init)
        goto end_log_easy_init;

    log_easy_stream_init();

    log_easy_config_init();

    init = 1;
    end_log_easy_init:
    pthread_mutex_unlock(&lock);
}

void log_easy_exit(void){
    pthread_mutex_lock(&lock);
    if(!init)
        goto end_log_easy_exit;

    log_easy_stream_exit();

    log_easy_config_exit();

    init = 0;
    end_log_easy_exit:
    pthread_mutex_unlock(&lock);
}

void log_easy_printf(void *ctx,const char *type,const char *fmt,...){
    va_list  args;
    va_start(args,fmt);
    log_easy_class_vprintf(ctx,type,fmt,args);
    va_end(args);
}

void log_easy_vprintf(void *ctx,const char *type,const char *fmt, va_list args){
    log_easy_class_vprintf(ctx,type,fmt,args);
}

void log_easy_raw_printf(void *ctx,const char *type,const char *fmt,...){
    va_list  args;
    va_start(args,fmt);
    log_easy_class_raw_vprintf(ctx,type,fmt,args);
    va_end(args);
}

void log_easy_raw_vprintf(void *ctx,const char *type,const char *fmt, va_list args){
    log_easy_class_raw_vprintf(ctx,type,fmt,args);
}

void *log_easy_find(const char *name) {
    return log_easy_class_find(name);
}
