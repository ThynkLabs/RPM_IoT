#include "store.h"

struct board_status{
  bool board_ready=false;
  bool connection=false;
  bool mqtt_connection =false;
  bool wifi_connection=false;
}board_status;

struct hmi_store{
  uint8_t current_screen= 0;
  char*init_screen_buffer;
  bool monitoring_flag;
uint8_t interrupt_time_counter=0;
}hmi_store;

struct sensor_data{
  float temperature=0.0;
  float humidity=0;
  uint8_t spo2=0;
  float pulse=0;
  float ecg;
  float hrv;
}sensor_data;

void store_change_current_screen(uint8_t screen)
{
    hmi_store.current_screen = screen;
}

uint8_t store_get_current_screen()
{
  return hmi_store.current_screen;
}

void store_set_interrupt_time_counter(uint8_t time_c)
{
    hmi_store.interrupt_time_counter = time_c;
}

uint8_t store_get_interrupt_time_counter()
{
  return hmi_store.interrupt_time_counter;
}


extern void store_board_ready(bool status)
{
  board_status.board_ready=status;
}

extern bool store_get_board_ready_status()
{
  return board_status.board_ready;
}

void store_set_wifi_status(bool status)
{
  board_status.wifi_connection=status;
}

bool store_get_wifi_status()
{
  return board_status.wifi_connection;
}


void store_set_mqtt_status(bool status)
{
  board_status.mqtt_connection=status;
}

bool store_get_mqtt_status()
{
  return board_status.mqtt_connection;
}

void store_set_connection_status(bool status)
{
  board_status.connection=status;
}

bool store_get_connection_status()
{
  return board_status.connection;
}

void store_set_monitoring_flag_status(bool status)
{
  hmi_store.monitoring_flag=status;
}

bool store_get_monitoring_flag_status()
{
  return hmi_store.monitoring_flag;
}


void store_set_init_screen_buffer(char* buffer)
{
  hmi_store.init_screen_buffer=buffer;
}

char* store_get_init_screen_buffer()
{
  return hmi_store.init_screen_buffer;
}

void store_set_temperature(float value)
{
  sensor_data.temperature=value;
}

float store_get_temperature()
{
  return sensor_data.temperature;
}


void store_set_humidity(float value)
{
  sensor_data.humidity=value;
}

float store_get_humidity()
{
  return sensor_data.humidity;
}


void store_set_pulse(float value)
{
  sensor_data.pulse=value;
}

float store_get_pulse()
{
  return sensor_data.pulse;
}


void store_set_spo2(uint8_t value)
{
  ESP_LOGI("STORE","Set SPO2 in store : %d",value);
  sensor_data.spo2=value;
}

uint8_t store_get_spo2()
{
  return sensor_data.spo2;
}


void store_set_ecg(float value)
{
  sensor_data.ecg=value;
}

float store_get_ecg()
{
  return sensor_data.ecg;
}
void store_set_hrv(float value)
{
  sensor_data.hrv=value;
}

float store_get_hrv()
{
  return sensor_data.hrv;
}