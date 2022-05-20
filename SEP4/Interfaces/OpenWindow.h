#ifndef OPEN_WINDOW_H_
#define OPEN_WINDOW_H_
#include <stdbool.h>
#include <ATMEGA_FreeRTOS.h>
#include <message_buffer.h>

#include <serial.h>

void create_task_open_window(void);
void task_open_window(void *pvParameters);
bool getIsWindowOpen();

#endif /* OPEN_WINDOW_H_ */