#include <OpenWindow.h>
#include "ATMEGA_FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "C02.h"
#include "rc_servo.h"
#include <stdio.h>
#include "event_groups.h"

//for the delay
TickType_t xLastWakeTime;
const TickType_t xFrequency = pdMS_TO_TICKS(300000UL); // 5 minutes

//Variable for latest CO2 reading
int readingsFromC02 = 0;

//Upper and lower constraints of CO2 levels to which Servo has to react
int upperConstraint = 0;
int lowerConstraint = 0;

bool isWindowOpen = false;
SemaphoreHandle_t constraintsHandle = NULL;

extern EventGroupHandle_t readingsEventGroup;

#define BIT_LIGHT (1 << 3)
//Indicates that all of the tasks that needed to be done before transmission already ran
#define BIT_COMPLETE (1 << 4)

//Servo task's prototype
void task_open_window( void *pvParameters );

//Creating Servo task
void create_task_open_window(void)
{
	xTaskCreate(
	task_open_window
	,  "task_open_window"
	,  configMINIMAL_STACK_SIZE
	,  NULL
	,  2
	,  NULL );
}

bool getIsWindowOpen()
{
	return isWindowOpen;
}

//Setting the upper constraint of CO2 levels. Protected by mutex
void setUpperConstraint(int up)
{
	if(xSemaphoreTake(constraintsHandle, portMAX_DELAY) == pdPASS) {
		upperConstraint = up;
		xSemaphoreGive(constraintsHandle);
	}
}

//Setting the lower constraint of CO2 levels. Protected by mutex
void setLowerConstraint(int down)
{
	if(xSemaphoreTake(constraintsHandle, portMAX_DELAY) == pdPASS) {
		lowerConstraint = down;
		xSemaphoreGive(constraintsHandle);
	}
}

//Method that defines the functionality of the Servo task
void task_open_window(void *pvParameters){
	
task_open_window_init();

	for(;;)
	{
		task_open_window_run();
	}
}

void task_open_window_init()
{
		
		//Creating the mutex when the program is run for the first time
		constraintsHandle = xSemaphoreCreateMutex();
		if(NULL != constraintsHandle) {
			xSemaphoreGive(constraintsHandle);
		}

		xLastWakeTime = xTaskGetTickCount();
		
		rc_servo_initialise();
}
void task_open_window_run()
{
	EventBits_t readingsStatus;
	//Waiting for the CO2 bit to be set, meaning that the latest CO2 reading is available
	readingsStatus = xEventGroupWaitBits(readingsEventGroup, BIT_LIGHT, pdTRUE, pdTRUE, portMAX_DELAY);
	
	//Obtaining mutex and performing Servo logic
	if(xSemaphoreTake(constraintsHandle, portMAX_DELAY) == pdPASS) {
		readingsFromC02 = getLatestCO2();
		printf("The lower bound constraint: %d\n", lowerConstraint);
		printf("The upper bound constraint: %d\n", upperConstraint);
		
		
		//If the "window" was closed and the current CO2 reading is higher than the recommended upper bound, the "window" opens
		if(!isWindowOpen && upperConstraint<readingsFromC02)
		{
			rc_servo_setPosition(0, 100);
			isWindowOpen = true;
			printf("Window opened.\n");
		}
		
		//If the "window" was open and the current CO2 reading is less than the recommended lower bound, the "window" closes
		if(isWindowOpen && lowerConstraint>readingsFromC02)
		{
			rc_servo_setPosition(0, 0);
			isWindowOpen = false;
			printf("Window closed.\n");
		}
		//Setting the complete bit, indicating that all internal tasks were done and it is ready for transmission
		xEventGroupSetBits(readingsEventGroup, BIT_COMPLETE);
		//Giving back the mutex when the Servo logic is done executing
		xSemaphoreGive(constraintsHandle);
		xTaskDelayUntil( &xLastWakeTime, xFrequency );
	}
	
}