/*
 *
 * Copyright (C) 2024-08-08 16:11 dongbin <dongbin0625@163.com>
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "mqtt_async_recv_send.h"
#include "read_file.h"
#include "version_hash.h"

int read_version_file(char *version, const char *file_path) {
    char tmp_file_path[128] = {0};
    sprintf(tmp_file_path, "%s%s", tmp_file_path, file_path);

    FILE *fd = fopen(tmp_file_path, "r");
    if (!fd) {
        LOG_ERROR("fopen is error");
        return -1;
    }

    if (fgets(version, 1024, fd) == NULL) {
        LOG_WARN("version file is NULL");
        return -1;
    }

    fclose(fd);

    return 0;
}

int get_hash_file_path_and_read_version(hash_element_t *hash) {
    if (!hash) {
        LOG_ERROR("hash is NULL");
        return -1;
    }

    hash_element_t *hash_element = NULL, *iter_helper = NULL;
    char *version = NULL;
    HASH_ITER(hh, hash, hash_element, iter_helper) {
        read_version_file(version, hash_element->service_version);
        update_hash_value(hash, hash_element->service_name, version);
        LOG_INFO("version: %s", hash_element->service_version);
    }

    return 0;
}
