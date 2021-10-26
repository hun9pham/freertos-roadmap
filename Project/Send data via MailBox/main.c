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
#include "event_groups.h"
#include "queue.h"

/* Avoid warning about unused parameters from compiler */
#define UNUSED(parameter)					do { (void)(parameter); }while(0);

/* Define individual LEDs structures */
#define mainREDLED										PINDEF(PORTF, PIN1)
#define mainBLUELED										PINDEF(PORTF, PIN2)
#define mainGREENLED									PINDEF(PORTF, PIN3)

/* Priorities at which the tasks are created. */
#define mainTASK_RENDEZVOUS_PRIO		(tskIDLE_PRIORITY + 2)
#define mainTASK_COMMON_PRIO				(tskIDLE_PRIORITY + 1)

/* MailBox length and size of items MailBox can hold */
#define mainQUEUE_LENGTH						(1)
#define mainSIZE_OF_ITEM						(sizeof(MailBoxData_t))

/* Event bits corresponding to three task control LEDs */
#define mainEVT_BIT_RED							(1U << 0) // bit 0
#define mainEVT_BIT_BLUE						(1U << 1) // bit 1
#define mainEVT_BIT_GREEN						(1U << 2) // bit 2
#define mainEVT_BITS_WAIT						(mainEVT_BIT_RED | mainEVT_BIT_GREEN)

/*--------------------------------DECLARE GLOBAL VARIABLES--------------------------------*/

/* Data structure send to MailBox */
typedef struct MailBoxDataStructure{
	TickType_t TimeUpdate;
	uint32_t Data;
} MailBoxData_t;


uint32_t SystemCoreClock = 16000000U;

/* EventGroup object */
EventGroupHandle_t xEventGroup = NULL;

/* Mutex object */
SemaphoreHandle_t xMutex = NULL;;

/* MailBox object */
QueueHandle_t xMailBox = NULL;

/* Get bit-band address three LEDs */
volatile uint32_t * g_RedLED = NULL;
volatile uint32_t * g_BlueLED = NULL;
volatile uint32_t * g_GreenLED = NULL;

TaskHandle_t TaskReadMailBox = NULL;


/*--------------------------------END DECLARE GLOBAL VARIABLES-----------------------------*/

/*--------------------------------DECLARE PROTOTYPE FUNCTION-----------------------------*/

void vTask_UpdateMailBox(void * param); /* Overwrite data to MailBox */

void vTask_ReadMailBox(void * param); /* Read data update from MailBox */

/*--------------------------------END DECLARE PROTOTYPE FUNCTION---------------------------*/

int main(){
	/* Initializes the PLL to a bus clock frequency of 80MHz */
	PLL_Init80MHz();
	SystemCoreClock = PLL_GetBusClockFreq(); /* SystemCoreClock as 80MHz */
	
	/* Enable digital output on PIN1, PIN2, PIN3 */
	GPIO_EnableDO(PORTF, PIN1 | PIN2 | PIN3, DRIVE_2MA, PULL_DOWN);
	
	/* UART5 Enable with baud-rate 9600 */
	UART_Enable(UART5, 9600);
	
	/* Point to bit-band addess PORTF three LEDs */
	g_RedLED = GPIO_BitBandPheripheralAddress(&mainREDLED);
	g_BlueLED = GPIO_BitBandPheripheralAddress(&mainBLUELED);
	g_GreenLED = GPIO_BitBandPheripheralAddress(&mainGREENLED);
	
	/* Turn LEDs off */
	(*g_RedLED) = 0;
	(*g_BlueLED) = 0;
	(*g_GreenLED) = 0;
	
	xEventGroup = xEventGroupCreate(); 
	
	/* Create MailBox */
	xMailBox  = xQueueCreate(mainQUEUE_LENGTH, mainSIZE_OF_ITEM);
	
	/* Create Mutex*/
	xMutex = xSemaphoreCreateMutex();

	if (xMailBox != NULL)
	{
		/* Initialize tasks */
		xTaskCreate(vTask_UpdateMailBox, (const char *)"Update", 
								configMINIMAL_STACK_SIZE, (void *)1, mainTASK_COMMON_PRIO, NULL);
		
		/* Start the scheduler */
		vTaskStartScheduler();
	}
	
	/* This code following will be never executed */
	for(;;);
}

void vTask_UpdateMailBox(void * param)
{
	char buffer[50];
	/* Set up data send to MailBox */
	MailBoxData_t mbData;
	mbData.Data = 0; /* Start data counter at 0 */
	
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = pdMS_TO_TICKS(3000);
	xLastWakeTime = xTaskGetTickCount();
	while(1)
	{
		/* Get time update afer each iterator */
		mbData.TimeUpdate = xTaskGetTickCount(); 
		/* Send data to MailBox and overwrite old data */
		xQueueOverwrite(xMailBox, &mbData);
		
		/* Clear char buffer */
		memset(buffer, 0, sizeof(buffer) / sizeof(buffer[0]));
		sprintf(buffer, "Data sends to MailBox = %d\r\n", mbData.Data);
		
		if (TaskReadMailBox == NULL)
		{
			xTaskCreate(vTask_ReadMailBox, (const char *)"Read", 
								configMINIMAL_STACK_SIZE, (void *)&mbData, mainTASK_COMMON_PRIO + 1, &TaskReadMailBox);
		}
		
		/* Avoid race condtion by using Mutex */
		xSemaphoreTake(xMutex, portMAX_DELAY);
		UART_WriteString(UART5, buffer); /* Print message to monitor via UART5 */
		xSemaphoreGive(xMutex);
		
		++(mbData.Data); /* Increse data value which is replaced old value in next iterator */
		
		/* Block task */
		vTaskDelayUntil(&xLastWakeTime, xFrequency);
	}
}


void vTask_ReadMailBox(void * param)	
{
	char buffer[50];
	
	/* Pointer points to MailBox data address */
	MailBoxData_t *p_mbData = (MailBoxData_t *)param;
	MailBoxData_t mbDataReceive; /* Stored data read from MailBox */
	MailBoxData_t *p_mbDataReceive = &mbDataReceive;
	
	TickType_t previousTimeUpdate = 0;
	
	while(1)
	{
		if (p_mbData->TimeUpdate > previousTimeUpdate)
		{
			previousTimeUpdate = p_mbData->TimeUpdate;
			xQueuePeek(xMailBox, p_mbDataReceive, (TickType_t)0);
			
			/* Clear char buffer */
			memset(buffer, 0, sizeof(buffer) / sizeof(buffer[0]));
			sprintf(buffer, "Data reads from MailBox = %d\r\n", p_mbDataReceive->Data);
			
			/* Avoid race condtion by using Mutex */
			xSemaphoreTake(xMutex, portMAX_DELAY);
			UART_WriteString(UART5, buffer); /* Print message to monitor via UART5 */
			UART_WriteString(UART5, "\r\n");
			xSemaphoreGive(xMutex);
			
		}
		else
		{
			vTaskDelay(pdMS_TO_TICKS(100));
		}
	}
}



