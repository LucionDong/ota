/*******************************************************************************
 * Copyright (c) 2012, 2022 IBM Corp., Ian Craggs
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v2.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *   https://www.eclipse.org/legal/epl-2.0/
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial contribution
 *******************************************************************************/

#include <MQTTAsync.h>
#include <MQTTClient.h>
#include <MQTTClientPersistence.h>
#include <MQTTSubscribeOpts.h>
#include <jansson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "culr_install.h"
#include "log.h"
#include "mqtt_async_recv_send.h"
#include "parse_json.h"
#include "read_file.h"
#include "version_hash.h"

#define TIMEOUT 10000L

void onConnect(void *context, MQTTAsync_successData *response);
void onConnectFailure(void *context, MQTTAsync_failureData *response);

int get_time_stamp(time_t *timestamp) {
    struct tm timeinfo;
    *timestamp = time(NULL);

    localtime_r(timestamp, &timeinfo);
    return 0;
}

int composition_timestamp(const time_t *timestamp, char *client_id) {
    sprintf(client_id, "%s%ld", client_id, *timestamp);
    return 0;
}

void onReconnected(void *context, char *cause) {
    // MQTTAsync client = (MQTTAsync) context;
    mqtt_res_t *mqtt_res = (mqtt_res_t *) context;
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    int rc;

    opts.onSuccess = onSubscribe;
    opts.onFailure = onSubscribeFailure;
    opts.context = mqtt_res->client;
    if ((rc = MQTTAsync_subscribe(mqtt_res->client, RECV_TOPIC, QOS, &opts)) != MQTTASYNC_SUCCESS) {
        finished = 1;
    }
}

void onReconnected5(void *context, char *cause) {
    mqtt_res_t *mqtt_res = (mqtt_res_t *) context;

    MQTTSubscribe_options subscribe_options = MQTTSubscribe_options_initializer;
    MQTTAsync_callOptions copts = MQTTAsync_callOptions_initializer;

    subscribe_options.noLocal = 1;
    copts.subscribeOptions = subscribe_options;
    // copts.onSuccess5 = error_reporting_onSubscribeSuccess5;
    copts.context = context;
    int rc;
    if ((rc = MQTTAsync_subscribe(mqtt_res->client, RECV_TOPIC, QOS, &copts)) != MQTTASYNC_SUCCESS) {
        perror("MQTTAsync_subscribe");
    }
}

void connlost(void *context, char *cause) {
    MQTTAsync client = (MQTTAsync) context;
    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
    int rc;

    printf("\nConnection lost\n");
    if (cause)
        printf("     cause: %s\n", cause);

    printf("Reconnecting\n");
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    conn_opts.onSuccess = onConnect;
    conn_opts.onFailure = onConnectFailure;
    if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS) {
        printf("Failed to start connect, return code %d\n", rc);
        finished = 1;
    }
}

void replace_push_with_progress(char *str) {
    const char *target = "push";
    const char *replacement = "progress";
    char buffer[256];  // 临时缓冲区，确保它足够大以容纳替换后的字符串
    char *pos;

    // 查找子字符串 "push" 的位置
    pos = strstr(str, target);
    if (pos == NULL) {
        // 没有找到 "push"，直接返回
        return;
    }

    // 复制替换字符串之前的内容到缓冲区
    strncpy(buffer, str, pos - str);
    buffer[pos - str] = '\0';

    // 在缓冲区中拼接上 "progress"
    strcat(buffer, replacement);

    // 在缓冲区中拼接上 "push" 之后的内容
    strcat(buffer, pos + strlen(target));

    // 将结果复制回原始字符串
    strcpy(str, buffer);
}

int command_handle(json_t *root, mqtt_res_t *mqtt_res, const char *topic) {
    const char *method = json_string_value(json_object_get(root, "method"));
    if (!method) {
        LOG_WARN("method is NULL");
        return -1;
    }

    int command_type = command_string_to_enum(method);
    if (command_type == -1) {
        LOG_WARN("method is error");
        return -1;
    }
    // LOG_INFO("root: %s\n", json_dumps(root, 1));
    hash_element_t *hash = NULL;
    const char *url = NULL, *tranid = json_string_value(json_object_get(root, "transId")), *md5_string = NULL;
    char send_topic[1024] = {0}, *send_to_server_json = NULL;
    strcpy(send_topic, topic);

    json_t *reply_json = json_object(), *params = json_object_get(root, "params");
    json_object_set_new(reply_json, "transId", json_string(tranid));

    switch (command_type) {
        case SERVICE_INFORM_GET:
            //解析services中的内容
            //上传版本号
            sprintf(send_topic, "%s/Reply", send_topic);
            parser_service_inform_json_and_piece(root, hash);
            get_hash_file_path_and_read_version(hash);
            composition_reply_version_json(reply_json, hash);
            MQTTAsync_message pubmsg = MQTTAsync_message_initializer;

            send_to_server_json = json_dumps(reply_json, 0);
            pubmsg.payload = send_to_server_json;
            pubmsg.payloadlen = strlen(send_to_server_json);
            if (MQTTAsync_sendMessage(mqtt_res->client, send_topic, &pubmsg, NULL) != MQTTASYNC_SUCCESS) {
                LOG_WARN("Failed to start sendMessage");
            }
            free(send_to_server_json);

            break;
        case SERVICE_INFORM_POST:
            break;
        case UPGRADE_PUSH:
            //下载url中的内容
            //对下载的文件进行校验
            //按顺序调用脚本
            //调用脚本过程中进行进度上报
            replace_push_with_progress(send_topic);
            url = json_string_value(json_object_get(params, "url"));
            if (!url) {
                LOG_ERROR("json_string_value NULL");
                return -1;
            }
            LOG_INFO("url: %s\n", url);
            md5_string = json_string_value(json_object_get(params, "signValue"));
            if (wget_install(url, md5_string) == -1) {
                composition_reply_upgrade_progress_json(reply_json, "-3");
                MQTTAsync_message pubmsg = MQTTAsync_message_initializer;

                send_to_server_json = json_dumps(reply_json, 0);
                pubmsg.payload = send_to_server_json;
                pubmsg.payloadlen = strlen(send_to_server_json);
                if (MQTTAsync_sendMessage(mqtt_res->client, send_topic, &pubmsg, NULL) != MQTTASYNC_SUCCESS) {
                    LOG_WARN("Failed to start sendMessage");
                }
            } else {
                unpack_tar(DOWNLOAD_PATH);
                executive_control_sh(reply_json, mqtt_res, send_topic);
            }
            break;
        case UPGRADE_PROGRESS:
            //上报进度
            break;
    }

    json_decref(reply_json);

    return 0;
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTAsync_message *message) {
    mqtt_res_t *mqtt_res = (mqtt_res_t *) context;
    LOG_INFO("Message arrived\n");
    LOG_INFO("     topic: %.*s\n", topicLen, topicName);
    LOG_INFO("   message: %.*s\n", message->payloadlen, (char *) message->payload);

    char *mqtt_message = malloc(sizeof(char) * (message->payloadlen + 1));
    strncpy(mqtt_message, message->payload, message->payloadlen);
    LOG_INFO("mqtt_message: %s\n", mqtt_message);

    json_error_t error;
    json_t *root = json_loads(mqtt_message, 0, &error);
    if (!root) {
        LOG_WARN("json_loads is NULL");
        return -1;
    }
    const char *root_str = json_dumps(root, JSON_INDENT(2));
    LOG_INFO("root: %s", root_str);
    free((void *) root_str);

    command_handle(root, mqtt_res, topicName);

    if (mqtt_message) {
        free(mqtt_message);
        mqtt_message = NULL;
    }

    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);
    json_decref(root);
    return 1;
}

void onDisconnectFailure(void *context, MQTTAsync_failureData *response) {
    printf("Disconnect failed, rc %d\n", response->code);
    disc_finished = 1;
}

void onDisconnect(void *context, MQTTAsync_successData *response) {
    printf("Successful disconnection\n");
    disc_finished = 1;
}

void onSubscribe(void *context, MQTTAsync_successData *response) {
    printf("Subscribe succeeded\n");
    subscribed = 1;
}

void onSubscribeFailure(void *context, MQTTAsync_failureData *response) {
    printf("Subscribe failed, rc %d\n", response->code);
    finished = 1;
}

void onConnectFailure(void *context, MQTTAsync_failureData *response) {
    printf("Connect failed, rc %d\n", response->code);
    finished = 1;
}

void onConnect(void *context, MQTTAsync_successData *response) {
    // mqtt_res_t *mqtt_res = (mqtt_res_t *) context;
    MQTTAsync client = (MQTTAsync) context;
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    int rc;

    printf("Successful connection\n");

    opts.onSuccess = onSubscribe;
    opts.onFailure = onSubscribeFailure;
    opts.context = client;
    if ((rc = MQTTAsync_subscribe(client, RECV_TOPIC, QOS, &opts)) != MQTTASYNC_SUCCESS) {
        printf("Failed to start subscribe, return code %d\n", rc);
        finished = 1;
    }
}

int destroy_all(mqtt_res_t **mqtt_res) {
    free((*mqtt_res)->client_id);
    (*mqtt_res)->client_id = NULL;

    free(*(mqtt_res));
    *(mqtt_res) = NULL;

    return 0;
}
int init_topic_to_composition_stamp(char *client_id) {
    time_t timestamp;
    get_time_stamp(&timestamp);
    composition_timestamp(&timestamp, client_id);

    return 0;
}

int init_mqtt_res(mqtt_res_t *mqtt_res) {
    // mqtt_res->recv_share = malloc(sizeof(mqtt_share_t));
    // mqtt_res->send_share = malloc(sizeof(mqtt_share_t));
    LOG_INFO("++++++++++++++++++++++++");
    mqtt_res->client_id = (char *) malloc(sizeof(char) * 20);
    strcpy(mqtt_res->client_id, "Client");
    init_topic_to_composition_stamp(mqtt_res->client_id);

    return 0;
}

int main() {
    printf("1111111111111111111111111\n");
    global_logger = logger_init(LOG_LEVEL_DEBUG, NULL, NULL);

    mqtt_res_t *mqtt_res = calloc(sizeof(mqtt_res_t), 1);
    init_mqtt_res(mqtt_res);
    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer5;
    MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;
    MQTTAsync_createOptions createOpts = MQTTAsync_createOptions_initializer;
    createOpts.MQTTVersion = MQTTVERSION_5;

    int rc;
    int ch;

    if ((rc = MQTTAsync_createWithOptions(&mqtt_res->client, ADDRESS, mqtt_res->client_id, MQTTCLIENT_PERSISTENCE_NONE,
                                          NULL, &createOpts)) != MQTTASYNC_SUCCESS) {
        perror("MQTTAsync_createWithOptions");
    }

    //  if ((rc = MQTTAsync_create(&mqtt_res->client, ADDRESS, mqtt_res->client_id, MQTTCLIENT_PERSISTENCE_NONE,
    //                        NULL)) != MQTTASYNC_SUCCESS) {
    // printf("Failed to create client, return code %d\n", rc);
    // }

    conn_opts.keepAliveInterval = 20;
    conn_opts.cleanstart = 1;
    // conn_opts.cleansession = 1;
    conn_opts.automaticReconnect = 1;
    conn_opts.minRetryInterval = 2;
    conn_opts.maxRetryInterval = 5;
    // conn_opts.onSuccess = onConnect;
    // conn_opts.onFailure = onConnectFailure;
    conn_opts.context = mqtt_res->client;

    LOG_INFO("-----------");
    if ((rc = MQTTAsync_setCallbacks(mqtt_res->client, mqtt_res, connlost, msgarrvd, NULL)) != MQTTASYNC_SUCCESS) {
        printf("Failed to set callbacks, return code %d\n", rc);
    }

    if ((rc = MQTTAsync_setConnected(mqtt_res->client, mqtt_res, onReconnected5)) != MQTTASYNC_SUCCESS) {
        printf("MQTTAsync_setConnected error\n");
    }

    if ((rc = MQTTAsync_connect(mqtt_res->client, &conn_opts)) != MQTTASYNC_SUCCESS) {
        printf("Failed to start connect, return code %d\n", rc);
    }

    do {
        ch = getchar();
    } while (ch != 'Q' && ch != 'q');

    disc_opts.onSuccess = onDisconnect;
    disc_opts.onFailure = onDisconnectFailure;
    if ((rc = MQTTAsync_disconnect(mqtt_res->client, &disc_opts)) != MQTTASYNC_SUCCESS) {
        printf("Failed to start disconnect, return code %d\n", rc);
    }

    MQTTAsync_destroy(&mqtt_res->client);
    destroy_all(&mqtt_res);
    return rc;
}
