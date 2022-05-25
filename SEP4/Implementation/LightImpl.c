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


void create_read_light(void);
void task_read_light(void* parameters);
void task_light_callback(tsl2591_returnCode_t rc);


void create_task_light(void) {
	
	
	if ( TSL2591_OK == tsl2591_initialise(task_light_callback)) {
		tsl2591_enable();
		
      xTaskCreate(task_read_light
	   ,"Gathering light readings"
	   ,configMINIMAL_STACK_SIZE
	   ,NULL
	   ,2
	   ,NULL);
}
}
void task_read_light(void* parameters) {
	
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = pdMS_TO_TICKS(300000UL); // 5 minutes
	xLastWakeTime = xTaskGetTickCount();
	
	for(;;) {
		printf("%d\n", xEventGroupGetBits(readingsEventGroup));
		EventBits_t readingsStatus;
		readingsStatus = xEventGroupWaitBits(readingsEventGroup, BIT_CO2, pdTRUE, pdTRUE, portMAX_DELAY);
		
			if ( TSL2591_OK != tsl2591_fetchData()) {
				printf("Something went wrong with the light reading\n" );
				}
		xTaskDelayUntil( &xLastWakeTime, xFrequency );
	}
	
	
}

void task_light_callback(tsl2591_returnCode_t rc) {
	float lux = 0;
	
	if(rc = TSL2591_DATA_READY) {
		if (TSL2591_OK == (rc = tsl2591_getLux(&lux))) {
			if(xQueueSend(xQueue, ( void * ) &lux, 0) == pdPASS) {
				printf("The light measurement is: %5.2f\n", lux);
				xEventGroupSetBits(readingsEventGroup, BIT_LIGHT);
			}
		}
	}
}
