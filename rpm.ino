#include "hmi.h"
#include "connection.h"
#include "sensors.h"

bool isWireInitialized = false;

void setup() {
  // put your setup code here, to run once:
  
  i2c_bus_mutex = xSemaphoreCreateMutex();
  printf("Setup Done"); 
  start_oled_task(); 
  // if (!isWireInitialized) {
  //   Wire.begin();
  //   isWireInitialized = true;
  // }
  
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
// start_test_mqtt();
  // xTaskCreate(&OnConnected, "handel comms", 1024 * 3, NULL, 5, NULL);s
}

void loop() {
  // put your main code here, to run repeatedly:
  // printf("Setup Done\n");
  // delay(1000);
}
