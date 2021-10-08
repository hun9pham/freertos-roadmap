/* Include library driver */
#include <stdint.h>
#include <stdbool.h>
#include "inc/TM4C123GH6PM.h"
#include "hardware_config.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "delay.h"

/* Include FreeRTOS library */
#include <FreeRTOS.h>
#include <task.h>
#include "queue.h"
#include "semphr.h"

/* Priorities at which the tasks are created. */
#define mainTASK_SHARED_RESOURCE_UP			(tskIDLE_PRIORITY + 1)
#define mainTASK_SHARED_RESOURCE_DOWN		(tskIDLE_PRIORITY + 2)

/*--------------------------------DECLARE GLOBAL VARIABLES--------------------------------*/

/* The internal clock. This is the value of macro configCPU_CLOCK_HZ which defined in FreeRTOSConfig.h */
uint32_t SystemCoreClock = 16000000U; 
SemaphoreHandle_t g_xMutex = NULL; /* Mutex type semaphore */
volatile uint32_t g_SharedResource = 500; /* LED flashing frequency */
TaskHandle_t g_taskUp = NULL;
TaskHandle_t g_taskDown = NULL;
bool g_flagGreenLED = true;
bool g_flagRedLED = true;
UBaseType_t g_uxPriorityTaskDown;
UBaseType_t g_uxPriorityTaskUp;
/*--------------------------------END DECLARE GLOBAL VARIABLES-----------------------------*/

/*--------------------------------DECLARE PROTOTYPE FUNCTION-----------------------------*/

void vTask_ShareResourceUp(void * param); 	/* Increment the shared resource */
void vTask_ShareResourceDown(void * param); /* Decrement the shared resource */
/*--------------------------------END DECLARE PROTOTYPE FUNCTION---------------------------*/

int main(){
	hardware_init();
	SystemCoreClock = SysCtlClockGet(); /* SystemCoreClock as 80MHz */
	/* Create a mutex type semaphore */
	g_xMutex = xSemaphoreCreateMutex();
	if (g_xMutex != NULL){
		/* Create task which receives message from MessageQueue */
		xTaskCreate(vTask_ShareResourceUp,			/* Function that implements the task */
							(const char *)"Up", 					/* Task name */
							configMINIMAL_STACK_SIZE, 		/* The size of the stack to allocate to the task */
							NULL, 												/* Parameter passed into the task - not used */
							mainTASK_SHARED_RESOURCE_UP,  /* The priority assigned to the task */
							&g_taskUp);												/* Task's handle is not used*/
		/* The similar to task sends message to MessageQueue */
		xTaskCreate(vTask_ShareResourceDown, (const char *)"Down", configMINIMAL_STACK_SIZE, 
													NULL, mainTASK_SHARED_RESOURCE_DOWN, &g_taskDown);
		
		/* Start the tasks and timer running */
		vTaskStartScheduler();
	}
	
	/* This code following will be never executed */
	for(;;);	
}


void vTask_ShareResourceUp(void * param)
{
	
	/* Establish the task's period */
	TickType_t xLastWakeTime;
	while(1){
		/* Take Mutex to acces the shared resource. If Mutex is not available, wait until it avaiable */
		if (xSemaphoreTake(g_xMutex, 0) == pdTRUE)
		{
			g_SharedResource *= 2;
			if (g_flagRedLED == true)
			{
				LEDsOff(hw_cfgLEDsRed);
				LEDsOn(hw_cfgLEDsGreen);
				g_flagGreenLED = true;
				g_flagRedLED = false;
			}
			xSemaphoreGive(g_xMutex); /* Release mutex */
		}		
		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(400)); /* Block task */
	}
}

void vTask_ShareResourceDown(void * param)
{
	g_uxPriorityTaskDown = uxTaskPriorityGet(g_taskDown);
	/* Establish the task's period */
	TickType_t xLastWakeTime;
	while(1){
		/* Take Mutex to acces the shared resource. If Mutex is not available, wait until it avaiable */
		xSemaphoreTake(g_xMutex, portMAX_DELAY);
		g_SharedResource /= 2;
		if (g_flagGreenLED == true){
			LEDsOff(hw_cfgLEDsGreen);
			LEDsOn(hw_cfgLEDsRed);
			g_flagRedLED = true;
			g_flagGreenLED = false;
		}
		xSemaphoreGive(g_xMutex); /* Release mutex */
		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(300)); /* Block task */
	}
}


