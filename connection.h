

#ifndef _CONNECTION_H_
#define _CONNECTION_H_
#include <cJSON.h>
#include <Arduino_JSON.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/timers.h"

#include "store.h"


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
#define STREAMER_TASK_PRIORITY 5

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
void sensor_data_generator_streamer(float temperature, uint8_t humidity, uint8_t spo2,uint8_t pulse);
void send_data_to_streamer_queue(char*stream);
void streamer(void* param);
void start_streamer_task();


void test_mqtt(void* param);
void start_test_mqtt();


/**
 * @brief Reporter Task Priority
 * 
 */
#define REPORTER_TASK_PRIORITY 4

/**
 * @brief Reporter Task stack size
 * 
 */
#define REPORTER_TASK_STACK_SIZE 6144

/**
 * @brief Reporter Task Handler
 * 
 */
extern TaskHandle_t task_reporter_handle;

void reporter(void*param);
void start_reporter_task();

extern QueueHandle_t streamer_queue;
#endif
