#include "connection.h"

const char* ssid = "Dany";
const char* password = "Dany1234";
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char* mqtt_user = "";
const char* mqtt_password = "";
const char* mqtt_client_id = "rpm_test";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

QueueHandle_t streamer_queue;

TaskHandle_t task_wifi_handle;
TaskHandle_t task_mqtt_handle;
TaskHandle_t task_streamer_handle;
TaskHandle_t task_reporter_handle;

void start_wifi_task() {
  xTaskCreatePinnedToCore(&connectToWiFi, "connectToWiFi", WIFI_TASK_STACK_SIZE, NULL, WIFI_TASK_PRIORITY, &task_wifi_handle, 1);
}

void connectToWiFi(void* parameter) {
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  printf("");
  printf("WiFi connected\n");
  Serial.print("IP address: \n");
  printf("%s\n", WiFi.localIP());
  // vTaskDelete(NULL);
  while (true) {
    if (WiFi.status() != WL_CONNECTED) {
      printf("wifi Not Connected\n");
      store_set_wifi_status(false);
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}


void start_mqtt_task() {
  xTaskCreatePinnedToCore(&connectToMQTT, "connectToMQTT", MQTT_TASK_STACK_SIZE, NULL, MQTT_TASK_PRIORITY, &task_mqtt_handle, 0);
}

void connectToMQTT(void* parameter) {
  mqttClient.setServer(mqtt_server, 1883);
  while (true) {
    if (WiFi.status() == WL_CONNECTED) {
      store_set_wifi_status(true);
      if (!mqttClient.connected()) {
        Serial.print("Connecting to MQTT server...\n");
        store_set_mqtt_status(false);
        if (mqttClient.connect(mqtt_client_id)) {
          printf("");
          printf("MQTT connected\n");
          store_set_mqtt_status(true);
        } else {
          Serial.print(".");
        }
      }
      mqttClient.loop();
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}



void sensor_data_generator_streamer(float temperature, uint8_t humidity, uint8_t spo2,uint8_t pulse){
    cJSON *data = cJSON_CreateObject();
    cJSON_AddNumberToObject(data, "temperature",temperature);
    cJSON_AddNumberToObject(data, "humidity",humidity);
    cJSON_AddNumberToObject(data, "spo2",spo2);
    cJSON_AddNumberToObject(data, "pulse",pulse);
    char *string;
    string = cJSON_PrintUnformatted(data);
    send_data_to_streamer_queue(string);
    cJSON_Delete(data);
    cJSON_free(string);
}

void start_streamer_task() {
  xTaskCreatePinnedToCore(&streamer, "streamer", STREAMER_TASK_STACK_SIZE, NULL, STREAMER_TASK_PRIORITY, &task_streamer_handle, 0);
}

void streamer(void* param) {
  char *data= NULL;
  while (1) {
    if (xQueueReceive(streamer_queue, &data, 100 / portTICK_PERIOD_MS)) {
      // printf("PUB STREAM %s\n", data);
      mqttClient.publish("bncoe/rpm/stream", data);
    }
  }
}

void send_data_to_streamer_queue(char*stream)
{
  xQueueSend(streamer_queue, &stream, 100 / portTICK_PERIOD_MS);  
}

void start_test_mqtt() {
  xTaskCreatePinnedToCore(&test_mqtt, "test_mqtt", 2048, NULL, 1, NULL, 0);
}

void test_mqtt(void* param) {
  char* send = "hello";
  while (1) {
    if (store_get_mqtt_status() == true) {
      
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void start_reporter_task() {
  xTaskCreatePinnedToCore(&reporter, "reporter", REPORTER_TASK_STACK_SIZE, NULL, REPORTER_TASK_PRIORITY, &task_reporter_handle, 0);
}

void reporter(void* param) {
  while (1) {
    if (store_get_mqtt_status() == true && store_get_wifi_status() == true && store_get_monitoring_flag_status()==true)
    {
      // mqttClient.publish("bncoe/rpm/sensor/data", data);      
      sensor_data_generator_streamer(25.6,60,95,69);
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}
