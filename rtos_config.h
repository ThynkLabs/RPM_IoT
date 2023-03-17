
#ifndef _RTOS_CONFIG_H_
#define _RTOS_CONFIG_H_

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"



extern xSemaphoreHandle init_semaphore;
extern xSemaphoreHandle wifi_connection_semaphore;

//--------------------------------------------------------Queues-------------------------------------------------------------------






#endif