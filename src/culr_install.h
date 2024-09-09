/*
 * culr_install.h
 * Copyright (C) 2024 dongbin <dongbin0625@163.com>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef __CULR_INSTALL_H__
#define __CULR_INSTALL_H__

#define OUT_FILE_NAME "ota.tar.gz"
int wget_install(const char *url,const char *recv_md5_string);

#endif /* !__CULR_INSTALL_H__ */
