/* Include library driver */
#include <stdint.h>
#include <stdbool.h>
#include "inc/TM4C123GH6PM.h"
#include "delay.h"
#include "hardware_config.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
//#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"

/* Include FreeRTOS library */
#include <FreeRTOS.h>
#include <task.h>

/* Priorities at which the tasks are created. */
#define mainBLUE_BLINKY_TASK_PRIORITY		(tskIDLE_PRIORITY + 2)
#define mainGREEN_BLINKY_TASK_PRIORITY		(tskIDLE_PRIORITY + 1)


#define mainLED_BLINKY_FREQUENCY_MS			pdMS_TO_TICKS(250UL)

/*--------------------------------DECLARE GLOBAL VARIABLES--------------------------------*/
uint32_t SystemCoreClock = 16000000;
TaskHandle_t g_Handle_BlueTask = NULL;  /* Point to vTask_BlueBlinky */
TaskHandle_t g_Handle_GreenTask = NULL; /* Point to vTask_GreenBlinky */
uint32_t g_ui32Counter = 0; /* Increment of this variable will delete or create a task */
/*--------------------------------END DECLARE GLOBAL VARIABLES-----------------------------*/


/*--------------------------------DECLARE PROTOTYPE FUNCTION-----------------------------*/
/* Task 1 - Blue Led Blinky */
void vTask_BlueBlinky(void * param);

/* Task 2 - Green Led Blinky */
void vTask_GreenBlinky(void * param);
/*--------------------------------END DECLARE PROTOTYPE FUNCTION---------------------------*/


int main(){
	uint8_t i;
	hardware_init();
	/* SystemCoreClock as 80MHz */
	SystemCoreClock = SysCtlClockGet();
	/* 
	* RTOS create vTask_BlueBlinky
	* If the creation task fails, the red led will signal by blinking three times after 250ms 
	*/
	BaseType_t xTaskReturn = xTaskCreate(vTask_BlueBlinky, /* Function that implements the task */
									(const char *)"BlueTask", /* Text name for the task is "BlueTask" */
									configMINIMAL_STACK_SIZE, /* The size of the stack to allocate to the task */
									NULL, /* Parameter passed into the task - not used */
									mainBLUE_BLINKY_TASK_PRIORITY,/* The priority assigned to the task */
									&g_Handle_BlueTask); /* Task's handle */
	if (xTaskReturn != pdPASS){ /* Check the creation task */
		for (i = 0; i < 3; i++){
			LEDsOn(LED_RED);
			DelayMS(250);
			LEDsOff(LED_RED);
			DelayMS(250);
		}
	}
	/* 
	* RTOS create vTask_GreenBlinky
	* If the creation task fails, the red led will signal by blinking two times after 500ms 
	* This creation task is similar to the creation task above
	*/
	xTaskReturn = xTaskCreate(vTask_GreenBlinky, (const char *)"GreenTask", configMINIMAL_STACK_SIZE, 
								NULL, mainGREEN_BLINKY_TASK_PRIORITY, &g_Handle_GreenTask);
	if (xTaskReturn != pdPASS){ /* Check the creation task */
		for (i = 0; i < 2; i++){
			LEDsOn(LED_RED);
			DelayMS(500);
			LEDsOff(LED_RED);
			DelayMS(500);
		}
	}
	/* Start the tasks and timer running */
	vTaskStartScheduler();
	/* This code following will be never executed */
	for(;;);	
}

void vTask_BlueBlinky(void * param){
	static uint32_t s_ui32PreviousCounter = 0;
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = mainLED_BLINKY_FREQUENCY_MS;
	xLastWakeTime = xTaskGetTickCount(); /* Initialise the xLastWakeTime variable with the current time */
	UNUSED(param); /* Avoid warning unused parameters from compilers */
	for(;;){
		LEDsOn(LED_BLUE);
		vTaskDelayUntil(&xLastWakeTime, xFrequency);
		LEDsOff(LED_BLUE);
		vTaskDelayUntil(&xLastWakeTime, xFrequency);
		++g_ui32Counter;
		if (g_ui32Counter - s_ui32PreviousCounter == 10U && g_Handle_GreenTask != NULL){
			LEDsOff(LED_GREEN | LED_BLUE); /* Turn off two LEDs */
			vTaskDelete(g_Handle_GreenTask);	
			g_Handle_GreenTask = NULL;
			s_ui32PreviousCounter = g_ui32Counter;
		}
		else if (g_ui32Counter - s_ui32PreviousCounter == 5U && g_Handle_GreenTask == NULL){
			xTaskCreate(vTask_GreenBlinky, (const char *)"GreenTask", configMINIMAL_STACK_SIZE, 
											NULL, mainGREEN_BLINKY_TASK_PRIORITY, &g_Handle_GreenTask);
		}
	}
}

void vTask_GreenBlinky(void * param){
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = mainLED_BLINKY_FREQUENCY_MS;
	xLastWakeTime = xTaskGetTickCount(); /* Initialise the xLastWakeTime variable with the current time */
	UNUSED(param); /* Avoid warning unused parameters from compilers */
	for(;;){
		LEDsOn(LED_GREEN);
		vTaskDelayUntil(&xLastWakeTime, xFrequency);
		LEDsOff(LED_GREEN);
		vTaskDelayUntil(&xLastWakeTime, xFrequency);
	}
}
