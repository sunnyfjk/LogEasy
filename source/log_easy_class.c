//
// Created by fjk on 2023/6/24.
//

#include <stdarg.h>
#include "log_easy_class.h"

static LIST_HEAD(head);
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

static inline int appender_class_init(struct appender_class_t *ac) {

    int ret = 0;
    ret = pthread_mutex_init(&ac->lock, NULL);
    if (ret < 0)
        goto error_pthread_mutex_init;

    pthread_mutex_lock(&ac->lock);
    ac->ctx = ac->appender_class_open(ac->appender_class_path);
    pthread_mutex_unlock(&ac->lock);
    if (ac->ctx == NULL)
        goto error_appender_class_open;

    INIT_LIST_HEAD(&ac->node);

    return 0;
    error_appender_class_open:
    pthread_mutex_destroy(&ac->lock);
    error_pthread_mutex_init:
    return -1;
}

static inline void appender_class_exit(struct appender_class_t *ac) {
    INIT_LIST_HEAD(&ac->node);
    pthread_mutex_lock(&ac->lock);
    ac->appender_class_close(ac->ctx);
    pthread_mutex_unlock(&ac->lock);
    pthread_mutex_destroy(&ac->lock);
}

static inline void appender_mach_category_register(struct log_easy_class_t *le, struct appender_class_t *ac) {
    struct category_class_t *pos;

    if (list_empty_careful(&(le->category_class_head)))
        goto error_category_class_head_empty;

    list_for_each_entry(pos, &(le->category_class_head), node) {
        int ret = le->log_easy_class_match(ac, pos);
        if (ret == 0)
            continue;
        pos->appender_class = ac;
        pthread_mutex_lock(&ac->lock);
        ac->quote++;
        pthread_mutex_unlock(&ac->lock);
    }
    error_category_class_head_empty:
    return;
}

static inline void appender_mach_category_unregister(struct log_easy_class_t *le, struct appender_class_t *ac) {
    struct category_class_t *pos;

    if (list_empty_careful(&(le->category_class_head)))
        goto error_category_class_head_empty;

    list_for_each_entry(pos, &(le->category_class_head), node) {
        int ret = le->log_easy_class_match(ac, pos);
        if (ret == 0)
            continue;
        pos->appender_class = NULL;
        pthread_mutex_lock(&ac->lock);
        ac->quote--;
        pthread_mutex_unlock(&ac->lock);
    }
    error_category_class_head_empty:
    return;
}


int appender_class_regitser(struct log_easy_class_t *le, struct appender_class_t *appender_class) {
    int ret = 0;
    ret = appender_class_init(appender_class);
    if (ret)
        goto error_appender_class_init;
#if defined(LOG_EASY_LOCK)
    pthread_mutex_lock(&le->lock);
#endif
    list_add(&appender_class->node, &le->appender_class_head);
    appender_mach_category_register(le, appender_class);
#if defined(LOG_EASY_LOCK)
    pthread_mutex_unlock(&le->lock);
#endif
    error_appender_class_init:
    return ret;

}

void appender_class_unregister(struct log_easy_class_t *le, struct appender_class_t *appender_class) {
#if defined(LOG_EASY_LOCK)
    pthread_mutex_lock(&le->lock);
#endif
    list_del_init(&appender_class->node);
#if defined(LOG_EASY_LOCK)
    pthread_mutex_unlock(&le->lock);
#endif
    appender_mach_category_unregister(le, appender_class);
    appender_class_exit(appender_class);
}


static inline void category_mach_appender_register(struct log_easy_class_t *le, struct category_class_t *cc) {
    struct appender_class_t *pos;

    if (list_empty_careful(&(le->appender_class_head)))
        goto error_appender_class_head_empty;

    list_for_each_entry(pos, &(le->appender_class_head), node) {
        int ret = le->log_easy_class_match(pos, cc);
        if (ret == 0)
            continue;
        cc->appender_class = pos;
        pthread_mutex_lock(&pos->lock);
        pos->quote++;
        pthread_mutex_unlock(&pos->lock);
        break;
    }
    error_appender_class_head_empty:
    return;
}

static inline void category_mach_appender_unregister(struct log_easy_class_t *le, struct category_class_t *cc) {
    struct appender_class_t *pos;

    if (list_empty_careful(&(le->appender_class_head)))
        goto error_appender_class_head_empty;

    list_for_each_entry(pos, &(le->appender_class_head), node) {
        int ret = le->log_easy_class_match(pos, cc);
        if (ret == 0)
            continue;
        cc->appender_class = NULL;
        pthread_mutex_lock(&pos->lock);
        pos->quote--;
        pthread_mutex_unlock(&pos->lock);
        break;
    }
    error_appender_class_head_empty:
    return;
}

static inline int category_class_init(struct category_class_t *cc) {
    INIT_LIST_HEAD(&cc->node);
    return 0;
}

static inline void category_class_exit(struct category_class_t *cc) {

}

int category_class_register(struct log_easy_class_t *le, struct category_class_t *category_class) {
    int ret = 0;
    ret = category_class_init(category_class);
    if (ret)
        goto error_category_class_init;
#if defined(LOG_EASY_LOCK)
    pthread_mutex_lock(&le->lock);
#endif
    list_add(&category_class->node, &le->category_class_head);
    category_mach_appender_register(le, category_class);
#if defined(LOG_EASY_LOCK)
    pthread_mutex_unlock(&le->lock);
#endif
    error_category_class_init:
    return ret;
}

void category_class_unregister(struct log_easy_class_t *le, struct category_class_t *category_class) {
#if defined(LOG_EASY_LOCK)
    pthread_mutex_lock(&le->lock);
#endif
    list_del_init(&category_class->node);
    category_mach_appender_unregister(le, category_class);
#if defined(LOG_EASY_LOCK)
    pthread_mutex_unlock(&le->lock);
#endif
    category_class_exit(category_class);
}

static inline int log_easy_class_init(struct log_easy_class_t *le) {
    int ret = 0;
#if defined(LOG_EASY_LOCK)
    ret = pthread_mutex_init(&le->lock, NULL);
    if (ret)
        goto error_pthread_mutex_init;
#endif
    INIT_LIST_HEAD(&le->appender_class_head);
    INIT_LIST_HEAD(&le->category_class_head);
    INIT_LIST_HEAD(&le->node);
#if defined(LOG_EASY_LOCK)
    error_pthread_mutex_init:
#endif
    return ret;
}

static inline int log_easy_class_exit(struct log_easy_class_t *le) {
#if defined(LOG_EASY_LOCK)
    pthread_mutex_destroy(&le->lock);
#endif
}

int log_easy_class_register(struct log_easy_class_t *le) {
    int ret = log_easy_class_init(le);
    if (ret)
        goto error_log_easy_class_init;
    pthread_mutex_lock(&lock);
    list_add(&le->node, &head);
    pthread_mutex_unlock(&lock);
    error_log_easy_class_init:
    return 0;
}

void log_easy_class_unregister(struct log_easy_class_t *le) {
    pthread_mutex_lock(&lock);
    list_del_init(&le->node);
    pthread_mutex_unlock(&lock);
    log_easy_class_exit(le);
}


void log_easy_class_vprintf(struct log_easy_class_t *le, const char *type, const char *fmt, va_list ap) {
    struct category_class_t *pos = NULL;

    if (list_empty_careful(&(le->category_class_head)))
        goto error_category_class_head_empty;

    list_for_each_entry(pos, &(le->category_class_head), node) {
        struct appender_class_t *ac = pos->appender_class;
        va_list args;

        if (strncmp(pos->category_class_type, type, sizeof(pos->category_class_type)))
            continue;

        if (ac == NULL)
            continue;

        va_copy(args, ap);
        pthread_mutex_lock(&ac->lock);
        ac->appender_class_vprintf(ac->ctx, fmt, args);
        pthread_mutex_unlock(&ac->lock);
    }
    error_category_class_head_empty:
    return;
}

void log_easy_class_raw_vprintf(struct log_easy_class_t *le, const char *type, const char *fmt, va_list ap) {
    struct category_class_t *pos = NULL;

    if (list_empty_careful(&(le->category_class_head)))
        goto error_category_class_head_empty;

    list_for_each_entry(pos, &(le->category_class_head), node) {
        struct appender_class_t *ac = pos->appender_class;
        va_list args;

        if (strncmp(pos->category_class_type, type, sizeof(pos->category_class_type)))
            continue;

        if (ac == NULL)
            continue;

        va_copy(args, ap);
        pthread_mutex_lock(&ac->lock);
        ac->appender_class_raw_vprintf(ac->ctx, fmt, args);
        pthread_mutex_unlock(&ac->lock);
    }
    error_category_class_head_empty:
    return;
}

struct log_easy_class_t *log_easy_class_find(const char *name) {
    struct log_easy_class_t *pos = NULL;

    pthread_mutex_lock(&lock);

    if (list_empty_careful(&head))
        goto error_log_easy_class_empty;

    list_for_each_entry(pos, &head, node) {
        if (!strncmp(pos->log_easy_class_type, name, sizeof(pos->log_easy_class_type))) {
            pthread_mutex_unlock(&lock);
            return pos;
        }
    }
    error_log_easy_class_empty:
    pthread_mutex_unlock(&lock);
    return NULL;
}
