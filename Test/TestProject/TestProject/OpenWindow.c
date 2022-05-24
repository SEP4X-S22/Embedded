#include "OpenWindow.h"
#include "../Tests/FreeRTOS/FreeRTOS.h"
#include "../Tests/FreeRTOS/task.h"
#include "../Tests/FreeRTOS/semphr.h"
#include "../Tests/FreeRTOS/rc_servo.h"
#include <stdio.h>
#include <stdbool.h>


//extern EventGroupHandle_t readingsEventGroup;
int readingsFromC02 = 0;
int constraint = 1;
bool isWindowOpen = false;
SemaphoreHandle_t c02Semaphore;
TickType_t xLastWakeTime;
const TickType_t xFrequency = pdMS_TO_TICKS(300000UL);

void task_open_window(void* pvParameters);

void create_task_open_window(void)
{
	xTaskCreate(
		task_open_window
		, "task_open_window"  // A name just for humans
		, configMINIMAL_STACK_SIZE  // This stack size can be checked & adjusted by reading the Stack Highwater
		, NULL
		, 2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
		, NULL);
}

bool getIsWindowOpen()
{
	return isWindowOpen;
}

void task_open_window_init()
{
	xLastWakeTime = xTaskGetTickCount();
	c02Semaphore = xSemaphoreCreateMutex();
	xSemaphoreGive(c02Semaphore);
	
}

void task_open_window_run()
{
	if (xSemaphoreTake(c02Semaphore, (TickType_t)100) == pdTRUE)
	{
		if (!isWindowOpen && constraint <= readingsFromC02)
		{
			//rc_servo_setPosition(0, 100);
			isWindowOpen = true;
			puts("Window opened.");
		}
		if (isWindowOpen && constraint > readingsFromC02)
		{
			//rc_servo_setPosition(0, -100);
			isWindowOpen = false;
			puts("Window closed.");
		}

		xSemaphoreGive(c02Semaphore);
	}

	vTaskDelayUntil(&xLastWakeTime, xFrequency);
}

void task_open_window(void* pvParameters) {
	//c02Semaphore = (SemaphoreHandle_t)pvParameters;
	
	task_open_window_init();
	//rc_servo_initialise();
	for (;;)
	{
		task_open_window_run();
	}
}