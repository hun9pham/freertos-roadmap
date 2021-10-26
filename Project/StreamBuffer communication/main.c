#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "tm4c_lib/tm4c_lib.h"

/* Include FreeRTOS library */
#include <FreeRTOS.h>
#include <task.h>
#include "stream_buffer.h"

/* Avoid warning about unused parameters from compiler */
#define UNUSED(parameter)				do { (void)(parameter); }while(0);

/* Define pin, port */
#define mainREDLED					PINDEF(PORTF, PIN1)
#define mainGREENLED					PINDEF(PORTF, PIN3)

/* Priorities at which the tasks are created. */
#define mainTASK_SENDER_PRIO				(tskIDLE_PRIORITY + 1)
#define mainTASK_RECEIVER_PRIO				(tskIDLE_PRIORITY + 2)

/* Define xBufferSizeBytes and xTriggerLevelBytes of StreamBuffer */
#define mainBUFFER_SIZE_BYTES			(13)
#define mainTRIGGER_LEVEL_BYTES		(7)

/*--------------------------------DECLARE GLOBAL VARIABLES--------------------------------*/
uint32_t SystemCoreClock = 16000000U;
StreamBufferHandle_t xStreamBuffer = NULL;

const char * stringControlRedLED = "RedLED on";
const char * stringControlGreenLED = "GreenLED on";

volatile uint32_t * g_RedLED = NULL;
volatile uint32_t * g_GreenLED = NULL;

size_t g_xByteSent = 0; /* The number of byte sent to streambuffer */

/* Task Handle */
TaskHandle_t g_xTaskReceive = NULL;

/*--------------------------------END DECLARE GLOBAL VARIABLES-----------------------------*/

/*--------------------------------DECLARE PROTOTYPE FUNCTION-----------------------------*/
void vTask_Sender(void * param); /* Task sends byte of data to streambuffer */

void vTask_Receiver(void * param); /* Task receives byte of data to streambuffer */

/*--------------------------------END DECLARE PROTOTYPE FUNCTION---------------------------*/

int main(){
	/* Initializes the PLL to a bus clock frequency of 80MHz */
	PLL_Init80MHz();
	
	SystemCoreClock = PLL_GetBusClockFreq(); /* SystemCoreClock as 80MHz */
	
	GPIO_EnableDO(PORTF, PIN1 | PIN3, DRIVE_2MA, PULL_DOWN); /* Enable 3 LEDs on board as output */
	/* Get bit-band address peripherals */
	g_RedLED = GPIO_BitBandPheripheralAddress(&mainREDLED);
	g_GreenLED = GPIO_BitBandPheripheralAddress(&mainGREENLED);
	
	/* Turn off LEDs */
	(*g_RedLED) = 0;
	(*g_GreenLED) = 0;
	
	/* UART5 baud-rate = 9600 */
	UART_Enable(UART5, 9600);	
	
	/* Create Streambuffer */
	xStreamBuffer = xStreamBufferCreate(mainBUFFER_SIZE_BYTES, mainTRIGGER_LEVEL_BYTES);
	if (xStreamBuffer != NULL)
	{
		/* Initialize tasks */
		xTaskCreate(vTask_Sender, (const char *)"Sender", 
								configMINIMAL_STACK_SIZE, NULL, mainTASK_SENDER_PRIO, NULL);
		
		vTaskStartScheduler();
	}
	
	/* This code following will be never executed */
	for(;;);
}

void vTask_Sender(void * param)
{
	bool flagToggleSend = true;
	size_t sbufferSpace;
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = pdMS_TO_TICKS(2000);
	/* Initialise the xLastWakeTime variable with the current time */
  xLastWakeTime = xTaskGetTickCount();
	UNUSED(param);
	while(1)
	{
		sbufferSpace = xStreamBufferSpacesAvailable(xStreamBuffer);
		if (sbufferSpace >= mainBUFFER_SIZE_BYTES)
		{
			if (flagToggleSend)
			{
				g_xByteSent = xStreamBufferSend(xStreamBuffer, (void *)stringControlRedLED, strlen(stringControlRedLED), 0);
				flagToggleSend = false;
			}
			else
			{
				g_xByteSent = xStreamBufferSend(xStreamBuffer, (void *)stringControlGreenLED, strlen(stringControlGreenLED), 0);
				flagToggleSend = true;
			}
			
			/* Print to monitor notification about data sent */
			if (g_xByteSent == strlen(stringControlRedLED))
			{
				taskENTER_CRITICAL();
				/* Print "stringControlRedLED is sent successfully" */
				UART_WriteString(UART5, (char *)"[Sender]stringControlRedLED is sent successfully.\r\n");
				taskEXIT_CRITICAL();
			}
			else if (g_xByteSent == strlen(stringControlGreenLED))
			{
				taskENTER_CRITICAL();
				/* Print "stringControlGreenLED is sent successfully" */
				UART_WriteString(UART5, (char *)"[Sender]stringControlGreenLED is sent successfully.\r\n");
				taskEXIT_CRITICAL();
			}
		}
		else
		{
			/* Block task until space in streambuffer becomes available */
			vTaskDelayUntil(&xLastWakeTime, xFrequency);
		}
		 /* Create task receiver after sending data to streambuffer */
		if (g_xTaskReceive == NULL)
		{
			xTaskCreate(vTask_Receiver, (const char *)"Receiver", 
								configMINIMAL_STACK_SIZE, NULL, mainTASK_RECEIVER_PRIO, &g_xTaskReceive);
		}
	}
}

void vTask_Receiver(void * param)
{
	char BufferRecievie[14];
	size_t xReceivedBytes, sbufferByteExist;
	
	UNUSED(param);
	while(1)
	{
		char receiverNote[] = "[Receiver]";
		sbufferByteExist = xStreamBufferBytesAvailable(xStreamBuffer);
		if (sbufferByteExist == g_xByteSent)
		{
			memset(BufferRecievie, 0, sizeof(BufferRecievie));
			xReceivedBytes = xStreamBufferReceive(xStreamBuffer, BufferRecievie, sizeof(BufferRecievie), 0);
			strcat(receiverNote, BufferRecievie);
			strcat(receiverNote, (char *)"\r\n\r\n"); /* New line */
			if (xReceivedBytes == strlen(stringControlRedLED))
			{
				(*g_GreenLED) = 0; /* Green LED off */
				(*g_RedLED) = 1; /* Red LED on */
			}
			else if (xReceivedBytes == strlen(stringControlGreenLED))
			{
				(*g_RedLED) = 0; /* Red LED off */
				(*g_GreenLED) = 1; /* Green LED on */
			}
			/* Print notification */
			UART_WriteString(UART5, receiverNote);
		}
		else
		{
			vTaskDelay(pdMS_TO_TICKS(300));
		}
	}
}


	


