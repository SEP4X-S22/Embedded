#include <OpenWindow.h>
#include <ATMEGA_FreeRTOS.h>
#include <task.h>
#include <semphr.h>
//Temp and humidity
#include <rc_servo.h>
#include <stdio.h>
#include <event_groups.h>
#include <avr/io.h>
#include <stdio_driver.h>
#include <serial.h>


//extern EventGroupHandle_t readingsEventGroup;
int readingsFromC02 = 0;
int constraint = 1;
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
			
 		if ( xSemaphoreTake(c02Semaphore, 0) == pdTRUE  )
 		{
			 if(!isWindowOpen && constraint<=readingsFromC02)
			 {
				rc_servo_setPosition(0, 100);
				isWindowOpen = true;
				puts("Window opened.");
			 }
			 if(isWindowOpen && constraint>readingsFromC02)
			 {
				 rc_servo_setPosition(0, -100);
				 isWindowOpen = false;
				 puts("Window closed.");
			 }
			 
			 xSemaphoreGive(c02Semaphore);
 		}
		
		xTaskDelayUntil( &xLastWakeTime, xFrequency );
	}
}