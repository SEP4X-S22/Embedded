 #include <TempHumidity.h>
#include <ATMEGA_FreeRTOS.h>
#include <task.h>
#include <semphr.h>
//Temp and humidity
#include <hih8120.h>
#include <stdio.h>
#include <event_groups.h>

extern bool  temp;
extern QueueHandle_t xQueue;
extern EventGroupHandle_t readingsEventGroup;

#define BIT_TEMPERATURE (1 << 0)
#define BIT_HUMIDITY (1 << 1)

void task_read_temp_humidity( void *pvParameters );

float temperature = 0.0;
float humidity = 0.0;

void create_task_temperature_humidity(void)
{
	xTaskCreate(
	task_read_temp_humidity
	,  "task_read_temp_humidity"  // A name just for humans
	,  configMINIMAL_STACK_SIZE  // This stack size can be checked & adjusted by reading the Stack Highwater
	,  NULL
	,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
	,  NULL );
}

void task_read_temp_humidity(void *pvParameters){
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = pdMS_TO_TICKS(300000UL); // 5 minutes

	// Initialise the xLastWakeTime variable with the current time.
	xLastWakeTime = xTaskGetTickCount();

	for(;;)
	{
		if ( 1 == temp )
		{
			if ( HIH8120_OK == hih8120_wakeup() )
			{
				vTaskDelay(50);
				if ( HIH8120_OK ==  hih8120_measure() )
				{
					vTaskDelay(1);
					temperature = hih8120_getTemperature();
					if(xQueueSend(xQueue, ( void * ) &temperature, 0) == pdPASS) {
						printf("%f\n",temperature);
						xEventGroupSetBits(readingsEventGroup, BIT_TEMPERATURE);
						}
					humidity = hih8120_getHumidity();
					if(xQueueSend(xQueue, ( void * ) &humidity, 0) == pdPASS) {
						printf("%f\n",humidity);
						xEventGroupSetBits(readingsEventGroup, BIT_HUMIDITY);
						
					}
				}
			}
		}
		
		xTaskDelayUntil( &xLastWakeTime, xFrequency );
	}
}