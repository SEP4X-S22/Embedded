#ifndef OPEN_WINDOW_H_
#define OPEN_WINDOW_H_
#include <stdbool.h>
#include <ATMEGA_FreeRTOS.h>

void create_task_open_window(void);
void task_open_window(void *pvParameters);
bool getIsWindowOpen();
void setUpperConstraint(int up);
void setLowerConstraint(int down);

#endif /* OPEN_WINDOW_H_ */