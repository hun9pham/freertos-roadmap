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
#include "queue.h"

/* Priorities at which the tasks are created. */
#define mainTASK_RECEIVER_PRIORITY				(tskIDLE_PRIORITY + 2)
#define mainTASK_SENDER_PRIORITY				(tskIDLE_PRIORITY + 1)

/* Define length of MessageQueue and size of item in MessageQueue */
#define mainMESSAGE_QUEUE_LENGTH				(3U)
#define mainMESSAGE_QUEUE_ITEM_SIZE			(sizeof(char))
	
/* The maximum amount of time the task will block when waiting for avaiable on MessageQueue */
#define mainSENDER_TICK_TO_WAIT					((TickType_t) 10)
#define mainRECEIVER_TICK_TO_WAIT				((TickType_t)  0)

/*--------------------------------DECLARE GLOBAL VARIABLES--------------------------------*/

/* The internal clock. This is the value of macro configCPU_CLOCK_HZ which defined in FreeRTOSConfig.h */
uint32_t SystemCoreClock = 16000000U; 
QueueHandle_t g_xMessageQueue = NULL; /* Messages sent will be kept MessageQueue  */
const char g_cMessageLetterArray[] = {'R', 'B', 'G'}; /* Array holds letter messages that task 2 will send to MessageQueue */

/*--------------------------------END DECLARE GLOBAL VARIABLES-----------------------------*/

/*--------------------------------DECLARE PROTOTYPE FUNCTION-----------------------------*/

void vTask_Receiver(void * param); /* This task always receives message from MessageQueue */
void vTask_Sender(void * param);   /* This task always sends message to MessageQueue */

/*--------------------------------END DECLARE PROTOTYPE FUNCTION---------------------------*/

int main(){
	hardware_init();
	SystemCoreClock = SysCtlClockGet(); /* SystemCoreClock as 80MHz */
	/* Create MessageQueue */
	g_xMessageQueue = xQueueCreate(mainMESSAGE_QUEUE_LENGTH, mainMESSAGE_QUEUE_ITEM_SIZE);
	if (g_xMessageQueue != NULL){ /* If MessageQueue created successfully, OS kernel will create two task */
		/* Create task which receives message from MessageQueue */
		xTaskCreate(vTask_Receiver, 		/* Function that implements the task */
			(const char *)"Receiver", 	/* Task name */
			configMINIMAL_STACK_SIZE, 	/* The size of the stack to allocate to the task */
			NULL, 				/* Parameter passed into the task - not used */
			mainTASK_RECEIVER_PRIORITY,     /* The priority assigned to the task */
			NULL);				/* Task's handle is not used*/
		/* The similar to task sends message to MessageQueue */
		xTaskCreate(vTask_Sender, (const char *)"Sender", configMINIMAL_STACK_SIZE, 
													NULL, mainTASK_SENDER_PRIORITY, NULL);
		
		/* Start the tasks and timer running */
		vTaskStartScheduler();
	}
	else { /* Insufficient memory allocated for MessageQueue */
	}
	
	/* This code following will be never executed */
	for(;;);	
}

void vTask_Receiver(void * param){
	/* uxMessageExit indicates the number of messages available in the MessageQueue */
	UBaseType_t uxMessageExist;
	/* Message receive from MessageQueue will be stored in this variable */
	char cLetterReceiv; 
	/* Block for 300ms */
	const TickType_t xDelay = 300 / portTICK_PERIOD_MS;
	UNUSED(param); /* Avoid warning from compiler about unused parameter */
	for(;;){
		uxMessageExist = uxQueueMessagesWaiting(g_xMessageQueue);
		if (uxMessageExist > 0U){
			xQueueReceive(g_xMessageQueue, &(cLetterReceiv), 0U);
			switch(cLetterReceiv){ /* cLetterReceiv now contains a copy of message */
				case 'R':{ /* Red led on - Blue and green offf */
					LEDsOn(LED_RED);
					LEDsOff(LED_BLUE | LED_GREEN);
					break;
				}
				case 'B':{ /* Blue led on - Red and green offf */
					LEDsOn(LED_BLUE);
					LEDsOff(LED_RED | LED_GREEN);
					break;
				}
				case 'G':{ /* Green led on - Blue and red offf */
					LEDsOn(LED_GREEN);
					LEDsOff(LED_BLUE | LED_RED);
					break;
				}
			}
		}
		vTaskDelay(xDelay); /* Task blocking */
	}
}


void vTask_Sender(void * param){
	static uint8_t s_ui8LetterIndex = 0; /* Indicate index item in g_cMessageLetterArray will be sent to MessageQueue */
	UNUSED(param); /* Avoid warning from compiler about unused parameter */
	for(;;){
		if (xQueueSendToBack(g_xMessageQueue, (void *)&g_cMessageLetterArray[s_ui8LetterIndex],
								mainSENDER_TICK_TO_WAIT) == pdTRUE) /* MessageQueue is not full */
		{
			/* next letter message in g_xMessageLetterArray will be sent for the next time */
			if (s_ui8LetterIndex == (mainMESSAGE_QUEUE_LENGTH - 1)){
				s_ui8LetterIndex = 0U;
			}
			else{
				++(s_ui8LetterIndex);
			}
		}
		else{ /* if MessageQueue is full, task sender will be blocked after mainSENDER_TICK_TO_WAIT */
		}
	}
}
