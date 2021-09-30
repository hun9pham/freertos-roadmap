/* Include library driver */
#include <stdint.h>
#include <stdbool.h>
#include "inc/TM4C123GH6PM.h"
#include "hardware_config.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
//#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"

/* Include FreeRTOS library */
#include <FreeRTOS.h>
#include <task.h>
#include "queue.h" 	/* Include Queue */
#include "timers.h" /* Software timer API functions include */

/* Define lenght and size of item in Queue */
#define mainQUEUE_LENGTH					(2U)
#define mainQUEUE_ITEM_SIZE					sizeof(uint32_t)
	
/* Priority level of tasks execute */
#define mainQUEUE_RECEIVE_TASK_PRIORITY				(tskIDLE_PRIORITY + 2)
#define	mainQUEUE_SEND_TASK_PRIORITY				(tskIDLE_PRIORITY + 1)

/* Duration which task and timer send data to queue in milliseconds */
#define mainTASK_SEND_FREQUENCY_MS				(pdMS_TO_TICKS(200U))
#define mainTIMER_SEND_FREQUENCY_MS				(pdMS_TO_TICKS(2000U))

/* Values will be sent to Queue. Task receive will rely on this value to determine sent from task send or timer */
#define mainVALUE_SEND_FROM_TASK				(100U)
#define mainVALUE_SEND_FROM_TIMER				(200U)

/* Duration for blinking LED depends on received value  */
#define mainTASK_BLINK_DELAY_MS					(pdMS_TO_TICKS(20U))
#define mainTIMER_BLINK_DELAY_MS				(pdMS_TO_TICKS(200U))

/*--------------------------------DECLARE GLOBAL VARIABLES--------------------------------*/
/* The internal clock. This is the value of macro configCPU_CLOCK_HZ which defined in FreeRTOSConfig.h */
uint32_t SystemCoreClock = 16000000U; 
QueueHandle_t g_xQueue = NULL; /* Queue hold value and used by both task */
TimerHandle_t g_xTimer = NULL; /* Software Timer instance */
/*--------------------------------END DECLARE GLOBAL VARIABLES-----------------------------*/

/*--------------------------------DECLARE PROTOTYPE FUNCTION-----------------------------*/
void QueueSendTask(void *param); 			/* Task 1 - Send value to Queue */
void QueueReceiveTask(void *param); 			/* Task 2 - Receive value from Queue */
void QueueSendTimerCallback(TimerHandle_t xTimer); 	/* Software timer callback function */
/*--------------------------------END DECLARE PROTOTYPE FUNCTION---------------------------*/

/* Fucntion handle the interrupt occurs when SW1 pressed */
void SwitchHandler(void){
	GPIOIntClear(GPIO_PORTF_BASE, GPIO_INT_PIN_0); /* Clear interrupt flag */
 	/* 
	* When SW1 is pressed, the spike voltage will appear (Switch bouncing problem). The spike voltage will 
	* have a huge effective to Software Timer's reset (https://www.freertos.org/Resetting-an-RTOS-software-timer.html)
	* So disabling the interrupt is necessary when entering the interrupt handler function and allowing the 
	* interrupt to work again so the software timer completes its task. 
	*/
	GPIOIntDisable(GPIO_PORTF_BASE, GPIO_INT_PIN_0); /* Disable interrupt on GPIOF pin 0 (SW1) */
	/* Attempt a wake a higher priority task to continue execution */	
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xTimerResetFromISR(g_xTimer, &xHigherPriorityTaskWoken); /* Reset the timer when SW1 has been pressed */
 	/* 
	* If a higher priority was successfully woken, then yield execution to it
	* and go to that task (instead of changing context to another task) 
	*/
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
} 

int main(){
	const uint32_t xTimerPeriod = pdMS_TO_TICKS(2000); /* 2 seconds */
	hardware_init();
	/* SystemCoreClock as 80MHz */
	SystemCoreClock = SysCtlClockGet();
	/* Create xQueue */
	g_xQueue = xQueueCreate(mainQUEUE_LENGTH , mainQUEUE_ITEM_SIZE);
	if (g_xQueue != NULL){
		/* Create tasks */
		xTaskCreate(QueueReceiveTask,					/* The function that implements the task */
					(const char *)"Receive", 		/* The text name assigned to the task */
					configMINIMAL_STACK_SIZE, 		/* The size of the stack to allocate to the task */
					NULL, 					/* The parameter passed to the task */
					mainQUEUE_RECEIVE_TASK_PRIORITY,	/* The priority assigned to the task */
					NULL );					/* The task handle */
		xTaskCreate(QueueSendTask, (const char *)"Send", configMINIMAL_STACK_SIZE, NULL, mainQUEUE_SEND_TASK_PRIORITY, NULL );
		/* Create the software timer, but don't start it yet. */
		g_xTimer = xTimerCreate("Timer",			/* The text name assigned to the Software Timer */
					xTimerPeriod,			/* The period of the software timer in ticks */
					pdFALSE,			/* pdFALSE for one-shot type, pdTRUE for auto-reload type */
					NULL,				/* The timer's ID */
					QueueSendTimerCallback);     	/* The function executed when the timer expires */
		/* Start the tasks and timer running */
		vTaskStartScheduler();
	}
	/* This code following will be never executed */
	for(;;);	
}

void QueueSendTask(void *param){
	UBaseType_t uQueueStatus; /* Check space available in queue */
	const UBaseType_t ui8SpaceAvailable = 1U; /* if queue has one slot not be used, this is space available on queue */
	const uint32_t ui32ValueSend = mainVALUE_SEND_FROM_TASK;
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = mainTASK_SEND_FREQUENCY_MS;
	/* Initialise the xLastWakeTime variable with the current time */
  xLastWakeTime = xTaskGetTickCount();
	UNUSED(param); /* Avoid warning about unused parameters from compiler */
	while (1){
		uQueueStatus = uxQueueSpacesAvailable(g_xQueue);
		if (uQueueStatus >= ui8SpaceAvailable){ /* Queue is not full */
			xQueueSendToBack(g_xQueue, &(ui32ValueSend),(TickType_t) 0U);
		}
		vTaskDelayUntil(&xLastWakeTime, xFrequency);
	}
}

void QueueReceiveTask(void *param){
	uint32_t ui32ValueReceive; /* The value receive from queue will be stored in this variable */
	UNUSED(param); /* Avoid warning about unused parameters from compiler */
	while (1){
		/* Wait until value arrives in the queue */
		xQueueReceive(g_xQueue, &(ui32ValueReceive), portMAX_DELAY);
		if (ui32ValueReceive == mainVALUE_SEND_FROM_TASK){ /* Receive value from task */
			/*
			LEDsOn(LED_GREEN);
			vTaskDelay(mainTASK_BLINK_DELAY_MS);
			LEDsOff(LED_GREEN);
			*/
			LEDsToggle(LED_GREEN);
			vTaskDelay(mainTASK_BLINK_DELAY_MS);
		}
		else if (ui32ValueReceive == mainVALUE_SEND_FROM_TIMER){ /* Receive value from timer */
			LEDsOn(LED_RED);
			vTaskDelay(mainTIMER_BLINK_DELAY_MS);
			LEDsOff(LED_RED);
			/* Enable SW1 interrupt when timer callback function finished */
			GPIOIntEnable(GPIO_PORTF_BASE, GPIO_INT_PIN_0);
		}
	}
}

void QueueSendTimerCallback(TimerHandle_t xTimer){
	/* 
	* This is the software timer callback function.  The software timer has a
	* period of two seconds and is reset each time a SW1 is pressed. This callback 
	* function will execute if the timer expires, which will only happen if a
  * key is not pressed for two seconds. 
	*/
	const uint32_t ui32ValueSend = mainVALUE_SEND_FROM_TIMER;
	UNUSED(xTimer);
	xQueueSendToBack(g_xQueue, &(ui32ValueSend),(TickType_t) 0U);
}



