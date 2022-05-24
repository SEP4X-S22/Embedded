#include <ATMEGA_FreeRTOS.h>
#include <task.h>
#include <semphr.h>

// Includes for C02
#include <mh_z19.h>
#include <stdio.h>
#include <event_groups.h>

extern EventGroupHandle_t readingsEventGroup;

#define BIT_TEMPERATURE (1 << 0)
#define BIT_HUMIDITY (1 << 1)
#define BIT_CO2 (1 << 2)

uint16_t lastCO2Value = 0;

uint16_t getLatestCO2() {
	return lastCO2Value;
}

void task_co2_callback(uint16_t ppm)
{
	lastCO2Value = ppm;
	printf("%d\n", getLatestCO2());
	xEventGroupSetBits(readingsEventGroup, BIT_CO2);
}

void task_read_c02(void *pvParameters)
{
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = pdMS_TO_TICKS(300000UL); // 5 minutes

	// Initialize the xLastWakeTime variable with the current time.
	xLastWakeTime = xTaskGetTickCount();

	for(;;)
	{
		EventBits_t readingsStatus;
		readingsStatus = xEventGroupWaitBits(readingsEventGroup, BIT_TEMPERATURE | BIT_HUMIDITY, pdTRUE, pdTRUE, portMAX_DELAY);
		int r = mh_z19_takeMeassuring();
		if (r != MHZ19_OK) {
			printf("Could not measure the C02");
		}	
		 xTaskDelayUntil(&xLastWakeTime, xFrequency);
	}
}

void create_task_c02(void)
{
	mh_z19_initialise(ser_USART3);
	mh_z19_injectCallBack(task_co2_callback);
	
	xTaskCreate(
		task_read_c02,
		"task_read_c02",  // A name just for humans
		configMINIMAL_STACK_SIZE,  // This stack size can be checked & adjusted by reading the Stack Highwater
		NULL,
		2,  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
		NULL
	);
}

