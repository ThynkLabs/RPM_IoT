#ifndef _STORE_H_
#define _STORE_H_

#include <stdio.h>
#include <stdbool.h>

/**
* @brief Change Current Screen in store
*
* @param screen Current screen Number
*/
extern void store_change_current_screen(uint8_t screen);

/**
* @brief Get Current Screen from store
*
* @return Current screen Number
*/
extern uint8_t store_get_current_screen();

extern void store_board_ready(bool status);

extern bool store_get_board_ready_status();


void store_set_wifi_status(bool status);

bool store_get_wifi_status();


void store_set_mqtt_status(bool status);

bool store_get_mqtt_status();

void store_set_connection_status(bool status);

bool store_get_connection_status();

void store_set_monitoring_flag_status(bool status);
bool store_get_monitoring_flag_status();
#endif