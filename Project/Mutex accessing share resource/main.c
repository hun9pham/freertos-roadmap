#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "tm4c_lib/tm4c_lib.h"
#include "tm4c_lib/tm4c123gh6pm.h"

/* Include FreeRTOS library */
#include <FreeRTOS.h>
#include <task.h>
#include "semphr.h"

/* Avoid warning about unused parameters from compiler */
#define UNUSED(parameter)					do { (void)(parameter); }while(0);


/*--------------------------------DECLARE GLOBAL VARIABLES--------------------------------*/

uint32_t SystemCoreClock = 16000000U;

/* Mutex object */
SemaphoreHandle_t xMutex = NULL;;

char g_MessageShareResource[20];

/*--------------------------------END DECLARE GLOBAL VARIABLES-----------------------------*/

/*--------------------------------DECLARE PROTOTYPE FUNCTION-----------------------------*/

void vTask_SendMessageOne(void * param); /* Overwrite data to MailBox */

void vTask_SendMessageTwo(void * param); /* Read data update from MailBox */

/*--------------------------------END DECLARE PROTOTYPE FUNCTION---------------------------*/

int main(){
	/* Initializes the PLL to a bus clock frequency of 80MHz */
	PLL_Init80MHz();
	SystemCoreClock = PLL_GetBusClockFreq(); /* SystemCoreClock as 80MHz */
	
	/* UART5 Enable with baud-rate 9600 */
	UART_Enable(UART5, 9600);

	/* Create Mutex*/
	xMutex = xSemaphoreCreateMutex();

	/* Initialize tasks */
	xTaskCreate(vTask_SendMessageOne, (const char *)"MsgOne", 
							configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1), NULL);
	
	xTaskCreate(vTask_SendMessageTwo, (const char *)"MsgTwo", 
							configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 2), NULL);
	
	/* Start the scheduler */
	vTaskStartScheduler();
	
	/* This code following will be never executed */
	for(;;);
}

void vTask_SendMessageOne(void * param)
{
	const char * MsgOneString = "MessageOne";
	while(1)
	{		
		uint8_t i;
		/* Avoid race condtion by using Mutex */
		xSemaphoreTake(xMutex, portMAX_DELAY);
		for (i = 0; i < strlen(MsgOneString); i++)
		{
			g_MessageShareResource[i] = MsgOneString[i];
			vTaskDelay(50);
		}
		g_MessageShareResource[i] = 0;
		strcat(g_MessageShareResource, "\r\n");
		UART_WriteString(UART5, g_MessageShareResource);
		xSemaphoreGive(xMutex);
		
		vTaskDelay(100);
	}
}


void vTask_SendMessageTwo(void * param)	
{
	const char * MsgTwoString = "MessageTwo";
	while(1)
	{
		uint8_t i;
		/* Avoid race condtion by using Mutex */
		xSemaphoreTake(xMutex, portMAX_DELAY);
		for (i = 0; i < strlen(MsgTwoString); i++)
		{
			g_MessageShareResource[i] = MsgTwoString[i];
			vTaskDelay(50);
		}
		g_MessageShareResource[i] = 0;
		strcat(g_MessageShareResource, "\r\n");
		UART_WriteString(UART5, g_MessageShareResource);
		xSemaphoreGive(xMutex);
		
		vTaskDelay(100);
	}
}


