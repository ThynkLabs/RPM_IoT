#include "hmi.h"
#include "connection.h"
#include "sensors.h"

bool isWireInitialized = false;

void setup() {
  // put your setup code here, to run once:
  
  i2c_bus_mutex = xSemaphoreCreateMutex();
  start_oled_task(); 
  // start_oled_ecg_task();
   start_led_indication_task();
  start_interrupt_task();
  start_wifi_task();
  start_mqtt_task();
start_streamer_task();

 start_reporter_task(); 
  start_fall_detection_task();
  start_pulse_sensor_task();
  start_temp_sensor_task();
  start_ecg_sensor_task();
}

void loop() {
  // put your main code here, to run repeatedly:
  // printf("Setup Done\n");
  // delay(1000);
}
