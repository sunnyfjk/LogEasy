# LogEasy
可以设置日志输出至不同的的文件

# 例子
1. 只能保证使用`LogEasy`库函数的支持并发写入。
2. 列子中使用时,对库进行了简单封装，不能保证日志并发写入。
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
    void *a = log_easy_find("system");
    if (!IS_ERR_OR_NULL(a)) {
        va_list ap;
        va_start(ap, fmt);
        switch (type) {
            case 1:
                log_easy_vprintf(a,s_type, fmt, ap);
                break;
            case 2:
                log_easy_raw_vprintf(a,s_type, fmt, ap);
                break;
        }
        va_end(ap);
    }
}

static inline void _VPBASE(int type, const char *s_type, const char *fmt, va_list ap) {
    void *a = log_easy_find("system");
    if (!IS_ERR_OR_NULL(a)) {
        switch (type) {
            case 1:
                log_easy_vprintf(a,s_type, fmt, ap);
                break;
            case 2:
                log_easy_raw_vprintf(a,s_type, fmt, ap);
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

1. 可将配置文件放置在`$LOG_EASY_PATH/log_easy.json`、`$HOME/.log_easy.json`、`./log_easy.josn`。
2. `category` 节点中的`type`为自定义名称,`appender`指定输出的文件。
2. `appender` 节点中的`path`日志输出文件，当`path`为`stderr`、`stdout`时直接打印至终端，当`path`为普通文件时会将日志存储至文件。
3. `appender` 节点中的`name`为自定义类型，应与`category`中`appender`节点保持一致。

```json
{
  "category": [
    {
      "type": "INFO",
      "appender": "stderr"
    },
    {
      "type": "INFO",
      "appender": "file"
    },
    {
      "type": "DEBUG",
      "appender": "stderr"
    },
    {
      "type": "DEBUG",
      "appender": "file"
    },
    {
      "type": "ERROR",
      "appender": "stderr"
    },
    {
      "type": "ERROR",
      "appender": "file"
    }
  ],
  "appender": [
    {
      "type": "stderr",
      "path": "stderr"
    },
    {
      "type": "stdout",
      "path": "stdout"
    },
    {
      "type": "file",
      "path": "a.log"
    }
  ]
}
```
