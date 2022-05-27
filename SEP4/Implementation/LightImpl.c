#include <ATMEGA_FreeRTOS.h>
#include <task.h>
#include <stdio.h>

#include <tsl2591.h>
#include <event_groups.h>
#include <semphr.h>

#define BIT_CO2 (1 << 2)
#define BIT_LIGHT (1 << 3)

//Event group for sequential execution of the tasks
extern EventGroupHandle_t readingsEventGroup;
//Queue for readings
extern QueueHandle_t xQueue;

void task_read_light(void *pvparameters) {
	
	//Setting up the frequency of execution
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = pdMS_TO_TICKS(300000UL); // 5 minutes
	xLastWakeTime = xTaskGetTickCount();
	
	for(;;) {
		EventBits_t readingsStatus;
		//Waiting for the CO2 bit to be set
		readingsStatus = xEventGroupWaitBits(readingsEventGroup,BIT_CO2, pdTRUE, pdTRUE, portMAX_DELAY);
		
		//Performing the reading
		if ( TSL2591_OK != tsl2591_fetchData()) {
			printf("Something went wrong with the light reading\n" );
		}
		//Pausing for 5 minutes
		xTaskDelayUntil( &xLastWakeTime, xFrequency );
	}
}

void task_light_callback(tsl2591_returnCode_t rc) {
	float lux = 0;
	
	//If data is ready and is successfully put into the queue, it sets the Light bit
	if(rc == TSL2591_DATA_READY) {
		if (TSL2591_OK == (rc = tsl2591_getLux(&lux))) {
				if(xQueueSend(xQueue, ( void * ) &lux, 0) == pdPASS) {
						printf("The light measurement is %5.2f lux\n", lux);
						xEventGroupSetBits(readingsEventGroup, BIT_LIGHT);
				}
			}
		}
}

//Sets up the sensor and the light task
void create_task_light(void) {
	
	if ( TSL2591_OK != tsl2591_initialise(task_light_callback)) {
		printf("Light sensor did not initialize successfully\n");

	}
	if(TSL2591_OK != tsl2591_enable()) {
		printf("Light sensor was not enabled successfully\n");

	}
	 xTaskCreate(
		task_read_light,
		"Gathering light readings",
		configMINIMAL_STACK_SIZE,
		NULL,
		2,
		NULL
	);
}
