/*
 * version_hash.h
 * Copyright (C) 2024 dongbin <dongbin0625@163.com>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef __VERSION_HASH_H__
#define __VERSION_HASH_H__
#include "mqtt_async_recv_send.h"
#include "ut_include/uthash.h"

int find_hash_value(hash_element_t *hash, const char *service_name, char **version);
int insert_hash_element(hash_element_t *hash, const char *service_name, char *version);
int destroy_hash(hash_element_t **hash);
int update_hash_value(hash_element_t *hash, const char *service_name, char *version);

#endif /* !__VERSION_HASH_H__ */
