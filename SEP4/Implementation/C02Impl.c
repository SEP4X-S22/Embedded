#include <TempHumidity.h>
#include <ATMEGA_FreeRTOS.h>
#include <task.h>
#include <semphr.h>

// Includes for C02
#include <mh_z19.h>
#include <stdio.h>
#include <event_groups.h>

#define BIT_CO2 (1 << 2)

extern QueueHandle_t xQueue;
extern EventGroupHandle_t readingsEventGroup;

mh_z19_returnCode_t rc;


void task_co2_callback(uint16_t ppm)
{
	printf("Measured CO2 value: %i\n", ppm);
	if (xQueueSend(xQueue, (void *) &ppm, 0) == pdPASS) {
		xEventGroupSetBits(readingsEventGroup, BIT_CO2);
	}
}

void task_read_c02(void *pvParameters)
{
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = pdMS_TO_TICKS(300000UL); // 5 minutes

	// Initialize the xLastWakeTime variable with the current time.
	xLastWakeTime = xTaskGetTickCount();

	for(;;)
	{
		rc = mh_z19_takeMeassuring();
		if (rc != MHZ19_OK)
		{
			printf("Could not read the c02 sensor\n");
		}
		xTaskDelayUntil(&xLastWakeTime, xFrequency);
	}
}

void create_task_c02(void)
{
	xTaskCreate(
		task_read_c02,
		"task_read_c02",  // A name just for humans
		configMINIMAL_STACK_SIZE,  // This stack size can be checked & adjusted by reading the Stack Highwater
		NULL,
		2,  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
		NULL
	);
}

