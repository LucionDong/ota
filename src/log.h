#ifndef __LOG_H__
#define __LOG_H__

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef enum { LOG_LEVEL_DEBUG, LOG_LEVEL_INFO, LOG_LEVEL_WARN, LOG_LEVEL_ERROR, LOG_LEVEL_FATAL } log_level_t;

typedef void (*log_print)(const char *, const char *, const char *, int, const char *);

typedef struct {
    log_level_t level;
    FILE *log_file;
    log_print func;
    // 可以再写一个回调函数，由调用者实现，log_message中如果该回调存在，那么调用该回调进行

} logger_t;

extern logger_t *global_logger;

const char *extract_filename(const char *path);

logger_t *logger_init(log_level_t level, const char *file_path, log_print call_func);

void logger_cleanup(logger_t *global_logger);

void log_message(logger_t *logger, log_level_t level, const char *file, int line, const char *format, ...);

#define LOG_DEBUG(format, ...) log_message(global_logger, LOG_LEVEL_DEBUG, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define LOG_INFO(format, ...) log_message(global_logger, LOG_LEVEL_INFO, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define LOG_WARN(format, ...) log_message(global_logger, LOG_LEVEL_WARN, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) log_message(global_logger, LOG_LEVEL_ERROR, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define LOG_FATAL(format, ...) log_message(global_logger, LOG_LEVEL_FATAL, __FILE__, __LINE__, format, ##__VA_ARGS__)

#endif