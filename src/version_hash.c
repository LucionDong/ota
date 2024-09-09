/*
 *
 * Copyright (C) 2024-08-08 17:32 dongbin <dongbin0625@163.com>
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "mqtt_async_recv_send.h"
#include "ut_include/uthash.h"

int insert_hash_element(hash_element_t *hash, const char *service_name, char *version) {
    if (!hash) {
        LOG_ERROR("hash is NULL");
        return -1;
    }

    hash_element_t *hash_element = calloc(1, sizeof(hash_element_t));
    hash_element->service_name = strdup(service_name);
    hash_element->service_version = strdup(version);
    HASH_ADD_KEYPTR(hh, hash, hash_element->service_name, strlen(hash_element->service_name), hash_element);

    return 0;
}

int find_hash_value(hash_element_t *hash, const char *service_name, char **version) {
    if (!hash) {
        LOG_ERROR("hash is NULL");
        return -1;
    }

    hash_element_t *hash_element = NULL;
    HASH_FIND_STR(hash, service_name, hash_element);
    if (hash_element) {
        *version = strdup(hash_element->service_version);
        return 0;
    }

    LOG_WARN("do not find hash_element");
    return -1;
}

int update_hash_value(hash_element_t *hash, const char *service_name, char *version) {
    if (!hash) {
        LOG_ERROR("hash is NULL");
        return -1;
    }

    hash_element_t *hash_element = NULL;
    HASH_FIND_STR(hash, service_name, hash_element);
    if (hash_element && hash_element->service_version) {
        free(hash_element->service_version);
        hash_element->service_version = strdup(version);
        return 0;
    } else {
        insert_hash_element(hash, service_name, version);
        return 0;
    }
}

int destroy_hash(hash_element_t **hash) {
    if (!hash) {
        LOG_ERROR("hash is NULL");
        return -1;
    }

    hash_element_t *temp_hash_element = NULL, *free_hash_helper = NULL;
    HASH_ITER(hh, *hash, temp_hash_element, free_hash_helper) {
        free((void *) temp_hash_element->service_name);
        free((void *) temp_hash_element->service_version);
        HASH_DEL(*hash, temp_hash_element);
        free(temp_hash_element);
    }

    if (*hash) {
        free(*hash);
        *hash = NULL;
    }

    return 0;
}
