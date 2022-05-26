#include "gtest/gtest.h"
#include "../Tests/FreeRTOS/FreeRTOS_FFF_MocksDeclaration.h"
#include "../fff/fff.h"

extern "C"
{
	#include "../TestProject/OpenWindow.h"
	#include "../Tests/FreeRTOS/FreeRTOS.h"
	#include "../Tests/FreeRTOS/task.h"
	#include "../Tests/FreeRTOS/semphr.h"
	#include "../../../SEP4/drivers/rc_servo.h"
	#include <stdio.h>
	#include <stdbool.h>
	//creating mocks for the rc_servo functions
	//created here, as it was throwing exceptions if outside
	FAKE_VOID_FUNC(rc_servo_initialise);
	FAKE_VOID_FUNC(rc_servo_setPosition, uint8_t, int8_t);
	FAKE_VALUE_FUNC(uint16_t, getLatestCO2);
	EventGroupHandle_t readingsEventGroup = xEventGroupCreate();
	
}

class test : public ::testing::Test {
protected:
	void SetUp() override {
		RESET_FAKE(vTaskDelayUntil);
		RESET_FAKE(xSemaphoreGive);
		RESET_FAKE(rc_servo_initialise);
		RESET_FAKE(rc_servo_setPosition);
		RESET_FAKE(xSemaphoreTake);
		RESET_FAKE(getLatestCO2);
		FFF_RESET_HISTORY();
	}

	void TearDown() override {}
};

TEST_F(test, CreateTask) {
	create_task_open_window();
	EXPECT_EQ(1, xTaskCreate_fake.call_count);
}

TEST_F(test, InitTask) {
	task_open_window_init();
	EXPECT_EQ(1, xSemaphoreCreateMutex_fake.call_count);
	EXPECT_EQ(1, xSemaphoreGive_fake.call_count);
	EXPECT_EQ(1, rc_servo_initialise_fake.call_count);
}

TEST_F(test, RunTask) {
	task_open_window_run();
	EXPECT_EQ(1, xSemaphoreTake_fake.call_count);
	EXPECT_EQ(1, vTaskDelayUntil_fake.call_count);
}

TEST_F(test, OpenWindowNotChangedConstrint) {
	getLatestCO2_fake.return_val = 50;
	openCloseWindow();
	EXPECT_EQ(1, xSemaphoreGive_fake.call_count);
	EXPECT_EQ(0, rc_servo_setPosition_fake.call_count);
}

TEST_F(test, OpenWindowChangedConstrint) {
	getLatestCO2_fake.return_val = 101;
	openCloseWindow();
	EXPECT_EQ(1, xSemaphoreGive_fake.call_count);
	EXPECT_EQ(1, rc_servo_setPosition_fake.call_count);
}
