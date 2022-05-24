#include <ATMEGA_FreeRTOS.h>
#include <task.h>
#include <semphr.h>

// Includes for C02
#include <mh_z19.h>
#include <stdio.h>
#include <event_groups.h>


//Event group for sequential execution of the tasks
extern EventGroupHandle_t readingsEventGroup;


//Defining the bits of the event group
#define BIT_TEMPERATURE (1 << 0)
#define BIT_HUMIDITY (1 << 1)
#define BIT_CO2 (1 << 2)

//Last reading of the CO2 sensor
uint16_t lastCO2Value = 0;

uint16_t getLatestCO2() {
	return lastCO2Value;
}

//Persisting the reading when the sensor had read the CO2 level
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
	
	xLastWakeTime = xTaskGetTickCount();

	for(;;)
	{
		//Variable to store event group's result
		EventBits_t readingsStatus;
		//Waiting until the temperature and humidity bits are set
		readingsStatus = xEventGroupWaitBits(readingsEventGroup, BIT_TEMPERATURE | BIT_HUMIDITY, pdTRUE, pdTRUE, portMAX_DELAY);
		//Measuring CO2
		int r = mh_z19_takeMeassuring();
		if (r != MHZ19_OK) {
			printf("Could not measure the C02");
		}	
		 xTaskDelayUntil(&xLastWakeTime, xFrequency);
	}
}

//Creating the task for CO2 reading
void create_task_c02(void)
{
	mh_z19_initialise(ser_USART3);
	mh_z19_injectCallBack(task_co2_callback);
	
	xTaskCreate(
		task_read_c02,
		"task_read_c02",
		configMINIMAL_STACK_SIZE,
		NULL,
		2,
		NULL
	);
}

