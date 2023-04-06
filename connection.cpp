#include "connection.h"

const char* ssid = "Test";
const char* password = "Test1234";
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
  store_set_init_screen_buffer("Connecting to wifi..");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  printf("");
  printf("WiFi connected\n");
  store_set_init_screen_buffer("Connected to wifi!");
  Serial.print("IP address: \n");
  printf("%s\n", WiFi.localIP());
  // vTaskDelete(NULL);
  while (true) {
    if (WiFi.status() != WL_CONNECTED) {
      printf("wifi Not Connected\n");
      store_set_wifi_status(false);
      store_set_init_screen_buffer("Connecting to wifi..");
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
        store_set_init_screen_buffer("Connecting HiveMQ....");
        store_set_mqtt_status(false);
        if (mqttClient.connect(mqtt_client_id)) {
          store_set_init_screen_buffer("Connected to HiveMQ");
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



void start_streamer_task() {
  xTaskCreatePinnedToCore(&streamer, "streamer", STREAMER_TASK_STACK_SIZE, NULL, STREAMER_TASK_PRIORITY, &task_streamer_handle, 0);
}

void streamer(void* param) {
  mqtt_publish_queue_struct queue_data;
  streamer_queue = xQueueCreate(15, 10*sizeof(char));
  while (1) {
    if (xQueueReceive(streamer_queue, &(queue_data), 50 / portTICK_PERIOD_MS)) {
      // printf("PUB STREAM %s\n", data);
      mqttClient.publish(queue_data.topic, queue_data.message);
    }
  }
}

void send_data_to_streamer_queue(char*topic,char*message)
{
  mqtt_publish_queue_struct queue_data;
  queue_data.topic=topic;
  queue_data.message = message;
  xQueueSend(streamer_queue,(void *)&queue_data, 10 / portTICK_PERIOD_MS);  
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
void publish_message(char*topic,char*message)
{
  mqttClient.publish(topic, message);
}