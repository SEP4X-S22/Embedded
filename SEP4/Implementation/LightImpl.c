#include <ATMEGA_FreeRTOS.h>
#include <task.h>
#include <stdio.h>

#include <tsl2591.h>
#include <event_groups.h>
#include <semphr.h>

#define BIT_CO2 (1 << 2)
#define BIT_LIGHT (1 << 3)

extern EventGroupHandle_t readingsEventGroup;
extern QueueHandle_t xQueue;

uint16_t lastLightValue = 0;

void task_read_light(void *pvparameters) {
	
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = pdMS_TO_TICKS(300000UL); // 5 minutes
	xLastWakeTime = xTaskGetTickCount();
	
	for(;;) {
		EventBits_t readingsStatus;
		readingsStatus = xEventGroupWaitBits(readingsEventGroup,BIT_CO2, pdTRUE, pdTRUE, portMAX_DELAY);
		
		if ( TSL2591_OK != tsl2591_fetchData()) {
			printf("Something went wrong with the light reading\n" );
		}
		xTaskDelayUntil( &xLastWakeTime, xFrequency );
	}
}

void task_light_callback(tsl2591_returnCode_t rc) {
	float lux = 0;
	
	if(rc == TSL2591_DATA_READY) {
		if (TSL2591_OK == (rc = tsl2591_getLux(&lux))) {
				lastLightValue = lux;
				printf("The light measurement is %5.2f lux\n", lux);
				xEventGroupSetBits(readingsEventGroup, BIT_LIGHT);
			}
		}
}

uint16_t getLatestLight() {
	return lastLightValue;
}

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
