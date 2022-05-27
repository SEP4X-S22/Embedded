#include <ATMEGA_FreeRTOS.h>
#include <task.h>
#include <stdio.h>

#include <mh_z19.h>
#include <semphr.h>
#include <event_groups.h>

//Defining the bits of the event group
#define BIT_TEMPERATURE (1 << 0)
#define BIT_HUMIDITY (1 << 1)
#define BIT_CO2 (1 << 2)

//Event group for sequential execution of the tasks
extern EventGroupHandle_t readingsEventGroup;

//Last reading of the CO2 sensor
uint16_t lastCO2Value = 0;

// Simple getter function to retrieve latest CO2
uint16_t getLatestCO2() {
	return lastCO2Value;
}

// Callback function for persisting the reading when the sensor had read the CO2 level
void task_co2_callback(uint16_t ppm)
{
	lastCO2Value = ppm;
	printf("The CO2 level is %d ppm\n", getLatestCO2());
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
		
		// Indicate the sensor to start making a measurement
		int r = mh_z19_takeMeassuring();
		
		// If any errors occur when measuring print an error
		if (r != MHZ19_OK) {
			printf("Could not measure the C02");
		}	
		 xTaskDelayUntil(&xLastWakeTime, xFrequency);
	}
}

// Creating the task for CO2 reading
void create_task_c02(void)
{
	// Initialize the CO2 sensor
	mh_z19_initialise(ser_USART3);
	
	// Set the callback function so the sensor can call and give it the latest measurement
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

