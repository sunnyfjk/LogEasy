# LogEasy
可以设置日志输出至不同的的文件

# 例子
```c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <log_easy.h>


static inline void __attribute__((constructor)) utils_constructor() {
    log_easy_init();
    PINFO("\n");
}

static inline void __attribute__((destructor)) utils_destructor() {
    PINFO("\n");
    log_easy_fini();
}

static inline void _PBASE(int type, const char *s_type, const char *fmt, ...) {
    void *a = log_easy_category_find(s_type);
    if (!IS_ERR_OR_NULL(a)) {
        va_list ap;
        va_start(ap, fmt);
        switch (type) {
            case 1:
                log_easy_category_vprintf(a, fmt, ap);
                break;
            case 2:
                log_easy_category_raw_vprintf(a, fmt, ap);
                break;
        }
        va_end(ap);
    }
}

static inline void _VPBASE(int type, const char *s_type, const char *fmt, va_list ap) {
    void *a = log_easy_category_find(s_type);
    if (!IS_ERR_OR_NULL(a)) {
        switch (type) {
            case 1:
                log_easy_category_vprintf(a, fmt, ap);
                break;
            case 2:
                log_easy_category_raw_vprintf(a, fmt, ap);
                break;
        }
    }
}

void _PDATA_BASE(int type, const char *info, const char *file, const char *func, int line, const char *fmt, ...) {

    va_list args;
    _PBASE(1, info, "[%-8s %s %s %d]", info, func, file, line);
    va_start(args, fmt);
    _VPBASE(2, info, fmt, args);
    va_end(args);

}

void _PDATA_ARRAY(int type, const char *file, const char *func, int line, const char *name, void *v, size_t s) {
    unsigned char *d = v;
    size_t i = 0;
    _PBASE(1, "ARRAY", "[%-8s %s %s %d]\n%s={", "ARRAY", file, func, line, name);
    for (i = 0; i < s; i++) {
        if (i % 16 == 0)
            _PBASE(2, "ARRAY", "\n\t");
        _PBASE(2, "ARRAY", "0X%02x,", d[i]);
    }
    _PBASE(2, "ARRAY", "\n}\n");
}


#define PERR(fmt, args...)          \
    do{                             \
        _PDATA_BASE(300,"ERROR",__FUNCTION__,__FILE__,__LINE__,fmt,##args); \
    }while(0)

#define PINFO(fmt, args...)         \
    do{                             \
        _PDATA_BASE(600,"INFO",__FUNCTION__,__FILE__,__LINE__,fmt,##args); \
    }while(0)

#define PDBUG(fmt, args...)         \
    do{                             \
        _PDATA_BASE(700,"DEBUG",__FUNCTION__,__FILE__,__LINE__,fmt,##args); \
    }while(0)


#define PDATA_ARRAY(val, size)  \
    _PDATA_ARRAY(700,__FILE__,__FUNCTION__,__LINE__,#val,val,size);


int main(int argc ,char *argv[])
{
	uint8_t  data[26]={0};
        PINFO("TEST\n");
        PDBUG("TEST\n");
        PERR("TEST\n");
        PDATA_ARRAY(data,sizeof(data));
	return 0;

}

```

# 配置文件

1. 可将配置文件放置在`$LOG_EASY_PATH/log_easy.json`、`$HOME/.log_easy.json`、`./log_easy.josn`
2. `stream` 节点中的`name`为自定义名称
2. `stream` 节点中的`path`为存储日志的目录，当`path`为`stderr`、`stdout`时直接打印至终端，当`path`为普通文件时会将日志存储至文件
3. `appender` 节点中的`name`暂时支持`stream`

```json
{
    "category":[
      {
        "name": "INFO",
        "stream": [
          {
            "path": "stderr",
            "appender": {
              "name": "stream"
            }
          },
          {
            "path": "a.log",
            "appender": {
              "name": "stream"
            }
          }
        ]
      },
      {
        "name": "DEBUG",
        "stream": [
          {
            "path": "stderr",
            "appender": {
              "name": "stream"
            }
          },
          {
            "path": "a.log",
            "appender": {
              "name": "stream"
            }
          }
        ]
      },
      {
        "name": "ERROR",
        "stream": [
          {
            "path": "stderr",
            "appender": {
              "name": "stream"
            }
          },
          {
            "path": "a.log",
            "appender": {
              "name": "stream"
            }
          }
        ]
      },
      {
        "name": "ARRAY",
        "stream": [
          {
            "path": "stderr",
            "appender": {
              "name": "stream"
            }
          },
          {
            "path": "a.log",
            "appender": {
              "name": "stream"
            }
          }
        ]
      }
    ]
}
```
