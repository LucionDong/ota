/*
 * parse_json.h
 * Copyright (C) 2024 dongbin <dongbin0625@163.com>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef __PARSE_JSON_H__
#define __PARSE_JSON_H__

#include <wchar.h>
#define ENVIRONMENT_VAR "URL_VAR"
#define DOWNLOAD_PATH "/root/ota/ota.tar.gz"
#define OTA_PACK_DATA "/root/ota/data.tar.gz"
#define OTA_PACK_CONTROL "/root/ota/control.tar.gz"
#define OTA_PACK_DEBIAN "/root/ota/debian-binary"

#define CONTROL_ELEMENT_PREINST "/root/ota/preinst.sh"
#define CONTROL_ELEMENT_POSTINST "/root/ota/postinst.sh"
#define CONTROL_ELEMENT_PRERM "/root/ota/prerm.sh"

#define SERVICE_FILE_PATH "/usr/local/iot/service/"

#include "mqtt_async_recv_send.h"

int check_download_file_exit();
int wget_url(mqtt_res_t *mqtt_res);
int unpack_tar(const char *tar_path);
int command_string_to_enum(const char *method);
int parser_service_inform_json_and_piece(json_t *root, hash_element_t *hash);
int composition_reply_version_json(json_t *reply_json, hash_element_t *hash);
int composition_reply_upgrade_progress_json(json_t *reply_json, const char *step);
int executive_control_sh(json_t *reply_json, mqtt_res_t *mqtt_res,const char *send_topic);
#endif /* !__PARSE_JSON_H__ */
