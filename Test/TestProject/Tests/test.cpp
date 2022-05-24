#include "gtest/gtest.h"
#include "../Tests/FreeRTOS/FreeRTOS_FFF_MocksDeclaration.h"

// --- Create Mocks ---

extern "C"
{
	#include "../TestProject/OpenWindow.h"
	#include "../Tests/FreeRTOS/FreeRTOS.h"
	#include "../Tests/FreeRTOS/task.h"
	#include "../Tests/FreeRTOS/semphr.h"
	#include "../Tests/FreeRTOS/rc_servo.h"
	#include <stdio.h>
	#include <stdbool.h>
}

class test : public ::testing::Test {
protected:
	void SetUp() override {
		RESET_FAKE(vTaskDelay);
		RESET_FAKE(xSemaphoreGive);
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
}

TEST_F(test, RunTask) {
	task_open_window_run();
	EXPECT_EQ(1, xSemaphoreTake_fake.call_count);
    EXPECT_EQ(0, xSemaphoreGive_fake.call_count);
	EXPECT_EQ(1, vTaskDelayUntil_fake.call_count);
}

