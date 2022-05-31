/*
Author: Max
*/

#ifndef CO2_H_
#define CO2_H_

#include <stdbool.h>
#include <ATMEGA_FreeRTOS.h>
#include <message_buffer.h>

//Getting latest CO2 reading
uint16_t getLatestCO2();
//Callback to obtain the sensor's reading
void task_co2_callback(uint16_t ppm);
//Handling the creation of a task that manages CO2 readings
void create_task_c02(void);
//Defines the task's logic of CO2 reading
void task_read_c02(void *pvParameters);

#endif /* CO2_H_ */