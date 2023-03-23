
#ifndef _HMI_H_
#define _HMI_H_

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

// #include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "rtos_config.h"
#include "store.h"
#include "sensors.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "driver/gpio.h"
#include "esp_log.h"

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
// Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define HMI_SWITCH_A 12
#define HMI_SWITCH_B 14
#define HMI_INDICATION_LED 13

#define OLED_SCREEN_COUNT 4

#define OLED_ADDR   0x3C
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define HMI_LOG_TAG "HMI"


static char hmi_oled_screens[OLED_SCREEN_COUNT][25]= {"pulse_spo2","temperature","ecg","fall_alert"};



//---------------------------------------------------INTERRUPT TASK CONFIG-----------------------------------------------------------

/**
 * @brief Interrupt Task Priority
 * 
 */
#define INTERRUPT_TASK_PRIORITY 4

/**
 * @brief Interrupt Task stack size
 * 
 */
#define INTERRUPT_TASK_STACK_SIZE 5120

/**
 * @brief Interrupt Task Handler
 * 
 */
extern TaskHandle_t task_interrupt_handle;


extern xQueueHandle interruptQueue;

/**
 * @brief Initialize interrupt gpio
 * 
 */
void interrupt_gpio_init();

/**
 * @brief ISR handler
 * 
 * @param args 
 */
void IRAM_ATTR interrupt_gpio_isr_handler(void *args);

/**
 * @brief Install ISR
 * 
 */
void interrupt_install();

// Interrupt Task for Switch A

/**
 * @brief Start Interrupt task
 * 
 */
void start_interrupt_task();

void interrupt_task(void *Params);

// ---------------------------------------------------------------------------------OLED -------------------------------------------------------------------------------



/**
 * @brief OLED HMI Task Priority
 * 
 */
#define OLED_HMI_TASK_PRIORITY 3

/**
 * @brief OLED HMI Task stack size
 * 
 */
#define OLED_HMI_TASK_STACK_SIZE 10240

/**
 * @brief OLED HMI Task Handler
 * 
 */
extern TaskHandle_t task_oled_handle;


void change_screen_inc_logic();

void start_oled_task();

void oled_task(void*Params);



// ---------------------------------------------------------------------------------LED Indication-------------------------------------------------------------------------------



/**
 * @brief LED INDICATION Task Priority
 * 
 */
#define LED_INDICATION_TASK_PRIORITY 7

/**
 * @brief LED INDICATION Task stack size
 * 
 */
#define LED_INDICATION_TASK_STACK_SIZE 2048

/**
 * @brief LED INDICATION Task Handler
 * 
 */
extern TaskHandle_t task_led_indication_handle;

void start_led_indication_task();

void led_indication_task(void*Params);

#endif