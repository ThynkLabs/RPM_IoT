#ifndef _SENSORS_H_
#define _SENSORS_H_

#include "connection.h"
#include "store.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include "DHT.h"
#include "esp_log.h"
#include "cJSON.h"

#include <Wire.h>

#include "MAX30100_PulseOximeter.h"
 
#define SENSOR_TAG "Sensor"
 
#define REPORTING_PERIOD_MS 1000

#define ADC_VREF_mV    3300.0 // in millivolt
#define ADC_RESOLUTION 4096.0
#define PIN_LM35       36 // ESP32 pin GIOP36 (ADC0) connected to LM35
#define DHTTYPE DHT11 



extern SemaphoreHandle_t i2c_bus_mutex;

// --------------------------------------------------------------------------------- Temperature and humidity -------------------------------------------------------------------------------

/**
 * @brief Temperature Task Priority
 * 
 */
#define TEMP_TASK_PRIORITY 4

/**
 * @brief Temperature Task stack size
 * 
 */
#define TEMP_TASK_STACK_SIZE 2048

/**
 * @brief Temperature Task Handler
 * 
 */
extern TaskHandle_t task_temp_handle;



void start_temp_sensor_task();

void temp_sensor_task(void*Params);


// --------------------------------------------------------------------------------- Pulse Sensor Task -------------------------------------------------------------------------------

/**
 * @brief Pulse Sensor Task Priority
 * 
 */
#define PULSE_TASK_PRIORITY 10

/**
 * @brief Pulse Sensor Task stack size
 * 
 */
#define PULSE_TASK_STACK_SIZE 2048

/**
 * @brief Pulse Sensor Task Handler
 * 
 */
extern TaskHandle_t task_pulse_sensor_handle;

extern PulseOximeter pox;

void pulse_sensor_data_streamer(uint8_t spo2, float pulse);

void start_pulse_sensor_task();

void pulse_sensor_task(void*Params);

// --------------------------------------------------------------------------------- Fall Detection Task -------------------------------------------------------------------------------

/**
 * @brief Fall Detection Task Priority
 * 
 */
#define FALL_DETECTION_TASK_PRIORITY 8

/**
 * @brief Fall Detection Task stack size
 * 
 */
#define FALL_DETECTION_TASK_STACK_SIZE 4096

/**
 * @brief Fall Detection Task Handler
 * 
 */
extern TaskHandle_t task_fall_detection_handle;

void mpu_read();

void start_fall_detection_task();

void fall_detection_task(void*Params);


// --------------------------------------------------------------------------------- ECG Task -------------------------------------------------------------------------------

/**
 * @brief ECG Sensor Task Priority
 * 
 */
#define ECG_SENSOR_TASK_PRIORITY 6

/**
 * @brief ECG Sensor Task stack size
 * 
 */
#define ECG_SENSOR_TASK_STACK_SIZE 4096

/**
 * @brief ECG Sensor Task Handler
 * 
 */
extern TaskHandle_t task_ecg_sensor_handle;
#define ECG_SENSOR_PIN 32

void start_ecg_sensor_task();

void ecg_sensor_task(void*Params);

//-----------------------------------------------Reporter Task----------------------------------------------

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
void sensor_data_generator_streamer(float temperature, uint8_t humidity, uint8_t spo2,float pulse);

void reporter(void*param);
void start_reporter_task();

#endif