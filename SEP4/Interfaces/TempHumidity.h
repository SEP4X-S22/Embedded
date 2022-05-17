#ifndef TEMP_HUMIDITY_H_
#define TEMP_HUMIDITY_H_
#include <stdbool.h>
#include <ATMEGA_FreeRTOS.h>
#include <message_buffer.h>

#include <serial.h>

void create_task_temperature_humidity(void);
void task_read_temp_humidity(void *pvParameters);

#endif /* TEMP_HUMIDITY_H_ */