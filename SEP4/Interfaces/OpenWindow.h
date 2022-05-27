#ifndef OPEN_WINDOW_H_
#define OPEN_WINDOW_H_
#include <stdbool.h>
#include <ATMEGA_FreeRTOS.h>

//Creates Servo task
void create_task_open_window(void);

//Method that defines the functionality of Servo task
void task_open_window(void *pvParameters);
void task_open_window_init();
void task_open_window_run();

//Checks whether the window is open
bool getIsWindowOpen();

//Sets upper constraint of CO2 recommended level
void setUpperConstraint(int up);

//Sets lower constraint of CO2 recommended level
void setLowerConstraint(int down);

#endif /* OPEN_WINDOW_H_ */