#ifndef LIGHT_H
#define LIGHT_H
#include <ATMEGA_FreeRTOS.h>
#include <tsl2591.h>


void create_task_light(void);

void task_read_light(void *pvparameters);
//Callback to obtain the sensor's reading
void task_light_callback(tsl2591_returnCode_t rc);

uint16_t getLatestLight();

#endif
