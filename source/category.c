//
// Created by fjk on 2023/6/23.
//
#include <pthread.h>
#include <stdarg.h>
#include "appender.h"
#include "category.h"
#include "list.h"

static LIST_HEAD(category_head);
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

struct log_easy_stream_node_t {
    struct list_head node;
    void *ctx;
    char path[128];
    pthread_mutex_t lock;
    struct log_easy_appender_t *a;
};

struct log_easy_category_node_t {
    struct list_head node;
    char name[128];
    struct list_head stream_head;
};

void *log_easy_category_find(const char *name) {
    struct log_easy_category_node_t *pos = NULL, *n = NULL;
    pthread_mutex_lock(&lock);
    if (list_empty_careful(&category_head)) {
        pthread_mutex_unlock(&lock);
        return NULL;
    }

    list_for_each_entry_safe(pos, n, &category_head, node) {
        if (!strncmp(pos->name, name, sizeof(pos->name))) {
            pthread_mutex_unlock(&lock);
            return pos;
        }
    }
    pthread_mutex_unlock(&lock);
    return NULL;
}

static inline struct log_easy_stream_node_t *
log_easy_stream_node_create(const char *path, struct log_easy_appender_t *a) {
    int ret = 0;
    struct log_easy_stream_node_t *n = NULL;
    n = calloc(sizeof(*n), 1);
    if (n == NULL)
        goto error_create_log_easy_stream_node;

    INIT_LIST_HEAD(&n->node);
    ret = pthread_mutex_init(&n->lock, NULL);
    if (ret < 0) {
        goto error_pthread_mutex_init;
    }

    pthread_mutex_lock(&(n->lock));
    n->ctx = a->log_easy_open(path, a->flags);
    pthread_mutex_unlock(&(n->lock));
    if(n->ctx == NULL)
        goto erro_open_log_file_err;

    strncpy(n->path, path, sizeof(n->path));

    n->a = a;

    return n;
    erro_open_log_file_err:
    pthread_mutex_destroy(&(n->lock));
    error_pthread_mutex_init:
    free(n);
    error_create_log_easy_stream_node:
    return NULL;
}

static inline void log_easy_stream_node_release(struct log_easy_stream_node_t *n) {
    if (!(strncmp(n->path, "stderr", sizeof(n->path)) || strncmp(n->path, "stdout", sizeof(n->path)))) {
        struct log_easy_appender_t *a = n->a;
        pthread_mutex_lock(&(n->lock));
        a->log_easy_close(n->ctx);
        pthread_mutex_unlock(&(n->lock));
    }
    pthread_mutex_destroy(&(n->lock));
}

int log_easy_category_register(const char *name, const char *path, struct log_easy_appender_t *a) {
    struct log_easy_category_node_t *ctx = NULL;
    struct log_easy_stream_node_t *n = NULL;
    ctx = log_easy_category_find(name);
    if (ctx == NULL) {
        ctx = calloc(sizeof(*ctx), 1);
        if (ctx == NULL)
            return -1;
        strncpy(ctx->name, name, sizeof(ctx->name));
        INIT_LIST_HEAD(&(ctx->stream_head));
        INIT_LIST_HEAD(&(ctx->node));
        pthread_mutex_lock(&lock);
        list_add(&(ctx->node), &category_head);
        pthread_mutex_unlock(&lock);
    }

    n = log_easy_stream_node_create(path, a);
    if (n == NULL)
        return -1;

    list_add(&(n->node), &(ctx->stream_head));
    return 0;
}

static inline void log_easy_category_node_release(struct log_easy_category_node_t *ctx){
    struct log_easy_stream_node_t *pos = NULL, *n = NULL;
    list_for_each_entry_safe(pos, n, &(ctx->stream_head), node) {
        struct log_easy_appender_t *a = pos->a;
        list_del_init(&(pos->node));
        log_easy_stream_node_release(pos);
        free(pos);
    }
}

void log_easy_category_unregister(const char *name) {
    struct log_easy_category_node_t *ctx = NULL;
    ctx = log_easy_category_find(name);
    if (ctx == NULL)
        return;
    pthread_mutex_lock(&lock);
    list_del_init(&(ctx->node));
    pthread_mutex_unlock(&lock);
    log_easy_category_node_release(ctx);
}
void log_easy_category_deregister(void){
    struct log_easy_category_node_t *pos = NULL, *n = NULL;
    pthread_mutex_lock(&lock);
    if (list_empty_careful(&category_head)) {
        pthread_mutex_unlock(&lock);
        return;
    }

    list_for_each_entry_safe(pos, n, &category_head, node) {
        list_del_init(&pos->node);
        log_easy_category_node_release(pos);

    }
    pthread_mutex_unlock(&lock);
}

void log_easy_category_printf(void *ctx, const char *fmt, ...) {
    struct log_easy_category_node_t *c = ctx;
    struct log_easy_stream_node_t *pos, *n;

    if (ctx == NULL || list_empty_careful(&c->stream_head))
        return;
    list_for_each_entry_safe(pos, n, &c->stream_head, node) {
        struct log_easy_appender_t *a = pos->a;
        va_list va;
        va_start(va, fmt);
        pthread_mutex_lock(&pos->lock);
        a->log_easy_vprintf(pos->ctx,fmt,va);
        pthread_mutex_unlock(&pos->lock);
        va_end(va);
    }
}

void log_easy_category_vprintf(void *ctx, const char *fmt, va_list ap) {
    struct log_easy_category_node_t *c = ctx;
    struct log_easy_stream_node_t *pos, *n;

    if (ctx == NULL || list_empty_careful(&c->stream_head))
        return;

    list_for_each_entry_safe(pos, n, &c->stream_head, node) {
        struct log_easy_appender_t *a = pos->a;
        va_list args;
        va_copy(args,ap);
        pthread_mutex_lock(&pos->lock);
        a->log_easy_vprintf(pos->ctx,fmt,args);
        pthread_mutex_unlock(&pos->lock);
    }
}

void log_easy_category_raw_printf(void *ctx, const char *fmt, ...) {
    struct log_easy_category_node_t *c = ctx;
    struct log_easy_stream_node_t *pos, *n;

    if (ctx == NULL || list_empty_careful(&c->stream_head))
        return;
    list_for_each_entry_safe(pos, n, &c->stream_head, node) {
        struct log_easy_appender_t *a = pos->a;
        va_list va;
        va_start(va, fmt);
        pthread_mutex_lock(&pos->lock);
        a->log_easy_raw_vprintf(pos->ctx,fmt,va);
        pthread_mutex_unlock(&pos->lock);
        va_end(va);
    }
}


void log_easy_category_raw_vprintf(void *ctx, const char *fmt, va_list ap) {
    struct log_easy_category_node_t *c = ctx;
    struct log_easy_stream_node_t *pos, *n;

    if (ctx == NULL || list_empty_careful(&c->stream_head))
        return;

    list_for_each_entry_safe(pos, n, &c->stream_head, node) {
        struct log_easy_appender_t *a = pos->a;
        va_list args;
        va_copy(args,ap);
        pthread_mutex_lock(&pos->lock);
        a->log_easy_raw_vprintf(pos->ctx,fmt,args);
        pthread_mutex_unlock(&pos->lock);
    }
}

