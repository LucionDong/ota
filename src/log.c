#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "log.h"

logger_t *global_logger = NULL;

const char *extract_filename(const char *path) {
    const char *filename = strrchr(path, '/');
    if (filename) {
        return filename + 1;  // 移动指针到 '/' 后面
    }
    return path;
}

logger_t *logger_init(log_level_t level, const char *file_path, log_print call_func) {
    logger_t *global_logger = (logger_t *) malloc(sizeof(logger_t));
    if (!global_logger) {
        LOG_ERROR("Failed to allocate memory for global_logger");
        return NULL;
    }
    global_logger->level = level;
    if (file_path) {
        global_logger->log_file = fopen(file_path, "a");
        if (!global_logger->log_file) {
            LOG_ERROR("Failed to open log file");
            free(global_logger);
            return NULL;
        }
    } else {
        global_logger->log_file = NULL;
    }

    if (call_func == NULL) {
        global_logger->func = call_func;
    } else {
        global_logger->func = NULL;
    }

    return global_logger;
}

void logger_cleanup(logger_t *global_logger) {
    if (global_logger->log_file) {
        fclose(global_logger->log_file);
    }
    free(global_logger);
}

void log_message(logger_t *logger, log_level_t level, const char *file, int line, const char *format, ...) {
    if (!logger || level < logger->level) {
        return;
    }

    const char *level_strings[] = {"DEBUG", "INFO", "WARN", "ERROR", "FATAL"};
    time_t now = time(NULL);
    char time_str[20];
    struct tm *tm_info = localtime(&now);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);

    va_list args;
    va_start(args, format);

    const char *filename = extract_filename(file);

    if (logger->log_file) {
        fprintf(logger->log_file, "[%s] [%s] [%s:%d] ", time_str, level_strings[level], filename, line);
        vfprintf(logger->log_file, format, args);
        fprintf(logger->log_file, "\n");
        fflush(logger->log_file);
    } else {
        fprintf(stderr, "[%s] [%s] [%s:%d] ", time_str, level_strings[level], filename, line);
        vfprintf(stderr, format, args);
        fprintf(stderr, "\n");
    }

    if (logger->func) {
        char log_msg[1024];
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&now));

        // 使用可变参数处理format和args
        va_start(args, format);
        vsnprintf(log_msg, sizeof(log_msg), format, args);
        va_end(args);
    }

    va_end(args);
}
