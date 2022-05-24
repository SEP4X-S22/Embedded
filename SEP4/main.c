/*
* The backbone functionality of the project is taken from Ib Havn's repository https://github.com/ihavn/IoT_Semester_project
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

// CO2 includes
#include <CO2.h>

//Indicator whether humidity and temperature sensor's setup was successful
bool  temp;
//Queue into which the readings from sensors are put
QueueHandle_t xQueue;
//Event group for the tasks' order management
EventGroupHandle_t readingsEventGroup = NULL;
//Message buffer for downlink messages
MessageBufferHandle_t downLinkMessageBufferHandle = NULL;


// Prototype for LoRaWAN handler
void lora_handler_initialise(UBaseType_t lora_handler_task_priority);

/*-----------------------------------------------------------*/
void initialiseSystem()
{

	// Make it possible to use stdio on COM port 0 (USB) on Arduino board - Setting 57600,8,N,1
	stdio_initialise(ser_USART0);
	
	// Creating the tasks for sensors and the servo
	create_task_temperature_humidity();
	create_task_open_window();
	create_task_c02();
	
	// Initializing LoRaWAN status leds
	status_leds_initialise(5);
	
	
	// Initializing the LoRaWAN driver with a downlink buffer
	downLinkMessageBufferHandle = xMessageBufferCreate(sizeof(lora_driver_payload_t)*2);
	lora_driver_initialise(1, downLinkMessageBufferHandle);
	
	
	// Setting up LoRaWAN tasks for uplink and downlink operations
	lora_handler_initialise(3);
	
	//Verifying that the humidity and temperature setup went as planned
	if ( HIH8120_OK == hih8120_initialise() )
	{
		temp = true;
	}
}

int main(void)
{
	// Setting up the sensors and the LoRaWAN handler that is responsible for the readings' transmission
	initialiseSystem();
	printf("Program Started!!\n");
	
	//Defining the queue
	xQueue = xQueueCreate( 10, sizeof( unsigned long ) );
	
	//Defining the event group
	readingsEventGroup = xEventGroupCreate();
	
	//Handing control over to the FreeRTOS
	vTaskStartScheduler();
}

