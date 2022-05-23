#include <OpenWindow.h>
#include <ATMEGA_FreeRTOS.h>
#include <task.h>
#include <semphr.h>
//Temp and humidity
#include <rc_servo.h>
#include <stdio.h>


//extern EventGroupHandle_t readingsEventGroup;
int readingsFromC02 = 0;
int upperConstraint;
int lowerConstraint;
bool isWindowOpen = false;
extern SemaphoreHandle_t c02Semaphore;


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
	,  3  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
	,  NULL );
}

bool getIsWindowOpen()
{
	return isWindowOpen;
}
void setUpperConstraint(int up)
{
	upperConstraint = up;
}
void setLowerConstraint(int down)
{
	lowerConstraint = down;
}

void task_open_window(void *pvParameters){
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = pdMS_TO_TICKS(300000UL); // 5 minutes

	// Initialise the xLastWakeTime variable with the current time.
	xLastWakeTime = xTaskGetTickCount();
	rc_servo_initialise();
//TODO 
///finish the initialization and spinning

	for(;;)
	{
			
 		if ( xSemaphoreTake(c02Semaphore, portMAX_DELAY) == pdTRUE  )
 		{
			 readingsFromC02 = getLatestCO2();
			 
			 if(!isWindowOpen && upperConstraint<=readingsFromC02)
			 {
				rc_servo_setPosition(0, 100);
				isWindowOpen = true;
				printf("Window opened.\n");
			 }
			 if(isWindowOpen && lowerConstraint>=readingsFromC02)
			 {
				 rc_servo_setPosition(0, 0);
				 isWindowOpen = false;
				 printf("Window closed.\n");
			 }
			 
			 xSemaphoreGive(c02Semaphore);
 		}
		
		xTaskDelayUntil( &xLastWakeTime, xFrequency );
	}
}