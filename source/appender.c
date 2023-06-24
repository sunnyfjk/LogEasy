//
// Created by fjk on 2023/6/23.
//
#include <stdint.h>
#include <pthread.h>
#include "appender.h"
#include "list.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
static LIST_HEAD(appender_head);
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

struct log_easy_appender_node_t {
    struct list_head node;
    struct log_easy_appender_t *a;
};

struct log_easy_appender_node_t *_log_easy_appender_register(struct log_easy_appender_t *a) {
    struct log_easy_appender_node_t *n = calloc(sizeof(*n), 1);
    if (n == NULL)
        return NULL;
    INIT_LIST_HEAD(&(n->node));
    n->a = a;
    return n;
    free(n);
    return NULL;
}

void _log_easy_appender_unregister(struct log_easy_appender_node_t *n) {
    struct log_easy_appender_t *a = n->a;
}

int log_easy_appender_register(struct log_easy_appender_t *a, size_t count) {
    int i = 0;
    pthread_mutex_lock(&lock);
    for (i = 0; i < count; i++) {
        struct log_easy_appender_node_t *n = _log_easy_appender_register(&(a[i]));
        if (n == NULL) {
            goto error_create_log_easy_appender_node;
        }
        list_add(&(n->node), &appender_head);
    }
    pthread_mutex_unlock(&lock);
    return 0;
    error_create_log_easy_appender_node:
    if (!list_empty_careful(&appender_head)) {
        struct log_easy_appender_node_t *pos = NULL, *n = NULL;
        list_for_each_entry_safe(pos, n, &appender_head, node) {
            list_del_init(&pos->node);
            _log_easy_appender_unregister(pos);
            free(pos);
        }
    }
    pthread_mutex_unlock(&lock);
    return -1;
}

void log_easy_appender_unregister(struct log_easy_appender_t *a) {
    pthread_mutex_lock(&lock);
    if (!list_empty_careful(&appender_head)) {
        struct log_easy_appender_node_t *pos = NULL, *n = NULL;
        list_for_each_entry_safe(pos, n, &appender_head, node) {
            if (pos->a == a) {
                list_del_init(&pos->node);
                _log_easy_appender_unregister(pos);
                free(pos);
                break;
            }
        }
    }
    pthread_mutex_unlock(&lock);
}

struct log_easy_appender_t *log_easy_appender_find(const char *name) {
    struct log_easy_appender_node_t *pos = NULL, *n = NULL;
    pthread_mutex_lock(&lock);
    if (list_empty_careful(&appender_head)) {
        pthread_mutex_unlock(&lock);
        return NULL;
    }
    list_for_each_entry_safe(pos, n, &appender_head, node) {
        struct log_easy_appender_t *a = pos->a;
        if (!strcmp(a->name, name)) {
            pthread_mutex_unlock(&lock);
            return a;
        }
    }
    pthread_mutex_unlock(&lock);
    return NULL;
}