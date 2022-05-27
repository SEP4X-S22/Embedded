#include <ATMEGA_FreeRTOS.h>
#include <task.h>

#include <TempHumidity.h>

#include <stdio.h>
#include <event_groups.h>
#include <semphr.h>
#include <hih8120.h>

//Defining the bits of the event group
#define BIT_TEMPERATURE (1 << 0)
#define BIT_HUMIDITY (1 << 1)

//Indicates whether the initial temperature and humidity sensor setup went successfully
extern bool temp;
//Queue for readings
extern QueueHandle_t xQueue;
//Event group for sequential execution of the tasks
extern EventGroupHandle_t readingsEventGroup;

//Declaring and defining the variables for the readings
float temperature = 0.0;
float humidity = 0.0;

//Temperature and humidity task's prototype
void task_read_temp_humidity( void *pvParameters );

//Temperature and humidity task's setup
void create_task_temperature_humidity(void)
{
	xTaskCreate(
	task_read_temp_humidity //Specifies which method implements the required functionality of the task
	,  "task_read_temp_humidity"  // A name to indicate the task's function (does not affect the functionality in any way)
	,  configMINIMAL_STACK_SIZE  // Stack area size that is specifically designated for this task
	,  NULL //The arguments that will be passed to the task (if any)
	,  2  // Priority, with 3 being the highest, and 0 being the lowest.
	,  NULL ); //The handle of the task (if any)
}

//Method that defines the functionality of a running task
void task_read_temp_humidity(void *pvParameters){
	
	//Variable to hold the current time
	TickType_t xLastWakeTime;
	//The frequency at which the task will run
	const TickType_t xFrequency = pdMS_TO_TICKS(300000UL); // 5 minutes
	// Initializing the xLastWakeTime variable with the current time
	xLastWakeTime = xTaskGetTickCount();

	for(;;)
	{
		if ( 1 == temp )
		{
			//Waking up the sensor
			if ( HIH8120_OK == hih8120_wakeup() )
			{
				vTaskDelay(50);
				//Measuring temperature
				if ( HIH8120_OK ==  hih8120_measure() )
				{
					vTaskDelay(1);
					//Saving the reading in a variable
					temperature = hih8120_getTemperature();
					//Enqueueing the reading
					if(xQueueSend(xQueue, ( void * ) &temperature, 0) == pdPASS) {
						printf("The temperature is %f Celsius\n",temperature);
						//Setting the temperature bit to 1 in the event group
						xEventGroupSetBits(readingsEventGroup, BIT_TEMPERATURE);
						}
					//Same steps as for temperature, but for humidity
					humidity = hih8120_getHumidity();
					if(xQueueSend(xQueue, ( void * ) &humidity, 0) == pdPASS) {
						printf("The humidity is %f%%\n",humidity);
						xEventGroupSetBits(readingsEventGroup, BIT_HUMIDITY);
					}
				}
			}
		}
		//Putting the task to sleep for the amount of time that is defined in the beginning of this task
		xTaskDelayUntil( &xLastWakeTime, xFrequency );
	}
}