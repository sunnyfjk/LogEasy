//
// Created by fjk on 2023/6/23.
//

#include "log_easy.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <cJSON.h>
#include <appender.h>
#include <category.h>

#include <log_easy_appender_stream.h>

static int init=0;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

static struct log_easy_appender_t *log_easy_appender[]={
    &log_easy_appender_stream,
};


#define ARRAY_SIZE(a) (((sizeof(a))/(sizeof((a)[0]))))

const char *log_easy_config_path(void) {
    const char *name = NULL;
    int ret = 0;
    struct stat statbuf;

    static char path[128] = {0};
    /*系统环境变量定义配置文件*/
    name = getenv("LOG_EASY_PATH");
    if (name == NULL)
        goto export_log_easy_path;

    ret = stat(name, &statbuf);
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

static inline struct log_easy_appender_t *analysis_appender_config(cJSON *root){
    cJSON *item=NULL;
    struct log_easy_appender_t * a = NULL;

    item = cJSON_GetObjectItem(root,"name");
    if(item == NULL)
        return NULL;
    a = log_easy_appender_find(cJSON_GetStringValue(item));
    if(a==NULL)
        return NULL;
    return a;
}

static inline int analysis_category_config(const char *buf, size_t size) {
    cJSON *root = NULL, *array = NULL, *category_array_item = NULL;

    root = cJSON_ParseWithLength(buf, size);
    if (root == NULL)
        goto error_cJSON_ParseWithLength;

    array = cJSON_GetObjectItem(root, "category");
    if (array == NULL) {
        goto error_cJSON_GetObjectItem;
    }

    cJSON_ArrayForEach(category_array_item, array) {
        cJSON *stream_array = NULL, *name = NULL, *stream_array_item = NULL;
        name = cJSON_GetObjectItem(category_array_item, "name");
        if (name == NULL)
            goto error_cJSON_GetObjectItem;

        stream_array = cJSON_GetObjectItem(category_array_item, "stream");
        if (stream_array == NULL)
            goto error_cJSON_GetObjectItem;

        cJSON_ArrayForEach(stream_array_item, stream_array) {
            cJSON *appender = NULL,*path=NULL;
            struct log_easy_layout_t *l = NULL;
            struct log_easy_appender_t *a = NULL;
            int ret = 0;

            appender = cJSON_GetObjectItem(stream_array_item, "appender");
            if (appender == NULL)
                goto error_cJSON_GetObjectItem;

            path = cJSON_GetObjectItem(stream_array_item,"path");
            if (path == NULL)
                goto error_cJSON_GetObjectItem;


            a = analysis_appender_config(appender);
            if (a == NULL)
                continue;

            ret = log_easy_category_register(cJSON_GetStringValue(name),cJSON_GetStringValue(path), a);
            if (ret < 0)
                continue;
        }

    }

    return 0;
    error_cJSON_GetObjectItem:
    cJSON_Delete(root);
    error_cJSON_ParseWithLength:
    return -1;
}


int log_easy_init(void) {

    int fd = 0;
    int ret = 0;
    struct stat statbuf;
    char *buf = NULL;
    const char *path = NULL;
    size_t size = 0;
    int i = 0;


    pthread_mutex_lock(&lock);

    if(init!=0){
        pthread_mutex_unlock(&lock);
        return 0;
    }

    for(i=0;i<ARRAY_SIZE(log_easy_appender);i++){
        log_easy_appender_register(log_easy_appender[i], 1);
    }

    path = log_easy_config_path();
    if (path == NULL) {
        goto error_end;
    }
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

    analysis_category_config(buf, statbuf.st_size);

    free(buf);

    close(fd);
    init=1;
    pthread_mutex_unlock(&lock);
    return 0;
    error_read_file_err:
    close(fd);
    error_open_file:
    free(buf);
    error_end:
    pthread_mutex_unlock(&lock);
    return -1;
}

void log_easy_fini(void) {
    int i=0;
    pthread_mutex_lock(&lock);

    if(init != 1){
        pthread_mutex_unlock(&lock);
        return;
    }
    log_easy_category_deregister();

    for(i=0;i<ARRAY_SIZE(log_easy_appender);i++){
        log_easy_appender_unregister(log_easy_appender[i]);
    }

    init = 0;
    pthread_mutex_unlock(&lock);
}