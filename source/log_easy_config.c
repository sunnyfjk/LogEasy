//
// Created by fjk on 2023/6/24.
//

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include "log_easy_config.h"
#include "log_easy_stream_class.h"

static inline char *log_easy_config_path(void) {
    const char *name = NULL;
    int ret = 0;
    struct stat statbuf;

    static char path[128] = {0};
    /*系统环境变量定义配置文件*/
    name = getenv("LOG_EASY_PATH");
    if (name == NULL)
        goto export_log_easy_path;

    snprintf(path, sizeof(path), "%s/log_easy.json", name);

    ret = stat(path, &statbuf);
    if (ret == 0)
        goto analysis_log_easy_config;

    /*家目录定义的配置文件*/
    export_log_easy_path:
    name = getenv("HOME");
    if (name == NULL)
        goto export_home_log_easy_path;

    snprintf(path, sizeof(path), "%s/.log_easy.json", name);

    ret = stat(path, &statbuf);
    if (ret == 0)
        goto analysis_log_easy_config;

    /*配置文件目录*/
    export_home_log_easy_path:
#if defined(LOG_EASY_PATH)
    name = LOG_EASY_PATH;
    snprintf(path, sizeof(path), "%s/log_easy.json", name);
    ret = stat(path, &statbuf);
    if (ret == 0)
        goto analysis_log_easy_config;
#endif
    /*程序运行路径配置文件*/
    snprintf(path, sizeof(path), "%s/log_easy.json", ".");
    ret = stat(path, &statbuf);
    if (ret == 0)
        goto analysis_log_easy_config;

    return NULL;
    analysis_log_easy_config:
    return path;
}

static inline cJSON *log_easy_config_json(const char *path) {
    int ret = 0, fd = 0, size = 0;
    struct stat statbuf;
    char *buf = NULL;
    cJSON *root = NULL;
    ret = stat(path, &statbuf);
    if (ret < 0)
        goto error_end;

    if (statbuf.st_size <= 0)
        goto error_end;

    buf = calloc(statbuf.st_size + 1, 1);
    if (buf == NULL)
        goto error_end;

    fd = open(path, O_RDONLY);
    if (fd < 0)
        goto error_open_file;

    do {
        ret = read(fd, buf + size, statbuf.st_size - size);
        if (ret < 0)
            goto error_read_file_err;
        size += ret;
    } while (size != statbuf.st_size);

    root = cJSON_ParseWithLength(buf, size);
    if (root == NULL)
        goto error_josn_Parse;

    free(buf);

    close(fd);

    return root;
    error_josn_Parse:
    free(buf);
    error_read_file_err:
    close(fd);
    error_open_file:
    free(buf);
    error_end:
    return NULL;
}


struct system_normal_t {
    char path[128];
    FILE *fp;
};

static inline void *appender_class_open(char *path) {
    struct system_normal_t *sn = NULL;

    sn = calloc(sizeof(*sn), 1);
    if (sn == NULL)
        goto error_create_system_normal;


    if (!strncmp(path, "stderr", 128)) {
        sn->fp = stderr;
    } else if (!strncmp(path, "stdout", 128)) {
        sn->fp = stdout;
    } else {
        sn->fp = fopen(path, "a+");
    }

    if (sn->fp == NULL)
        goto error_appender_class_open;

    strncpy(sn->path, path, sizeof(sn->path));

    return sn;
    error_appender_class_open:
    free(sn);
    error_create_system_normal:
    return NULL;
}

static inline void appender_class_close(void *ctx) {
    struct system_normal_t *sn = ctx;

    if (!(strncmp(sn->path, "stderr", sizeof(sn->path)) || strncmp(sn->path, "stdout", sizeof(sn->path)))) {
        fclose(sn->fp);
    }
    free(sn);
}

static inline void appender_class_vprintf(void *ctx, const char *format, va_list va) {
    struct system_normal_t *sn = ctx;
    struct timeval val;
    struct tm tm;
    time_t tv_sec = 0;
    gettimeofday(&val, NULL);
    tv_sec = val.tv_sec;
    localtime_r(&tv_sec, &tm);


    fprintf(sn->fp, "[%04d-%02d-%02d %02d:%02d:%02d.%03ld]", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
            tm.tm_hour, tm.tm_min, tm.tm_sec,
            val.tv_usec / 1000);

    vfprintf(sn->fp, format, va);
}

static inline void appender_class_raw_vprintf(void *ctx, const char *format, va_list va) {
    struct system_normal_t *sn = ctx;
    vfprintf(sn->fp, format, va);
}

static LIST_HEAD(category_config_head);
static LIST_HEAD(appender_config_head);

struct analysis_category_t {
    struct list_head node;
    struct category_class_t cc;
};

struct analysis_appender_t {
    struct list_head node;
    struct appender_class_t ac;
};

static int analysis_category(cJSON *root) {
    cJSON *object = NULL;
    cJSON_ArrayForEach(object, root) {
        int ret = 0;
        cJSON *type = NULL, *appender = NULL;
        struct analysis_category_t *ac = NULL;
        struct category_class_t *cc = NULL;

        type = cJSON_GetObjectItem(object, "type");
        if (type == NULL)
            continue;

        appender = cJSON_GetObjectItem(object, "appender");
        if (appender == NULL)
            continue;

        ac = calloc(sizeof(*ac), 1);
        if (ac == NULL)
            goto error_end;

        INIT_LIST_HEAD(&(ac->node));
        cc = &(ac->cc);

        strncpy(cc->appender_class_type, cJSON_GetStringValue(appender), sizeof(cc->appender_class_type));
        strncpy(cc->category_class_type, cJSON_GetStringValue(type), sizeof(cc->category_class_type));

        ret = category_stream_register(cc);
        if (ret) {
            free(ac);
            goto error_end;
        }

    }
    return 0;
    error_end:
    if (!list_empty_careful(&category_config_head)) {
        struct analysis_category_t *pos = NULL, *n = NULL;
        list_for_each_entry_safe(pos, n, &category_config_head, node) {
            list_del_init(&pos->node);
            category_stream_unregister(&pos->cc);
            free(pos);
        }
    }
    return -1;
}

static inline int analysis_appender(cJSON *root) {
    cJSON *object = NULL;
    cJSON_ArrayForEach(object, root) {
        int ret = 0;
        cJSON *type = NULL, *path = NULL;
        struct analysis_appender_t *aa = NULL;
        struct appender_class_t *ac = NULL;

        type = cJSON_GetObjectItem(object, "type");
        if (type == NULL)
            continue;

        path = cJSON_GetObjectItem(object, "path");
        if (type == NULL)
            continue;


        aa = calloc(sizeof(*aa), 1);
        if (aa == NULL)
            goto error_end;

        INIT_LIST_HEAD(&aa->node);
        ac = &(aa->ac);

        strncpy(ac->appender_class_path, cJSON_GetStringValue(path), sizeof(ac->appender_class_path));
        strncpy(ac->appender_class_type, cJSON_GetStringValue(type), sizeof(ac->appender_class_type));

        ac->appender_class_open = appender_class_open;
        ac->appender_class_close = appender_class_close;
        ac->appender_class_vprintf = appender_class_vprintf;
        ac->appender_class_raw_vprintf = appender_class_raw_vprintf;

        ret = appender_stream_regitser(ac);
        if (ret) {
            free(aa);
            goto error_end;
        }
    }
    return 0;
    error_end:
    if (!list_empty_careful(&appender_config_head)) {
        struct analysis_appender_t *pos = NULL, *n = NULL;
        list_for_each_entry_safe(pos, n, &appender_config_head, node) {
            list_del_init(&pos->node);
            appender_stream_unregister(&pos->ac);
            free(pos);
        }
    }
    return -1;
}

void log_easy_config_init(void) {
    int ret = 0;
    char *path = NULL;
    cJSON *root = NULL;
    cJSON *category = NULL, *appender = NULL;

    path = log_easy_config_path();
    if (path == NULL)
        goto error_log_easy_config_path;

    root = log_easy_config_json(path);
    if (root == NULL)
        goto error_log_easy_config_path;

    category = cJSON_GetObjectItem(root, "category");
    if (category == NULL)
        goto error_cjson_get_object;

    appender = cJSON_GetObjectItem(root, "appender");
    if (appender == NULL)
        goto error_cjson_get_object;

    ret = analysis_category(category);
    if(ret)
        goto error_cjson_get_object;

    ret = analysis_appender(appender);
    if(ret)
        goto error_cjson_get_object;

    error_cjson_get_object:
    cJSON_Delete(root);
    error_log_easy_config_path:
    return;
}

void log_easy_config_exit(void) {

    if (!list_empty_careful(&appender_config_head)) {
        struct analysis_appender_t *pos = NULL, *n = NULL;
        list_for_each_entry_safe(pos, n, &appender_config_head, node) {
            list_del_init(&pos->node);
            appender_stream_unregister(&pos->ac);
            free(pos);
        }
    }

    if (!list_empty_careful(&category_config_head)) {
        struct analysis_category_t *pos = NULL, *n = NULL;
        list_for_each_entry_safe(pos, n, &category_config_head, node) {
            list_del_init(&pos->node);
            category_stream_unregister(&pos->cc);
            free(pos);
        }
    }

}

