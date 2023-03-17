#include "hmi.h"
#include "connection.h"
void setup() {
  // put your setup code here, to run once:
  printf("Setup Done");
  
  streamer_queue = xQueueCreate(7, 10*sizeof(char));
   start_led_indication_task();
  start_interrupt_task();
  start_oled_task();
  start_wifi_task();
  start_mqtt_task();
  
start_streamer_task();
 start_reporter_task(); 
// start_test_mqtt();
  // xTaskCreate(&OnConnected, "handel comms", 1024 * 3, NULL, 5, NULL);s
}

void loop() {
  // put your main code here, to run repeatedly:
  // printf("Setup Done\n");
  // delay(1000);
}
