/*
 *
 * Copyright (C) 2024-05-07 11:05 dongbin <dongbin0625@163.com>
 *
 *
 */
#include <MQTTAsync.h>
#include <MQTTClient.h>
#include <MQTTClientPersistence.h>
#include <MQTTSubscribeOpts.h>
#include <archive.h>
#include <archive_entry.h>
#include <jansson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "log.h"
#include "mqtt_async_recv_send.h"
#include "openssl/sha.h"
#include "parse_json.h"
#include "version_hash.h"

int check_download_file_exit() {
    return access(DOWNLOAD_PATH, F_OK) == 0;
}

int wget_url(mqtt_res_t *mqtt_res) {
    const char *url = json_string_value(json_object_get(mqtt_res->root, "url"));
    if (url == NULL) {
        LOG_ERROR("url error");
        return -1;
    }

    setenv(ENVIRONMENT_VAR, url, 1);
    system("./wget_install.sh");
    if (!check_download_file_exit()) {
        LOG_ERROR("download file fail");
        // LOG_ERROR("download file fail");
        return -1;
    }

    return 0;
}

int unpack_tar(const char *file_path) {
    struct archive *arc = archive_read_new();
    archive_read_support_filter_all(arc);
    archive_read_support_format_all(arc);

    int ret = archive_read_open_filename(arc, file_path, 10240);
    if (ret != ARCHIVE_OK) {
        LOG_ERROR("archive_read_open_filename");
        return -1;
        ;
    }

    struct archive_entry *entry;
    while (archive_read_next_header(arc, &entry) == ARCHIVE_OK) {
        const char *file_name = archive_entry_pathname(entry);
        printf("file_name :%s\n", file_name);

        ret = archive_read_extract(arc, entry, ARCHIVE_EXTRACT_TIME);
        if (ret != ARCHIVE_OK) {
            LOG_ERROR("archive_read_extract");
            return -1;
        }
    }

    archive_read_close(arc);
    archive_read_free(arc);

    return 0;
}

// int unpack_tar(const char *tar_path) {
// printf("tar_path: %s\n", tar_path);
// char unpack_path[30] = "./unpack_tar.sh";
// sprintf(unpack_path, "%s %s", unpack_path, tar_path);
// printf("unpack_path: %s\n", unpack_path);
// system(unpack_path);
//
// return 0;
// }

int check_pack_element() {
    if (access(OTA_PACK_CONTROL, F_OK) != 0 || access(OTA_PACK_DATA, F_OK) != 0 || access(OTA_PACK_DEBIAN, F_OK) != 0) {
        return -1;
    }

    return 0;
}

int check_control_element() {
    if (access(CONTROL_ELEMENT_POSTINST, F_OK) != 0 || access(CONTROL_ELEMENT_PREINST, F_OK) != 0 ||
        access(CONTROL_ELEMENT_PRERM, F_OK) != 0) {
        return -1;
    }

    return 0;
}

int executive_control_sh(json_t *reply_json, mqtt_res_t *mqtt_res, const char *send_topic) {
    if (check_pack_element() != 0) {
        printf("pack element error\n");
        return -1;
    }

    MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
    char *send_to_server_json = NULL;
    unpack_tar(OTA_PACK_DATA);
    unpack_tar(OTA_PACK_CONTROL);
    // unpack_tar(OTA_PACK_DEBIAN);

    if (check_control_element() != 0) {
        printf("check control element error\n");
        return -1;
    }

    system("./preinst.sh");
    composition_reply_upgrade_progress_json(reply_json, "30");
    send_to_server_json = json_dumps(reply_json, 0);
    pubmsg.payload = send_to_server_json;
    pubmsg.payloadlen = strlen(send_to_server_json);
    if (MQTTAsync_sendMessage(mqtt_res->client, send_topic, &pubmsg, NULL) != MQTTASYNC_SUCCESS) {
        LOG_WARN("Failed to start sendMessage");
    }

    system("./postinst.sh");
    composition_reply_upgrade_progress_json(reply_json, "60");
    free(send_to_server_json);
    send_to_server_json = json_dumps(reply_json, 0);
    pubmsg.payload = send_to_server_json;
    pubmsg.payloadlen = strlen(send_to_server_json);
    if (MQTTAsync_sendMessage(mqtt_res->client, send_topic, &pubmsg, NULL) != MQTTASYNC_SUCCESS) {
        LOG_WARN("Failed to start sendMessage");
    }

    system("./prerm.sh");
    composition_reply_upgrade_progress_json(reply_json, "100");
    free(send_to_server_json);
    send_to_server_json = json_dumps(reply_json, 0);
    pubmsg.payload = send_to_server_json;
    pubmsg.payloadlen = strlen(send_to_server_json);
    if (MQTTAsync_sendMessage(mqtt_res->client, send_topic, &pubmsg, NULL) != MQTTASYNC_SUCCESS) {
        LOG_WARN("Failed to start sendMessage");
    }

    free(send_to_server_json);

    return 0;
}

int command_string_to_enum(const char *method) {
    if (!method) {
        LOG_WARN("method is NULL");
        return -1;
    }

    if (!strcmp(method, "thing.ota.device.servie-inform.get")) {
        return SERVICE_INFORM_GET;
    } else if (!strcmp(method, "thing.ota.device.service-inform.post")) {
        return SERVICE_INFORM_POST;
    } else if (!strcmp(method, "thing.ota.device.upgrade.push")) {
        return UPGRADE_PUSH;
    } else if (!strcmp(method, "thing.ota.device.upgrade.progress")) {
        return UPGRADE_PROGRESS;
    }

    LOG_WARN("method is not need command");
    return -1;
}

int parser_service_inform_json_and_piece(json_t *root, hash_element_t *hash) {
    json_t *params = json_object_get(root, "params");
    json_t *services = json_object_get(params, "services");
    json_t *services_array_value = NULL;
    size_t services_array_size;

    json_array_foreach(services, services_array_size, services_array_value) {
        const char *services_element = json_string_value(services_array_value);
        if (!services_element) {
            LOG_WARN("services_element is NULL");
            return -1;
        }

        char file_path[128] = {0};
        sprintf(file_path, "%s%s", SERVICE_FILE_PATH, services_element);
        LOG_INFO("piece str: %s", file_path);

        if (insert_hash_element(hash, services_element, file_path) == -1) {
            LOG_ERROR("insert_hash_element is error");
            return -1;
        }
    }

    return 0;
}

int composition_reply_version_json(json_t *reply_json, hash_element_t *hash) {
    hash_element_t *hash_element = NULL, *hash_iter_helper = NULL;
    json_t *params = json_object(), *services = json_array();

    HASH_ITER(hh, hash, hash_element, hash_iter_helper) {
        json_t *services_element = json_object();
        json_object_set_new(services_element, "service", json_string(hash_element->service_name));
        json_object_set_new(services_element, "version", json_string(hash_element->service_version));
        json_array_append_new(services, services_element);
    }

    json_object_set_new(params, "services", services);
    json_object_set_new(reply_json, "params", params);
    json_object_set_new(reply_json, "id", json_string("123"));
    json_object_set_new(reply_json, "version", json_string("1.0"));
    json_object_set_new(reply_json, "method", json_string("thing.ota.device.service-inform.getReply"));

    return 0;
}

int composition_reply_upgrade_progress_json(json_t *reply_json, const char *step) {
    json_t *params = json_object();

    json_object_set_new(params, "step", json_string(step));
    json_object_set_new(params, "desc", json_string(""));
    json_object_set_new(reply_json, "params", params);
    json_object_set_new(reply_json, "id", json_string("123"));
    json_object_set_new(reply_json, "version", json_string("1.0"));
    json_object_set_new(reply_json, "method", json_string("thing.ota.device.upgrade.progress"));

    return 0;
}
