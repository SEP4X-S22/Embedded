#include <OpenWindow.h>
#include <ATMEGA_FreeRTOS.h>
#include <task.h>
#include "semphr.h"
//Temp and humidity
#include <rc_servo.h>
#include <stdio.h>
#include <event_groups.h>


//extern EventGroupHandle_t readingsEventGroup;
int readingsFromC02 = 0;
int upperConstraint;
int lowerConstraint;
bool isWindowOpen = false;
SemaphoreHandle_t constraintsHandle = NULL;

extern EventGroupHandle_t readingsEventGroup;

#define BIT_CO2 (1 << 2)
#define BIT_COMPLETE (1 << 3)


//#define BIT_TEMPERATURE (1 << 0)
//#define BIT_HUMIDITY (1 << 1)

void task_open_window( void *pvParameters );

void create_task_open_window(void)
{
	xTaskCreate(
	task_open_window
	,  "task_open_window"  // A name just for humans
	,  configMINIMAL_STACK_SIZE  // This stack size can be checked & adjusted by reading the Stack Highwater
	,  NULL
	,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
	,  NULL );
}

bool getIsWindowOpen()
{
	return isWindowOpen;
}
void setUpperConstraint(int up)
{
	if(xSemaphoreTake(constraintsHandle, portMAX_DELAY) == pdPASS) {
		upperConstraint = up;
		xSemaphoreGive(constraintsHandle);	
	}
}
void setLowerConstraint(int down)
{
	if(xSemaphoreTake(constraintsHandle, portMAX_DELAY) == pdPASS) {
		lowerConstraint = down;
		xSemaphoreGive(constraintsHandle);
	}
}

void task_open_window(void *pvParameters){
	
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = pdMS_TO_TICKS(300000UL); // 5 minutes
	
	constraintsHandle = xSemaphoreCreateMutex();
	if(NULL != constraintsHandle) {
		xSemaphoreGive(constraintsHandle);
	}

	// Initialise the xLastWakeTime variable with the current time.
	xLastWakeTime = xTaskGetTickCount();
	rc_servo_initialise();

	for(;;)
	{
			EventBits_t readingsStatus;
			readingsStatus = xEventGroupWaitBits(readingsEventGroup, BIT_CO2, pdTRUE, pdTRUE, portMAX_DELAY);
				
			if(xSemaphoreTake(constraintsHandle, portMAX_DELAY) == pdPASS) {
			 readingsFromC02 = getLatestCO2();
			 printf("The lower bound constraint: %d\n", lowerConstraint);
			 printf("The upper bound constraint: %d\n", upperConstraint);
			
			 
			 if(!isWindowOpen && upperConstraint<readingsFromC02)
			 {
				 rc_servo_setPosition(0, 100);
				 isWindowOpen = true;
				 printf("Window opened.\n");
			 }
			 if(isWindowOpen && lowerConstraint>readingsFromC02)
			 {
				 rc_servo_setPosition(0, 0);
				 isWindowOpen = false;
				 printf("Window closed.\n");
			 }
			 xEventGroupSetBits(readingsEventGroup, BIT_COMPLETE);
			 xSemaphoreGive(constraintsHandle);
			}
				
			}
 	
		
		xTaskDelayUntil( &xLastWakeTime, xFrequency );
}