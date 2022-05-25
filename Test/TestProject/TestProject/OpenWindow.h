#ifndef OPEN_WINDOW_H_
#define OPEN_WINDOW_H_
#include <stdbool.h>
#include "../Tests/FreeRTOS/FreeRTOS.h"


void create_task_open_window(void);
void task_open_window(void* pvParameters);
bool getIsWindowOpen();
void task_open_window_init();
void task_open_window_run();
void setUpperConstraint(int up);
void setLowerConstraint(int down);
void openCloseWindow();

#endif /* OPEN_WINDOW_H_ */
