#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "tm4c_lib/tm4c_lib.h"
#include "tm4c_lib/tm4c123gh6pm.h"

/* Include FreeRTOS library */
#include <FreeRTOS.h>
#include <task.h>
#include "message_buffer.h"

/* Avoid warning about unused parameters from compiler */
#define UNUSED(parameter)					do { (void)(parameter); }while(0);

/* Define pin, port */
#define mainREDLED										PINDEF(PORTF, PIN1)
#define mainGREENLED									PINDEF(PORTF, PIN3)

/* Priorities at which the tasks are created. */
#define mainTASK_RECEIVER_PRIO				(tskIDLE_PRIORITY + 1)

/* The total number of bytes the message buffer will be able to hold at any one time */
#define mainMESS_BUFFER_SIZE_BYTES		(50)

/*--------------------------------DECLARE GLOBAL VARIABLES--------------------------------*/
const char * MessageArray[] = { /* Array holds message sent to MessageBuffer */
	"Message One",
	"Message Two",
	"Message Three",
};
const uint32_t g_MessArrayLength = sizeof(MessageArray)/sizeof(char *);

uint32_t SystemCoreClock = 16000000U;

/* Message buffer object */
MessageBufferHandle_t xMessageBuffer = NULL;

/* Get bit-band address SW1 */
volatile uint32_t * g_ReadSWstatus = NULL;

/*--------------------------------END DECLARE GLOBAL VARIABLES-----------------------------*/

/*--------------------------------DECLARE PROTOTYPE FUNCTION-----------------------------*/

void vTask_ReadMessageBuffer(void * param); /* Task reads byte of data to streambuffer */

void SwitchHandler(uint32_t port); /* Interrupt handler */

/*--------------------------------END DECLARE PROTOTYPE FUNCTION---------------------------*/

int main(){
	/* Initializes the PLL to a bus clock frequency of 80MHz */
	PLL_Init80MHz();
	SystemCoreClock = PLL_GetBusClockFreq(); /* SystemCoreClock as 80MHz */

	/* UART5 baud-rate = 9600 */
	UART_Enable(UART5, 9600);	
	
	/* Interrupt SW1 configuraiton */
	GPIO_EnableDI(PORTF, PIN4, PULL_UP);
	GPIO_EnableInterrupt(&PINDEF(PORTF, PIN4), (uint8_t)7, INT_TRIGGER_FALLING_EDGE, SwitchHandler);
	
	/* Point to bit-band addess GPIOF pin 4 */
	g_ReadSWstatus = GPIO_BitBandPheripheralAddress(&PINDEF(PORTF, PIN4));
	
	/* Create Messagebuffer */
	xMessageBuffer = xMessageBufferCreate(mainMESS_BUFFER_SIZE_BYTES);
	if (xMessageBuffer != NULL)
	{
		/* Initialize tasks */
		xTaskCreate(vTask_ReadMessageBuffer, (const char *)"ReadMessage", 
								configMINIMAL_STACK_SIZE, NULL, mainTASK_RECEIVER_PRIO, NULL);
		
		/* Start the scheduler */
		vTaskStartScheduler();
	}
	
	/* This code following will be never executed */
	for(;;);
}


void vTask_ReadMessageBuffer(void * param)
{
	/* Stored message received */
	char MessageArrayReceive[mainMESS_BUFFER_SIZE_BYTES];
	const uint32_t MessageArrayReceiveSize = sizeof(MessageArrayReceive) / sizeof(char);
	size_t xReceivedBytes;
	UNUSED(param);
	while(1)
	{
		/* Clear MessageArrayReceive after each received */
		memset(MessageArrayReceive, 0, MessageArrayReceiveSize);
		
		/* Wait ultil message buffer available from ISR */
		xReceivedBytes = xMessageBufferReceive(xMessageBuffer, (void *)MessageArrayReceive, MessageArrayReceiveSize, portMAX_DELAY);
		
		/* Print to monitor by UART5 */
		if (xReceivedBytes > 0)
		{
			strcat(MessageArrayReceive, "\r\n");
			UART_WriteString(UART5, MessageArrayReceive);
			GPIO_RecoverInterrupt(&PINDEF(PORTF, PIN4)); /* Enable ISR after disabling */
		}
	}
}

void SwitchHandler(uint32_t port)
{
	UNUSED(port);
	static int8_t MessageArrayIndex = -1;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	
	/* Avoid SW bounce */
	while ((*g_ReadSWstatus) == 0){}
	GPIO_DisableInterrupt(&PINDEF(PORTF, PIN4)); /* Disable ISR */
		
	/* Send messages sequently */
	if (MessageArrayIndex == (g_MessArrayLength -1))
	{
		MessageArrayIndex = 0;
	}
	else
	{
		MessageArrayIndex++; /* Increase message's index */
	}
	
	/* Send message to MessageBuffer from ISR */
	xMessageBufferSendFromISR(xMessageBuffer, (void *)MessageArray[MessageArrayIndex], 
														strlen(MessageArray[MessageArrayIndex]), &xHigherPriorityTaskWoken);
	
	/* Wake up task form ISR */	
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		
}


	


