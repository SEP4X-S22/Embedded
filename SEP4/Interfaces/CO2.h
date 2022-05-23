#ifndef CO2_H_
#define CO2_H_
#include <stdbool.h>
#include <ATMEGA_FreeRTOS.h>
#include <message_buffer.h>
#include <serial.h>

uint16_t getLatestCO2();
void task_co2_callback(uint16_t ppm);
void create_task_c02(void);

#endif /* C02_H_ */