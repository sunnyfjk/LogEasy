//
// Created by fjk on 2023/6/24.
//

#include "log_easy_stream_class.h"

static inline int log_easy_class_match(struct appender_class_t *ac, struct category_class_t *cc) {
    if (strncmp(ac->appender_class_type, cc->appender_class_type, sizeof(ac->appender_class_type)))
        return 0;
    return 1;
}

static struct log_easy_class_t les = {
        .log_easy_class_type = {"system"},
        .log_easy_class_match = log_easy_class_match,
};

int log_easy_stream_init(void) {
    return log_easy_class_register(&les);;
}

void log_easy_stream_exit(void) {
    log_easy_class_unregister(&les);
}

int appender_stream_regitser(struct appender_class_t *appender_class) {
    appender_class_regitser(&les, appender_class);
}

void appender_stream_unregister(struct appender_class_t *appender_class) {
    appender_class_unregister(&les, appender_class);
}

int category_stream_register(struct category_class_t *category_class) {
    category_class_register(&les, category_class);
}

void category_stream_unregister(struct category_class_t *category_class) {
    category_class_unregister(&les, category_class);
}

void log_easy_stream_vprintf(const char *type, const char *fmt, va_list ap) {
    log_easy_class_vprintf(&les, type, fmt, ap);
}

void log_easy_stream_raw_vprintf(const char *type, const char *fmt, va_list ap) {
    log_easy_class_raw_vprintf(&les, type, fmt, ap);
}


