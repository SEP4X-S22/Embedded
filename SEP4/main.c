/*
* main.c
* Author : IHA
*
* Example main file including LoRaWAN setup
* Just for inspiration :)
*/

#include <stdio.h>
#include <avr/io.h>

#include <ATMEGA_FreeRTOS.h>
#include <task.h>
#include <semphr.h>

#include <stdio_driver.h>
#include <serial.h>
#include <TempHumidity.h>

// Needed for LoRaWAN
#include <lora_driver.h>
#include <status_leds.h>
#include <event_groups.h>

//Temp and humidity
#include<hih8120.h>
bool  temp;

// CO2 includes
#include <CO2.h>

// define semaphore handle
QueueHandle_t xQueue;
SemaphoreHandle_t c02Semaphore;
EventGroupHandle_t readingsEventGroup = NULL;
MessageBufferHandle_t downLinkMessageBufferHandle = NULL;


// Prototype for LoRaWAN handler
void lora_handler_initialise(UBaseType_t lora_handler_task_priority);

/*-----------------------------------------------------------*/
void initialiseSystem()
{
	// Set output ports for leds used in the example
	DDRA |= _BV(DDA0) | _BV(DDA7);

	// Make it possible to use stdio on COM port 0 (USB) on Arduino board - Setting 57600,8,N,1
	stdio_initialise(ser_USART0);
	// Let's create some tasks
	create_task_temperature_humidity();
	create_task_c02();
	// vvvvvvvvvvvvvvvvv BELOW IS LoRaWAN initialisation vvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	// Status Leds driver
	status_leds_initialise(5); // Priority 5 for internal task
	
	
	// Initialise the LoRaWAN driver with down-link buffer
	downLinkMessageBufferHandle = xMessageBufferCreate(sizeof(lora_driver_payload_t)*2);
	lora_driver_initialise(1, downLinkMessageBufferHandle);
	
	
	// Create LoRaWAN task and start it up with priority 3
	lora_handler_initialise(3);
	if ( HIH8120_OK == hih8120_initialise() )
	{
		temp = true;
	}
}

/*-----------------------------------------------------------*/
int main(void)
{
	initialiseSystem(); // Must be done as the very first thing!!
	printf("Program Started!!\n");
	xQueue = xQueueCreate( 10, sizeof( unsigned long ) );
	c02Semaphore = xSemaphoreCreateMutex();
	if(c02Semaphore != NULL)
	{
		//xSemaphoreGive(c02Semaphore);
	}
	readingsEventGroup = xEventGroupCreate();
	vTaskStartScheduler(); // Initialise and run the freeRTOS scheduler. Execution should never return from here.
	
	/* Replace with your application code */
	while (1)
	{
	}
}

