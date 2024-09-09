/*
 * mqtt_async_recv_send.h
 * Copyright (C) 2024 dongbin <dongbin0625@163.com>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef __MQTT_ASYNC_RECV_SEND_H__
#define __MQTT_ASYNC_RECV_SEND_H__
#include <MQTTAsync.h>
#include <MQTTClient.h>
#include <jansson.h>

#include "ut_include/uthash.h"

// #define RECV_TOPIC "/sys/gatewaybrokercore/emlocalmqttt/thing/event/property/post"
#define RECV_TOPIC "wan/+/+/thing/ota/#"
#define SEND_TOPIC "/sys/esvcpm/mqtt/thing/event/property/post"
#define ADDRESS "tcp://127.0.0.1:1883"
// #define ADDRESS "mqtt://broker.emqx.io:1883"
#define QOS 0

static int finished = 0;
static int subscribed = 0;
static int disc_finished = 0;

typedef enum command_type {
    SERVICE_INFORM_GET = 1,
    SERVICE_INFORM_POST,
    UPGRADE_PUSH,
    UPGRADE_PROGRESS,
} command_type_e;

typedef struct mqtt_res {
    MQTTAsync client;
    char *client_id;
    json_t *root;
    command_type_e command_type;
} mqtt_res_t;

typedef struct hash_element {
    const char *service_name;
    char *service_version;
    UT_hash_handle hh;
} hash_element_t;

void onConnect(void *context, MQTTAsync_successData *response);
void connlost(void *context, char *cause);
int msgarrvd(void *context, char *topicName, int topicLen, MQTTAsync_message *message);
void onDisconnectFailure(void *context, MQTTAsync_failureData *response);
void onDisconnect(void *context, MQTTAsync_successData *response);
void onSubscribe(void *context, MQTTAsync_successData *response);
void onSubscribeFailure(void *context, MQTTAsync_failureData *response);
void onConnectFailure(void *context, MQTTAsync_failureData *response);

#endif /* !__MQTT_ASYNC_RECV_SEND_H__ */
