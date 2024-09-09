/*
 * read_file.h
 * Copyright (C) 2024 dongbin <dongbin0625@163.com>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef __READ_FILE_H__
#define __READ_FILE_H__

#include "version_hash.h"

#define VERSION_FILE_NAME "version"

int read_version_file(char *version, const char *file_path);
int get_hash_file_path_and_read_version(hash_element_t *hash);
#endif /* !__READ_FILE_H__ */
