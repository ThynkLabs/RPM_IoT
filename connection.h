

#ifndef _CONNECTION_H_
#define _CONNECTION_H_
#include <cJSON.h>
// #include <Arduino_JSON.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/timers.h"

#include "store.h"

#define STREAM_TOPIC "bncoe/rpm/stream"
#define FALL_TOPIC "bncoe/rpm/fall"
#define ECG_TOPIC "bncoe/rpm/ecg"
#define PULSE_TOPIC "bncoe/rpm/pulse"


typedef struct mqtt_publish_queue_struct_t{
    char*topic;
    char*message;
}mqtt_publish_queue_struct;

/**
 * @brief wifi Task Priority
 * 
 */
#define WIFI_TASK_PRIORITY 1

/**
 * @brief wifi Task stack size
 * 
 */
#define WIFI_TASK_STACK_SIZE 4096

/**
 * @brief wifi Task Handler
 * 
 */
extern TaskHandle_t task_wifi_handle;

void connectToWiFi(void* parameter);
void start_wifi_task();


/**
 * @brief MQTT Task Priority
 * 
 */
#define MQTT_TASK_PRIORITY 1

/**
 * @brief MQTT Task stack size
 * 
 */
#define MQTT_TASK_STACK_SIZE 4096

/**
 * @brief MQTT Task Handler
 * 
 */
extern TaskHandle_t task_mqtt_handle;

void connectToMQTT(void* parameter);
void start_mqtt_task();


/**
 * @brief Streamer Task Priority
 * 
 */
#define STREAMER_TASK_PRIORITY 10

/**
 * @brief Streamer Task stack size
 * 
 */
#define STREAMER_TASK_STACK_SIZE 6144

/**
 * @brief Streamer Task Handler
 * 
 */
extern TaskHandle_t task_streamer_handle;
void send_data_to_streamer_queue(char*topic,char*message);
void streamer(void* param);
void start_streamer_task();


void test_mqtt(void* param);
void start_test_mqtt();

void publish_message(char*topic,char*message);

extern QueueHandle_t streamer_queue;
#endif
