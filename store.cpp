#include "store.h"

struct board_status{
  bool board_ready=false;
  bool connection=false;
  bool mqtt_connection =false;
  bool wifi_connection=false;
}board_status;

struct hmi_store{
  uint8_t current_screen;
  bool monitoring_flag;
}hmi_store;

struct sensor_data{
  float temperature=0.0;
  uint8_t humidity=0;
  uint8_t spo2;
  uint8_t pulse;
}sensor_data;

void store_change_current_screen(uint8_t screen)
{
    hmi_store.current_screen = screen;
}

uint8_t store_get_current_screen()
{
  return hmi_store.current_screen;
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
