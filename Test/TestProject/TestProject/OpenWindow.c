#include "OpenWindow.h"
#include "../Tests/FreeRTOS/FreeRTOS.h"
#include "../Tests/FreeRTOS/task.h"
#include "../Tests/FreeRTOS/semphr.h"
#include "../../../SEP4/drivers/rc_servo.h"
#include "../Tests/FreeRTOS/event_groups.h"
#include <stdio.h>
#include <stdbool.h>


int readingsFromC02 = 0;
//Upper and lower constraints of CO2 levels to which Servo has to react
int upperConstraint = 0;
int lowerConstraint = 0;
bool isWindowOpen = false;
SemaphoreHandle_t constraintsHandle = NULL;
//extern EventGroupHandle_t readingsEventGroup;

#define BIT_CO2 (1 << 2)
//Indicates that all of the tasks that needed to be done before transmission already ran
#define BIT_COMPLETE (1 << 3)
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
	rc_servo_initialise();
	xLastWakeTime = xTaskGetTickCount();
	constraintsHandle = xSemaphoreCreateMutex();
	xSemaphoreGive(constraintsHandle);
	
}

void task_open_window_run()
{
	//EventBits_t readingsStatus;
	//Waiting for the CO2 bit to be set, meaning that the latest CO2 reading is available
	//readingsStatus = xEventGroupWaitBits(readingsEventGroup, BIT_CO2, pdTRUE, pdTRUE, portMAX_DELAY);
	if (xSemaphoreTake(constraintsHandle, portMAX_DELAY) == pdTRUE)
	{
		readingsFromC02 = 0;
		printf("The lower bound constraint: %d\n", lowerConstraint);
		printf("The upper bound constraint: %d\n", upperConstraint);
		openCloseWindow();
		
	}
	
	vTaskDelayUntil(&xLastWakeTime, xFrequency);
}

void openCloseWindow()
{
	if (!isWindowOpen && upperConstraint < readingsFromC02)
	{
		rc_servo_setPosition(0, 100);
		isWindowOpen = true;
		puts("Window opened.");
	}
	if (isWindowOpen && lowerConstraint > readingsFromC02)
	{
		rc_servo_setPosition(0, -100);
		isWindowOpen = false;
		puts("Window closed.");
	}
	//xEventGroupSetBits(readingsEventGroup, BIT_COMPLETE);
	//Giving back the mutex when the Servo logic is done executing
	xSemaphoreGive(constraintsHandle);
}

void task_open_window(void* pvParameters) {
	//c02Semaphore = (SemaphoreHandle_t)pvParameters;
	
	task_open_window_init();
	
	for (;;)
	{
		task_open_window_run();
	}
}
void setUpperConstraint(int up)
{
	upperConstraint = up;
}

//Setting the lower constraint of CO2 levels. Protected by mutex
void setLowerConstraint(int down)
{
	if (xSemaphoreTake(constraintsHandle, portMAX_DELAY) == pdTRUE) {
		lowerConstraint = down;
		xSemaphoreGive(constraintsHandle);
	}
}