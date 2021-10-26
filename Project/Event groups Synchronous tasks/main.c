#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "tm4c_lib/tm4c_lib.h"
#include "tm4c_lib/tm4c123gh6pm.h"

/* Include FreeRTOS library */
#include <FreeRTOS.h>
#include <task.h>
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


/* Event bits corresponding to three task control LEDs */
#define mainEVT_BIT_RED							(1U << 0) // bit 0
#define mainEVT_BIT_BLUE						(1U << 1) // bit 1
#define mainEVT_BIT_GREEN						(1U << 2) // bit 2
#define mainEVT_BITS_WAIT						(mainEVT_BIT_RED | mainEVT_BIT_BLUE | mainEVT_BIT_GREEN)

/*--------------------------------DECLARE GLOBAL VARIABLES--------------------------------*/

uint32_t SystemCoreClock = 16000000U;

/* EventGroup object */
EventGroupHandle_t xEventGroup = NULL;

/* MailBox object */
QueueHandle_t xMailBox = NULL;

/* Get bit-band address three LEDs */
volatile uint32_t * g_RedLED = NULL;
volatile uint32_t * g_BlueLED = NULL;
volatile uint32_t * g_GreenLED = NULL;


/*--------------------------------END DECLARE GLOBAL VARIABLES-----------------------------*/

/*--------------------------------DECLARE PROTOTYPE FUNCTION-----------------------------*/

void vTask_RedLED(void * param); /* Task control Red LED */
void vTask_BlueLED(void * param); /* Task control Blue LED */
void vTask_GreenLED(void * param); /* Task control Green LED */

void vTask_PointMeeting(void * param); /* Rendezvous of all tasks */


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
	
	/* Create Messagebuffer */
	xEventGroup = xEventGroupCreate();
	if (xEventGroup != NULL)
	{
		/* Initialize tasks */
		xTaskCreate(vTask_RedLED, (const char *)"Red", 
								configMINIMAL_STACK_SIZE, NULL, mainTASK_COMMON_PRIO, NULL);
		
		xTaskCreate(vTask_BlueLED, (const char *)"Blue", 
								configMINIMAL_STACK_SIZE, NULL, mainTASK_COMMON_PRIO, NULL);
		
		xTaskCreate(vTask_GreenLED, (const char *)"Green", 
								configMINIMAL_STACK_SIZE, NULL, mainTASK_COMMON_PRIO, NULL);
		
		xTaskCreate(vTask_PointMeeting, (const char *)"Rendezvous", 
								configMINIMAL_STACK_SIZE, NULL, mainTASK_RENDEZVOUS_PRIO, NULL);
		
		/* Start the scheduler */
		vTaskStartScheduler();
	}
	
	/* This code following will be never executed */
	for(;;);
}

void vTask_RedLED(void * param)
{
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = pdMS_TO_TICKS(500);
	xLastWakeTime = xTaskGetTickCount();
	UNUSED(param);
	while(1)
	{
		xEventGroupSetBits(xEventGroup, mainEVT_BIT_RED); /* Set bit 0 */
		/* Block task */
		vTaskDelayUntil(&xLastWakeTime, xFrequency);
	}
}

void vTask_BlueLED(void * param)
{
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = pdMS_TO_TICKS(500);
	xLastWakeTime = xTaskGetTickCount();
	UNUSED(param);
	while(1)
	{
		xEventGroupSetBits(xEventGroup, mainEVT_BIT_BLUE); /* Set bit 1 */
		/* Block task */
		vTaskDelayUntil(&xLastWakeTime, xFrequency);
	}
}

void vTask_GreenLED(void * param)
{
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = pdMS_TO_TICKS(500);
	xLastWakeTime = xTaskGetTickCount();
	UNUSED(param);
	while(1)
	{
		xEventGroupSetBits(xEventGroup, mainEVT_BIT_GREEN); /* Set bit 2 */
		/* Block task */
		vTaskDelayUntil(&xLastWakeTime, xFrequency);
	}
}

void vTask_PointMeeting(void * param)	
{
	EventBits_t xBitsWait;
	const TickType_t xTicksToWait = 300 / portTICK_PERIOD_MS;
	
	/* Block task setup parameter */
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = pdMS_TO_TICKS(500);
	xLastWakeTime = xTaskGetTickCount();
	
	UNUSED(param);
	while(1)
	{
		/* Wait for until all event bits is set and clear all correspond bits after exiting the function */
		xBitsWait = xEventGroupWaitBits(xEventGroup, 			/* Event group object */
																	mainEVT_BITS_WAIT, 	/* The bits within the event group to wait for */
																	pdTRUE, 						/* Enable clear bits before exiting this function */
																	pdTRUE, 						/* Don't wait for both bits, either bit will do */
																	xTicksToWait);			/* Wait for until all event bits is set */
		
		if (xBitsWait == mainEVT_BITS_WAIT) /* Synchronous rendezvous */
		{
			// All LEDs on
		(*g_RedLED) = (*g_BlueLED) = (*g_GreenLED) = 1;
			UART_WriteString(UART5, "LEDs on");
		}
		else
		{
			// All LEDs off
			(*g_RedLED) = (*g_BlueLED) = (*g_GreenLED) = 0;
			UART_WriteString(UART5, "LEDs off");
		}
		
		UART_WriteString(UART5, "\r\n");
	}
}

	


