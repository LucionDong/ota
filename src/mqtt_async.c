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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "culr_install.h"
#include "mqtt_async_recv_send.h"
#include "parse_json.h"

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
    Mqtt_class_t *mqtt_class = (Mqtt_class_t *) context;
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    int rc;

    opts.onSuccess = onSubscribe;
    opts.onFailure = onSubscribeFailure;
    opts.context = mqtt_class->client;
    if ((rc = MQTTAsync_subscribe(mqtt_class->client, RECV_TOPIC, QOS, &opts)) != MQTTASYNC_SUCCESS) {
        finished = 1;
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

int msgarrvd(void *context, char *topicName, int topicLen, MQTTAsync_message *message) {
    printf("Message arrived\n");
    printf("     topic: %s\n", topicName);
    printf("   message: %.*s\n", message->payloadlen, (char *) message->payload);

    json_error_t error;
    json_t *root = json_loads((char *) message->payload, 1, &error);
    const char *url = json_string_value(json_object_get(root, "url"));
    wget_install(url);

    unpack_tar(DOWNLOAD_PATH);
    executive_control_sh();

    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);
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
    // Mqtt_class_t *mqtt_class = (Mqtt_class_t *) context;
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

int destroy_all(Mqtt_class_t **mqtt_class) {
    free((*mqtt_class)->client_id);
    (*mqtt_class)->client_id = NULL;

    free(*(mqtt_class));
    *(mqtt_class) = NULL;

    return 0;
}
int init_topic_to_composition_stamp(char *client_id) {
    time_t timestamp;
    get_time_stamp(&timestamp);
    composition_timestamp(&timestamp, client_id);

    return 0;
}

int init_mqtt_class(Mqtt_class_t *mqtt_class) {
    // mqtt_class->recv_share = malloc(sizeof(mqtt_share_t));
    // mqtt_class->send_share = malloc(sizeof(mqtt_share_t));
    mqtt_class->client_id = (char *) malloc(sizeof(char) * 20);
    strcpy(mqtt_class->client_id, "Client");
    init_topic_to_composition_stamp(mqtt_class->client_id);

    return 0;
}

int main() {
    Mqtt_class_t *mqtt_class = calloc(sizeof(Mqtt_class_t), 1);
    init_mqtt_class(mqtt_class);
    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
    MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;
    int rc;
    int ch;

    if ((rc = MQTTAsync_create(&mqtt_class->client, ADDRESS, mqtt_class->client_id, MQTTCLIENT_PERSISTENCE_NONE,
                               NULL)) != MQTTASYNC_SUCCESS) {
        printf("Failed to create client, return code %d\n", rc);
    }

    if ((rc = MQTTAsync_setCallbacks(mqtt_class->client, mqtt_class->client, connlost, msgarrvd, NULL)) !=
        MQTTASYNC_SUCCESS) {
        printf("Failed to set callbacks, return code %d\n", rc);
    }

    if ((rc = MQTTAsync_setConnected(mqtt_class->client, mqtt_class, onReconnected)) != MQTTASYNC_SUCCESS) {
        printf("MQTTAsync_setConnected error\n");
    }

    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    conn_opts.automaticReconnect = 1;
    conn_opts.minRetryInterval = 2;
    conn_opts.maxRetryInterval = 5;
    // conn_opts.onSuccess = onConnect;
    // conn_opts.onFailure = onConnectFailure;
    conn_opts.context = mqtt_class->client;
    if ((rc = MQTTAsync_connect(mqtt_class->client, &conn_opts)) != MQTTASYNC_SUCCESS) {
        printf("Failed to start connect, return code %d\n", rc);
        // rc = EXIT_FAILURE;
        // goto destroy_exit;
    }

    do {
        ch = getchar();
    } while (ch != 'Q' && ch != 'q');

    disc_opts.onSuccess = onDisconnect;
    disc_opts.onFailure = onDisconnectFailure;
    if ((rc = MQTTAsync_disconnect(mqtt_class->client, &disc_opts)) != MQTTASYNC_SUCCESS) {
        printf("Failed to start disconnect, return code %d\n", rc);
    }

    MQTTAsync_destroy(&mqtt_class->client);
    destroy_all(&mqtt_class);
    return rc;
}
