#ifndef CO2_H_
#define CO2_H_
#include <stdbool.h>
#include <ATMEGA_FreeRTOS.h>
#include <message_buffer.h>

#include <serial.h>

void create_task_c02(void);
void task_read_c02(void *pvParameters);

#endif /* C02_H_ */