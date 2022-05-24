#ifndef TEMP_HUMIDITY_H_
#define TEMP_HUMIDITY_H_
#include <stdbool.h>
#include <ATMEGA_FreeRTOS.h>
#include <message_buffer.h>

#include <serial.h>

//Handling the creation of a task that manages temperature and humidity readings
void create_task_temperature_humidity(void);

//The method that defines the logic of the temperature and humidity task
void task_read_temp_humidity(void *pvParameters);

#endif