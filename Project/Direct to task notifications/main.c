/* Include library driver */
#include <stdint.h>
#include <stdbool.h>
#include "inc/TM4C123GH6PM.h"
#include "hardware_config.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"

/* Include FreeRTOS library */
#include <FreeRTOS.h>
#include <task.h>
#include "queue.h" 	/* Include Queue */
#include "timers.h" /* Software timer API functions include */

#define DEBUG_

/* The number of indexes in the array of task notifications */
#undef configTASK_NOTIFICATION_ARRAY_ENTRIES
#define configTASK_NOTIFICATION_ARRAY_ENTRIES		(1U)

#ifndef DEBUG_
	/* Priority level of tasks execute */
	#define mainWAKEUP_TASK_PRIORITY		(tskIDLE_PRIORITY + 1)

	/* Frequency blinky of LEDs */
	#define mainFREQUENCY_BLINKY_LEDs		(pdMS_TO_TICKS(300U))
#else
	/* Priority level of tasks execute */
	#define mainSENDER_TASK_PRIORITY		(tskIDLE_PRIORITY + 2)
	#define mainRECEIVE_TASK_PRIORITY		(tskIDLE_PRIORITY + 1)
#endif

/*--------------------------------DECLARE GLOBAL VARIABLES--------------------------------*/
/* The internal clock. This is the value of macro configCPU_CLOCK_HZ which defined in FreeRTOSConfig.h */
uint32_t SystemCoreClock = 16000000U; 
#ifndef DEBUG_
	TaskHandle_t g_xTaskWaken = NULL;
#else
	TaskHandle_t g_xTaskSender  = NULL;
	TaskHandle_t g_xTaskReceive = NULL;
#endif
/*--------------------------------END DECLARE GLOBAL VARIABLES-----------------------------*/

/*--------------------------------DECLARE PROTOTYPE FUNCTION-----------------------------*/
#ifndef DEBUG_
	void vApplicationIdleHook(void);	 	/* Task idle hook prototype */
	void vTaskWakeUp(void *param); 		 	/* Task needs to be waken up from ISR */
#else
	void vDEBUG_TaskReceive(void * param); 	/* Task sends notifications */
	void vDEBUG_TaskSend(void * param);		/* Task receives notifications */
#endif
/*--------------------------------END DECLARE PROTOTYPE FUNCTION---------------------------*/

/* Fucntion handle the interrupt occurs when SW1 pressed */
void WakeUp_Task_ISR_Handler(void){
	GPIOIntClear(GPIO_PORTF_BASE, GPIO_INT_PIN_0); /* Clear interrupt flag */
#ifndef DEBUG_
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	/* Notify the vTaskWakeUp that it will wake up when the ISR is complete */
  vTaskNotifyGiveFromISR(g_xTaskWaken, &xHigherPriorityTaskWoken);
	/* 
	* The use of portYIELD_FROM_ISR(). This is required when waking a task from an interrupt handler. 
	* If vTaskNotifyGiveFromISR indicates that a higher priority task is being woken, the portYIELD_FROM_ISR() 
	* routine will context switch to that task after returning from the ISR.
	*/
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
#endif
} 


int main(){
	hardware_init();
	/* SystemCoreClock as 80MHz */
	SystemCoreClock = SysCtlClockGet();
#ifndef DEBUG_
	xTaskCreate(vTaskWakeUp,				/* The function that implements the task 		*/
				(const char *)"WakeUp", 	/* The text name assigned to the task 			*/
				configMINIMAL_STACK_SIZE, 	/* The size of the stack to allocate to the task 	*/
				NULL, 				/* The parameter passed to the task 			*/
				mainWAKEUP_TASK_PRIORITY,	/* The priority assigned to the task			*/
				&g_xTaskWaken);			/* The task handle 			 		*/
#else
	xTaskCreate(vDEBUG_TaskReceive,				/* The function that implements the task 		*/
				(const char *)"Receive", 	/* The text name assigned to the task 			*/
				configMINIMAL_STACK_SIZE, 	/* The size of the stack to allocate to the task 	*/
				NULL, 				/* The parameter passed to the task  			*/
				mainRECEIVE_TASK_PRIORITY,	/* The priority assigned to the task			*/
				&g_xTaskReceive);		/* The task handle 					*/
				
	xTaskCreate(vDEBUG_TaskSend, (const char *)"Send", configMINIMAL_STACK_SIZE, 
					NULL, mainSENDER_TASK_PRIORITY, &g_xTaskSender);				
#endif
	
	/* Start the tasks and timer running */
	vTaskStartScheduler();
	/* This code following will be never executed */
	for(;;);	
}

#ifndef DEBUG_
	void vApplicationIdleHook(void){
		LEDsOn(hw_cfgLEDsRed);
	}

	void vTaskWakeUp(void *param){
		static uint32_t s_ui32NotificationReceive;
		static uint8_t s_ui8FlagWakeUp = false; 
		TickType_t xLastWakeTime;
		const TickType_t xFrequency = mainFREQUENCY_BLINKY_LEDs;
		/* Initialise the xLastWakeTime variable with the current time */
		xLastWakeTime = xTaskGetTickCount();
		UNUSED(param); /* Avoid warning about unused parameters from compiler */
		for(;;){
			/* If ui8FlagWakeUp is not to be set true, the waiting for notification arrived from ISR is necessary */
			if (!s_ui8FlagWakeUp){
				/* Block indefinitely to wait for a notification until task has received notification */
				s_ui32NotificationReceive = ulTaskNotifyTake(pdTRUE, portMAX_DELAY );
				if (s_ui32NotificationReceive != 0){ /* if task notification has received, change ui32NotificationReceive to true*/
					s_ui8FlagWakeUp = true; /* Change the flag to true to signals this task has been woken up */
				}
			}
			else{
				/* Active blinky LED when this task is woken up from ISR */
				LEDsToggle(hw_cfgLEDsGreen);
				vTaskDelayUntil( &xLastWakeTime, xFrequency);
			}
		}
	}
#else
	void vDEBUG_TaskSend(void * param){
		UNUSED(param); /* Avoid warning about unused parameters from compiler */
		for(;;){
			/* Send notification to task receive using xTaskNotifyGive() 
			xTaskNotifyGive(g_xTaskReceive); 
			xTaskNotifyGive(g_xTaskReceive); 
			xTaskNotifyGive(g_xTaskReceive); 
			*/
			
			/* Send notification to task receive using xTaskNotify() */
			xTaskNotify(g_xTaskReceive, (1 << 0) , eSetValueWithOverwrite);
			/* Block task */
			vTaskDelay(1000);
			xTaskNotify(g_xTaskReceive, (1 << 1) , eSetValueWithOverwrite);
			/* Block task */
			vTaskDelay(1000);
			xTaskNotify(g_xTaskReceive, (1 << 2) , eSetValueWithOverwrite);
			/* Block task */
			vTaskDelay(1000);
		}
	}
	void vDEBUG_TaskReceive(void * param){
		static uint32_t counter = 0;
		uint32_t ui32NotificationReceive; /* Stored the notification value */
		UNUSED(param); /* Avoid warning about unused parameters from compiler */
		for(;;){
			/* Receive the notification using ulTaskNotifyTake()
			ui32NotificationReceive = ulTaskNotifyTake(pdTRUE, portMAX_DELAY); 
			if (ui32NotificationReceive > 0){ // Check the notificatioin
				counter ++; // The number of times the notification was sent to
			}
			*/
			
			/* Receive the notification using xTaskNotifyWait() */
			if (xTaskNotifyWait(0, 0, &ui32NotificationReceive, portMAX_DELAY) == pdTRUE){
				counter++;
			}
		}
	}
	
#endif

	
