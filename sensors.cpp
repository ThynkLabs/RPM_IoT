#include "sensors.h"

TaskHandle_t task_temp_handle;
TaskHandle_t task_pulse_sensor_handle;
TaskHandle_t task_fall_detection_handle;
TaskHandle_t task_ecg_sensor_handle;

SemaphoreHandle_t i2c_bus_mutex;


//--------------------------------------------------Temperature Sensor------------------------------------------------------

// DHT Sensor
uint8_t DHTPin = 4;

// Initialize DHT sensor.
DHT dht(DHTPin, DHTTYPE);

void start_temp_sensor_task() {
  xTaskCreatePinnedToCore(&temp_sensor_task, "temp_sensor_task", TEMP_TASK_STACK_SIZE, NULL, TEMP_TASK_PRIORITY, &task_temp_handle, 0);
}

void temp_sensor_task(void *Params) {

  pinMode(DHTPin, INPUT);
  dht.begin();
  while (1) {
    // read the ADC value from the temperature sensor
    int adcVal = analogRead(PIN_LM35);
    // convert the ADC value to voltage in millivolt
    float milliVolt = adcVal * (ADC_VREF_mV / ADC_RESOLUTION);
    // convert the voltage to the temperature in °C
    float tempC = milliVolt / 10;

    float Humidity = dht.readHumidity();
    store_set_temperature(tempC);
    // ESP_LOGI(SENSOR_TAG,"Humidity : %f",Humidity);
    store_set_humidity(Humidity);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}


//--------------------------------------------------Pulse Sensor------------------------------------------------------

PulseOximeter pox;

uint32_t tsLastReport = 0;

void onBeatDetected() {
  ESP_LOGI(SENSOR_TAG, "BEAT.");
}

void pulse_sensor_task(void *Params) {

  ESP_LOGI(SENSOR_TAG, "Initializing pulse oximeter..");

  // Initialize the PulseOximeter instance
  // Failures are generally due to an improper I2C wiring, missing power supply
  // or wrong target chip
  if (!pox.begin()) {
    ESP_LOGI(SENSOR_TAG, "FAILED");
    for (;;)
      ;
  } else {
    ESP_LOGI(SENSOR_TAG, "SUCCESS");
  }

  // The default current for the IR LED is 50mA and it could be changed
  // by uncommenting the following line. Check MAX30100_Registers.h for all the
  // available options.
  // pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);

  // Register a callback for the beat detection
  pox.setOnBeatDetectedCallback(onBeatDetected);
  while (1) {
    // Make sure to call update as fast as possible
    if (xSemaphoreTake(i2c_bus_mutex, 1000 / portTICK_PERIOD_MS)) {
      pox.update();

      // Asynchronously dump heart rate and oxidation levels to the serial
      // For both, a value of 0 means "invalid"
      if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
        // spo2 =pox.getSpO2();
        // pulse =pox.getHeartRate() ;
        store_set_spo2(pox.getSpO2());
        store_set_pulse(pox.getHeartRate());
        ESP_LOGI(SENSOR_TAG, "Heart Rate: %f/bpm.", pox.getHeartRate());
        ESP_LOGI(SENSOR_TAG, "spo2: %d %", pox.getSpO2());
        // if(store_get_monitoring_flag_status())
        // {
        // pulse_sensor_data_streamer(pox.getSpO2(), pox.getHeartRate());
        // }
        tsLastReport = millis();
      }

      xSemaphoreGive(i2c_bus_mutex);
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}
void start_pulse_sensor_task() {
  xTaskCreatePinnedToCore(&pulse_sensor_task, "pulse_sensor_task", PULSE_TASK_STACK_SIZE, NULL, PULSE_TASK_PRIORITY, &task_temp_handle, 0);
}

//---------------------------------------------Fall Detection---------------------------------------------

const int MPU_addr = 0x68;  // I2C address of the MPU-6050
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;
float ax = 0, ay = 0, az = 0, gx = 0, gy = 0, gz = 0;
boolean fall = false;      //stores if a fall has occurred
boolean trigger1 = false;  //stores if first trigger (lower threshold) has occurred
boolean trigger2 = false;  //stores if second trigger (upper threshold) has occurred
boolean trigger3 = false;  //stores if third trigger (orientation change) has occurred
byte trigger1count = 0;    //stores the counts past since trigger 1 was set true
byte trigger2count = 0;    //stores the counts past since trigger 2 was set true
byte trigger3count = 0;    //stores the counts past since trigger 3 was set true
int angleChange = 0;

void start_fall_detection_task() {
  xTaskCreatePinnedToCore(&fall_detection_task, "fall_detection_task", FALL_DETECTION_TASK_STACK_SIZE, NULL, FALL_DETECTION_TASK_PRIORITY, &task_fall_detection_handle, 0);
}

void mpu_read() {
  if (xSemaphoreTake(i2c_bus_mutex, 1000 / portTICK_PERIOD_MS)) {
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_addr, 14, true);  // request a total of 14 registers
    AcX = Wire.read() << 8 | Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
    AcY = Wire.read() << 8 | Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
    AcZ = Wire.read() << 8 | Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
    Tmp = Wire.read() << 8 | Wire.read();  // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
    GyX = Wire.read() << 8 | Wire.read();  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
    GyY = Wire.read() << 8 | Wire.read();  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
    GyZ = Wire.read() << 8 | Wire.read();  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
    xSemaphoreGive(i2c_bus_mutex);
  }
}

void fall_detection_task(void *Params) {
  // Wire.setPins(23,24);

  if (xSemaphoreTake(i2c_bus_mutex, 2000 / portTICK_PERIOD_MS)) {
    Wire.begin();
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x6B);  // PWR_MGMT_1 register
    Wire.write(0);     // set to zero (wakes up the MPU-6050)
    Wire.endTransmission(true);
    xSemaphoreGive(i2c_bus_mutex);
  }

  while (1) {
    mpu_read();
    ax = (AcX - 2050) / 16384.00;
    ay = (AcY - 77) / 16384.00;
    az = (AcZ - 1947) / 16384.00;
    gx = (GyX + 270) / 131.07;
    gy = (GyY - 351) / 131.07;
    gz = (GyZ + 136) / 131.07;
    // calculating Amplitute vactor for 3 axis
    float raw_amplitude = pow(pow(ax, 2) + pow(ay, 2) + pow(az, 2), 0.5);
    int amplitude = raw_amplitude * 10;  // Mulitiplied by 10 bcz values are between 0 to 1
    // ESP_LOGI(SENSOR_TAG, "FA: %d", amplitude);
    if (amplitude <= 4 && trigger2 == false) {  //if AM breaks lower threshold (0.4g)
      trigger1 = true;
      ESP_LOGI(SENSOR_TAG, "TRIGGER 1 ACTIVATED");
    }
    if (trigger1 == true) {
      trigger1count++;
      if (amplitude >= 12) {  //if AM breaks upper threshold (3g)
        trigger2 = true;
        ESP_LOGI(SENSOR_TAG, "TRIGGER 2 ACTIVATED");
        trigger1 = false;
        trigger1count = 0;
      }
    }
    if (trigger2 == true) {
      trigger2count++;
      angleChange = pow(pow(gx, 2) + pow(gy, 2) + pow(gz, 2), 0.5);
      ESP_LOGI(SENSOR_TAG, "AC: %d", angleChange);
      if (angleChange >= 30 && angleChange <= 400) {  //if orientation changes by between 80-100 degrees
        trigger3 = true;
        trigger2 = false;
        trigger2count = 0;
        // ESP_LOGI(SENSOR_TAG, "AC: %d", angleChange);
        ESP_LOGI(SENSOR_TAG, "TRIGGER 3 ACTIVATED");
      }
    }
    if (trigger3 == true) {
      trigger3count++;
      if (trigger3count >= 10) {
        angleChange = pow(pow(gx, 2) + pow(gy, 2) + pow(gz, 2), 0.5);
        ESP_LOGI(SENSOR_TAG, "AC: %d", angleChange);
        if ((angleChange >= 0) && (angleChange <= 10)) {  //if orientation changes remains between 0-10 degrees
          fall = true;
          trigger3 = false;
          trigger3count = 0;
          ESP_LOGI(SENSOR_TAG, "AC: %d", angleChange);
        } else {  //user regained normal orientation
          trigger3 = false;
          trigger3count = 0;
          ESP_LOGI(SENSOR_TAG, "TRIGGER 3 DEACTIVATED");
        }
      }
    }
    if (fall == true) {  //in event of a fall detection
      ESP_LOGI(SENSOR_TAG, "FALL DETECTED");
      //  send_event("FALL DETECTION");
          store_change_current_screen(3);
      fall = false;
    }
    if (trigger2count >= 6) {  //allow 0.5s for orientation change
      trigger2 = false;
      trigger2count = 0;
      ESP_LOGI(SENSOR_TAG, "TRIGGER 2 DECACTIVATED");
    }
    if (trigger1count >= 6) {  //allow 0.5s for AM to break upper threshold
      trigger1 = false;
      trigger1count = 0;
      ESP_LOGI(SENSOR_TAG, "TRIGGER 1 DECACTIVATED");
    }

    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}


//-----------------------------------------------------------------ECG Sensor Task------------------------------------------------

void start_ecg_sensor_task() {
  xTaskCreatePinnedToCore(&ecg_sensor_task, "ecg_sensor_task", ECG_SENSOR_TASK_STACK_SIZE, NULL, ECG_SENSOR_TASK_PRIORITY, &task_ecg_sensor_handle, 0);
}

void ecg_sensor_task(void *Params) {
  pinMode(ECG_SENSOR_PIN, INPUT);
  char *my_string;
  while (1) {
    float sensor = analogRead(ECG_SENSOR_PIN);
    // Set in store;
    store_set_ecg(sensor);
    // Report to hivemq;

    // sprintf(my_string, "%.2f", sensor);
    // sprintf(num, "%f", sensor);
    // send_data_to_streamer_queue(ECG_TOPIC, num);
    // if (store_get_mqtt_status() == true && store_get_monitoring_flag_status() == true) {
    //   cJSON *data = cJSON_CreateObject();
    //   cJSON_AddNumberToObject(data, "ecg", sensor);
    //   char *string;
    //   string = cJSON_PrintUnformatted(data);
    //   // send_data_to_streamer_queue(PULSE_TOPIC, string);
    //   publish_message(ECG_TOPIC, string);
    //   cJSON_Delete(data);
    //   cJSON_free(string);
    // }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}


void sensor_data_generator_streamer(float temperature, float humidity, uint8_t spo2, float pulse) {
  cJSON *data = cJSON_CreateObject();
  cJSON_AddNumberToObject(data, "temperature", temperature);
  cJSON_AddNumberToObject(data, "humidity", humidity);
  cJSON_AddNumberToObject(data, "spo2", spo2);
  cJSON_AddNumberToObject(data, "pulse", pulse);
  char *string;
  string = cJSON_PrintUnformatted(data);
  send_data_to_streamer_queue(STREAM_TOPIC, string);
  cJSON_Delete(data);
  cJSON_free(string);
}

void pulse_sensor_data_streamer(uint8_t spo2, float pulse) {
  cJSON *data = cJSON_CreateObject();
  cJSON_AddNumberToObject(data, "spo2", spo2);
  cJSON_AddNumberToObject(data, "pulse", pulse);
  char *string;
  string = cJSON_PrintUnformatted(data);
  send_data_to_streamer_queue(PULSE_TOPIC, string);
  // mqttClient.publish(PULSE_TOPIC, string);
  cJSON_Delete(data);
  cJSON_free(string);
}

void start_reporter_task() {
  xTaskCreatePinnedToCore(&reporter, "reporter", REPORTER_TASK_STACK_SIZE, NULL, REPORTER_TASK_PRIORITY, &task_reporter_handle, 0);
}

void reporter(void *param) {
  while (1) {
    if (store_get_mqtt_status() == true && store_get_wifi_status() == true && store_get_monitoring_flag_status() == true) {
      // mqttClient.publish("bncoe/rpm/sensor/data", data);
      // ESP_LOGI(SENSOR_TAG,"SPO2:%d, pulse:%f",spo2,pulse);
      sensor_data_generator_streamer(25.6, store_get_humidity(), store_get_spo2(), store_get_pulse());
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
