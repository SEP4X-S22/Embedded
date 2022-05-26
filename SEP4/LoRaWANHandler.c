/*
* loraWANHandler.c
*
* Created: 12/04/2019 10:09:05
*  Author: IHA
* The backbone functionality of the project is taken from Ib Havn's repository https://github.com/ihavn/IoT_Semester_project
*/
#include <stddef.h>
#include <stdio.h>

#include <ATMEGA_FreeRTOS.h>

#include <lora_driver.h>
#include <status_leds.h>
#include <queue.h>
#include "event_groups.h"

// Parameters for OTAA join
#define LORA_appEUI "9226119BAA2DE982"
#define LORA_appKEY "65DE3D06F8D11CAA807EE317C60E144D"

//Event group bit indicating that all internal tasks were done and it is ready for transmission
#define BIT_COMPLETE (1 << 4)

extern QueueHandle_t xQueue;
extern EventGroupHandle_t readingsEventGroup;
//Message buffer for the downlink messages
extern MessageBufferHandle_t downLinkMessageBufferHandle;

//Tasks' prototypes
void lora_handler_task( void *pvParameters );
void lora_downlink_handler_task(void *pvParameters);

//Payload for the sensors' readings that is going to be sent uplink
static lora_driver_payload_t _uplink_payload;
//Payload that is received downlink to set the constraints for recommended CO2 levels
static lora_driver_payload_t downlinkPayload;

//Initializing both uplink and downlink tasks
void lora_handler_initialise(UBaseType_t lora_handler_task_priority)
{
	xTaskCreate(
	lora_handler_task
	,  "LRHand"
	,  configMINIMAL_STACK_SIZE+200  
	,  NULL
	,  lora_handler_task_priority
	,  NULL );
	xTaskCreate(
	lora_downlink_handler_task,
	"DownlinkTask",
	configMINIMAL_STACK_SIZE+200,
	NULL,
	lora_handler_task_priority,
	NULL);
}

static void _lora_setup(void)
{
	char _out_buf[20];
	lora_driver_returnCode_t rc;
	// Green led blinking slowly while LoRa is being setup
	status_leds_slowBlink(led_ST2);

	// Factory reset the transceiver
	printf("FactoryReset >%s<\n", lora_driver_mapReturnCodeToText(lora_driver_rn2483FactoryReset()));
	
	// Configure to EU868 LoRaWAN standards
	printf("Configure to EU868 >%s<\n", lora_driver_mapReturnCodeToText(lora_driver_configureToEu868()));

	// Get the transceivers HW EUI
	rc = lora_driver_getRn2483Hweui(_out_buf);
	printf("Get HWEUI >%s<: %s\n",lora_driver_mapReturnCodeToText(rc), _out_buf);

	// Set the HWEUI as DevEUI in the LoRaWAN software stack in the transceiver
	printf("Set DevEUI: %s >%s<\n", _out_buf, lora_driver_mapReturnCodeToText(lora_driver_setDeviceIdentifier(_out_buf)));

	// Set Over The Air Activation parameters to be ready to join the LoRaWAN
	printf("Set OTAA Identity appEUI:%s appKEY:%s devEUI:%s >%s<\n", LORA_appEUI, LORA_appKEY, _out_buf, lora_driver_mapReturnCodeToText(lora_driver_setOtaaIdentity(LORA_appEUI,LORA_appKEY,_out_buf)));

	// Save all the MAC settings in the transceiver
	printf("Save mac >%s<\n",lora_driver_mapReturnCodeToText(lora_driver_saveMac()));

	// Enable Adaptive Data Rate
	printf("Set Adaptive Data Rate: ON >%s<\n", lora_driver_mapReturnCodeToText(lora_driver_setAdaptiveDataRate(LORA_ON)));

	// Set receiver window delay to 500 ms - this is needed if down-link messages will be used
	printf("Set Receiver Delay: %d ms >%s<\n", 500, lora_driver_mapReturnCodeToText(lora_driver_setReceiveDelay(500)));

	// Join the LoRaWAN
	uint8_t maxJoinTriesLeft = 10;
	
	do {
		rc = lora_driver_join(LORA_OTAA);
		printf("Join Network TriesLeft:%d >%s<\n", maxJoinTriesLeft, lora_driver_mapReturnCodeToText(rc));

		if ( rc != LORA_ACCEPTED)
		{
			// Make the red led pulse to tell something went wrong
			status_leds_longPuls(led_ST1);
			// Wait 5 sec and try again to connect
			vTaskDelay(pdMS_TO_TICKS(5000UL));
		}
		else
		{
			break;
		}
	} while (--maxJoinTriesLeft);

	if (rc == LORA_ACCEPTED)
	{
		// Connected to LoRaWAN :-)
		// Make the green led steady
		status_leds_ledOn(led_ST2);
	}
	else
	{
		// Turn off the green led when something went wrong
		status_leds_ledOff(led_ST2); 
		// Make the red led blink fast to tell something went wrong
		status_leds_fastBlink(led_ST1); 

		while (1)
		{
			taskYIELD();
		}
	}
}

void lora_downlink_handler_task(void *pvParameters) {
		
		
	for(;;) {
			//Awaiting for a payload from the downlink message buffer
			xMessageBufferReceive(downLinkMessageBufferHandle, &downlinkPayload, sizeof(lora_driver_payload_t), portMAX_DELAY);
			//Printing out the port and length of the received payload
			printf("DOWN LINK: from port: %d with %d bytes received!\n", downlinkPayload.portNo, downlinkPayload.len);
			//If the payload is of expected length, proceed
			if(4 == downlinkPayload.len) {
				//Variables to store CO2 upper and lower bound constraints
				uint16_t co2LowerBound;
				uint16_t co2UpperBound;
				
				//Extracting the constraints from the payload and defining the variables
				co2LowerBound = (downlinkPayload.bytes[0] << 8) + downlinkPayload.bytes[1];
				co2UpperBound = (downlinkPayload.bytes[2] << 8) + downlinkPayload.bytes[3];
				
				//Printing out the values within the variables
				printf("%04d\n", co2LowerBound);
				printf("%04d\n", co2UpperBound);
				
				//Setting the new constraints based on which Servo will act accordingly
				setLowerConstraint(co2LowerBound);
				setUpperConstraint(co2UpperBound);
			}
			
			
	}
}


/*-----------------------------------------------------------*/
void lora_handler_task( void *pvParameters )
{
	// Hardware reset of LoRaWAN transceiver
	lora_driver_resetRn2483(1);
	vTaskDelay(2);
	lora_driver_resetRn2483(0);
	
	// Giving it a change to wake up
	vTaskDelay(150);

	// Getting rid of the first version string from module after reset!
	lora_driver_flushBuffers();

	_lora_setup();
	
	//Defining the length and port of the uplink payload
	_uplink_payload.len = 7;
	_uplink_payload.portNo = 1;

	TickType_t xLastWakeTime;
	const TickType_t xFrequency = pdMS_TO_TICKS(300000UL); // 5 minutes
	xLastWakeTime = xTaskGetTickCount();
	
	for(;;)
	{
		//Variable for storing the result of the event group
		EventBits_t readingsStatus;
		
		//Readings' declaration
		float p;
		int16_t temp = 0;
		uint8_t hum = 0;
		uint16_t co2_ppm = 0;
		uint16_t light_lux = 0;
		
		//Waiting for the COMPLETE bit to be set, indicating that it is time for transmission
		readingsStatus = xEventGroupWaitBits(readingsEventGroup, BIT_COMPLETE, pdTRUE, pdTRUE, portMAX_DELAY);
		
		//Getting the readings
			if(xQueueReceive(xQueue, &p, 0) == pdPASS) temp = (p*10);
			if(xQueueReceive(xQueue, &p, 0) == pdPASS) hum = p;
			co2_ppm = getLatestCO2();
			light_lux = getLatestLight();
			
		
		puts("Sending to LoRaWAN");

		//Populating the payload with the latest readings
		_uplink_payload.bytes[0] = hum;
		_uplink_payload.bytes[1] = temp >> 8;
		_uplink_payload.bytes[2] = temp & 0xFF;
		_uplink_payload.bytes[3] = co2_ppm >> 8;
		_uplink_payload.bytes[4] = co2_ppm & 0xFF;
		_uplink_payload.bytes[5] = light_lux >> 8;
		_uplink_payload.bytes[6] = light_lux & 0xFF;
		

		//Sending the payload uplink
		printf("Upload Message >%s<\n", lora_driver_mapReturnCodeToText(lora_driver_sendUploadMessage(false, &_uplink_payload)));
		
		xTaskDelayUntil( &xLastWakeTime, xFrequency );
	}
}