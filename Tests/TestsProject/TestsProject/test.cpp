#include "gtest/gtest.h"
#include "FreeRTOS_FFF_MocksDeclaration.h"
#include "../fff/fff.h"

extern "C"
{
	#include "OpenWindow.h"
	#include "ATMEGA_FreeRTOS.h"
	#include "task.h"
	#include "semphr.h"
	#include "rc_servo.h"
	#include <stdio.h>
	#include <stdbool.h>

	//creating mocks for the rc_servo functions
	FAKE_VOID_FUNC(rc_servo_initialise);
	FAKE_VOID_FUNC(rc_servo_setPosition, uint8_t, int8_t);

	//creating mocks for the CO2.h function
	FAKE_VALUE_FUNC(uint16_t, getLatestCO2);

	//creating mocks for the FreeRTOS xTaskDelayUntil
	FAKE_VOID_FUNC(xTaskDelayUntil, TickType_t*, TickType_t);

	//declaring readingsEventGroup as it is used as extern in OpenWindow.h
	EventGroupHandle_t readingsEventGroup = xEventGroupCreate();

	//declaring the boolean from OpenWindow.c as it is needed for some of the tests
	extern bool isWindowOpen;

}

class test : public ::testing::Test {
protected:
	void SetUp() override {
		RESET_FAKE(xTaskDelayUntil);
		RESET_FAKE(xSemaphoreGive);
		RESET_FAKE(rc_servo_initialise);
		RESET_FAKE(rc_servo_setPosition);
		RESET_FAKE(xSemaphoreTake);
		RESET_FAKE(getLatestCO2);
		RESET_FAKE(xEventGroupSetBits);
		RESET_FAKE(xEventGroupWaitBits);
		FFF_RESET_HISTORY();
	}
	void TearDown() override {}
};
/// <summary>
/// The test checks is the xTaskCreate method is called
/// </summary>
TEST_F(test, CreateTask) {
	create_task_open_window();
	EXPECT_EQ(1, xTaskCreate_fake.call_count);
}

/// <summary>
/// The test checks is the initializing method is called
/// </summary>
TEST_F(test, InitTask) {
	xSemaphoreCreateMutex_fake.return_val = (SemaphoreHandle_t)1;
	task_open_window_init();
	EXPECT_EQ(1, xSemaphoreCreateMutex_fake.call_count);
	EXPECT_EQ(1, xSemaphoreGive_fake.call_count);
	EXPECT_EQ(1, rc_servo_initialise_fake.call_count);
}

/// <summary>
/// The test checks is the methods in the run function are called
/// Sets the constraints
/// Sets the CO2 value
/// The CO2 is in the normal range
/// </summary>
TEST_F(test, RunTask) {
	getLatestCO2_fake.return_val = 500;
	xSemaphoreTake_fake.return_val = pdTRUE;
	xEventGroupWaitBits_fake.return_val = pdTRUE;
	setUpperConstraint(600);
	setLowerConstraint(300);
	task_open_window_run();
	EXPECT_EQ(3, xSemaphoreTake_fake.call_count);
	EXPECT_EQ(1, xEventGroupWaitBits_fake.call_count);
	EXPECT_EQ(1, getLatestCO2_fake.call_count);
	EXPECT_EQ(1, xEventGroupSetBits_fake.call_count);
	EXPECT_EQ(3, xSemaphoreGive_fake.call_count);
	EXPECT_EQ(1, xTaskDelayUntil_fake.call_count);
	EXPECT_EQ(0, rc_servo_setPosition_fake.call_count);
}

/// <summary>
/// The test checks is the methods in the run function are called
/// Sets the constraints
/// Sets the CO2 value
/// The CO2 is in the above the upper constraint
/// Window should be opened
/// </summary>
TEST_F(test, OpenWindowAboveUpperConstraint) {
	getLatestCO2_fake.return_val = 760;
	xEventGroupWaitBits_fake.return_val = pdTRUE;
	xSemaphoreTake_fake.return_val = pdTRUE;
	isWindowOpen = false;
	setUpperConstraint(600);
	setLowerConstraint(300);
	task_open_window_run();
	EXPECT_EQ(3, xSemaphoreTake_fake.call_count);
	EXPECT_EQ(1, xEventGroupWaitBits_fake.call_count);
	EXPECT_EQ(1, getLatestCO2_fake.call_count);
	EXPECT_EQ(1, xEventGroupSetBits_fake.call_count);
	EXPECT_EQ(3, xSemaphoreGive_fake.call_count);
	EXPECT_EQ(1, xTaskDelayUntil_fake.call_count);
	EXPECT_EQ(1, rc_servo_setPosition_fake.call_count);
}

/// <summary>
/// The test checks is the methods in the run function are called
/// Sets the constraints
/// Sets the CO2 value
/// The CO2 is in the below the lower constraint
/// Window should be closed
/// </summary>
TEST_F(test, CloseWindowBelowLowerConstraint) {
	getLatestCO2_fake.return_val = 294;
	xEventGroupWaitBits_fake.return_val = pdTRUE;
	xSemaphoreTake_fake.return_val = pdTRUE;
	isWindowOpen = true;
	setUpperConstraint(600);
	setLowerConstraint(300);
	task_open_window_run();
	EXPECT_EQ(3, xSemaphoreTake_fake.call_count);
	EXPECT_EQ(1, xEventGroupWaitBits_fake.call_count);
	EXPECT_EQ(1, getLatestCO2_fake.call_count);
	EXPECT_EQ(1, xEventGroupSetBits_fake.call_count);
	EXPECT_EQ(3, xSemaphoreGive_fake.call_count);
	EXPECT_EQ(1, xTaskDelayUntil_fake.call_count);
	EXPECT_EQ(1, rc_servo_setPosition_fake.call_count);
}

/// <summary>
/// The test checks is the methods in the run function are called
/// Sets the constraints
/// Sets the CO2 value
/// Window has already been open
/// The CO2 is in the above the upper constraint
/// Window should stay open
/// </summary>
TEST_F(test, DoNothingAboveUpperConstraint) {
	getLatestCO2_fake.return_val = 785;
	xEventGroupWaitBits_fake.return_val = pdTRUE;
	xSemaphoreTake_fake.return_val = pdTRUE;
	isWindowOpen = true;
	setUpperConstraint(600);
	setLowerConstraint(300);
	task_open_window_run();
	EXPECT_EQ(3, xSemaphoreTake_fake.call_count);
	EXPECT_EQ(1, xEventGroupWaitBits_fake.call_count);
	EXPECT_EQ(1, getLatestCO2_fake.call_count);
	EXPECT_EQ(1, xEventGroupSetBits_fake.call_count);
	EXPECT_EQ(3, xSemaphoreGive_fake.call_count);
	EXPECT_EQ(1, xTaskDelayUntil_fake.call_count);
	EXPECT_EQ(0, rc_servo_setPosition_fake.call_count);
}
/// <summary>
/// The test checks is the methods in the run function are called
/// Sets the constraints
/// Sets the CO2 value
/// Window has already been closed
/// The CO2 is in the below the lower constraint
/// Window should stay closed
/// </summary>
TEST_F(test, DoNothingBelowLowerConstraint) {
	getLatestCO2_fake.return_val = 162;
	xEventGroupWaitBits_fake.return_val = pdTRUE;
	xSemaphoreTake_fake.return_val = pdTRUE;
	isWindowOpen = false;
	setUpperConstraint(600);
	setLowerConstraint(300);
	task_open_window_run();
	EXPECT_EQ(3, xSemaphoreTake_fake.call_count);
	EXPECT_EQ(1, xEventGroupWaitBits_fake.call_count);
	EXPECT_EQ(1, getLatestCO2_fake.call_count);
	EXPECT_EQ(1, xEventGroupSetBits_fake.call_count);
	EXPECT_EQ(3, xSemaphoreGive_fake.call_count);
	EXPECT_EQ(1, xTaskDelayUntil_fake.call_count);
	EXPECT_EQ(0, rc_servo_setPosition_fake.call_count);
}

