#include <stdint.h>
#include <stdbool.h>
#include "tm4c_lib/tm4c_lib.h"

/* Include FreeRTOS library */
#include <FreeRTOS.h>
#include <task.h>
#include "queue.h"
#include "semphr.h"

/* Avoid warning about unused parameters from compiler */
#define UNUSED(parameter)					do { (void)(parameter); }while(0);

#define SWITCH_1									PINDEF(PORTF, PIN4)
#define REDLED										PINDEF(PORTF, PIN1)
#define BLUELED										PINDEF(PORTF, PIN2)
#define GREENLED									PINDEF(PORTF, PIN3)

/* Priorities at which the tasks are created. */
#define mainTASK_TURN_LED_ON			(tskIDLE_PRIORITY + 1)
#define mainTASK_TURN_LED_OFF			(tskIDLE_PRIORITY + 1)

/*--------------------------------DECLARE GLOBAL VARIABLES--------------------------------*/

/* The internal clock. This is the value of macro configCPU_CLOCK_HZ which defined in FreeRTOSConfig.h */
uint32_t SystemCoreClock = 16000000U; 
SemaphoreHandle_t g_xBinarySemaphore = NULL;
volatile uint32_t * g_RedLED = NULL; /* Control LED red */
volatile uint32_t * g_BlueLED = NULL; /* Control LED blue */
volatile uint32_t * g_GreenLED = NULL; /* Control LED green */

/*--------------------------------END DECLARE GLOBAL VARIABLES-----------------------------*/

/*--------------------------------DECLARE PROTOTYPE FUNCTION-----------------------------*/

/* 
* Enable the PLL and initialize the bus frequency to 80MHz
*	Config three LEDs on board
* Config enable interrupt on SW1 occurs
*/
void Hardware_Config(void);
void Switch1Handler(uint32_t pinMap);/* Interrupt handler function */

void vTask_TurnLEDsOn(void * param); /* Wait for semaphore from ISR to turn on three LEDs */
void vTask_TurnLEDsOff(void * param); /* Wait for semaphore from ISR to turn off three LEDs */

/*--------------------------------END DECLARE PROTOTYPE FUNCTION---------------------------*/

int main(){
	Hardware_Config();
	SystemCoreClock = PLL_GetBusClockFreq(); /* SystemCoreClock as 80MHz */
	/* Attempt to create a semaphore. */
  g_xBinarySemaphore = xSemaphoreCreateBinary();
	if (g_xBinarySemaphore != NULL)
	{
		/* Create task */
		xTaskCreate(vTask_TurnLEDsOff,				/* Function that implements the task */
							(const char *)"LEDsOff", 	/* Task name */
							configMINIMAL_STACK_SIZE, /* The size of the stack to allocate to the task */
							NULL, 										/* Parameter passed into the task - not used */
							mainTASK_TURN_LED_OFF,  		/* The priority assigned to the task */
							NULL);										/* Task's handle is not used*/
		xTaskCreate(vTask_TurnLEDsOn, (const char *)"LEDsOn", configMINIMAL_STACK_SIZE, 
													NULL, mainTASK_TURN_LED_ON, NULL);
		/* Start the tasks and timer running */
		vTaskStartScheduler();
	}
	
	/* This code following will be never executed */
	for(;;);
}

void Hardware_Config(void)
{
	/* Enable the PLL and initialize the bus frequency to 80MHz */
	PLL_Init80MHz();
	/* GPIOF PIN1 PIN2 PIN3 as output */
	GPIO_EnableDO(PORTF, PIN1 | PIN2 | PIN3, DRIVE_2MA, PULL_DOWN);
	/* Point to the bid-band address alias region */
	g_RedLED = GPIO_BitBandPheripheralAddress(&REDLED);
	g_BlueLED = GPIO_BitBandPheripheralAddress(&BLUELED);
	g_GreenLED = GPIO_BitBandPheripheralAddress(&GREENLED);
	
	*g_RedLED = *g_BlueLED = *g_GreenLED = 0; /* Turn off all LEDs first */
	/* Enable interrupt on SW1 */
	GPIO_EnableDI(PORTF, PIN4, PULL_UP);
	GPIO_EnableInterrupt(&SWITCH_1, (uint8_t)7, INT_TRIGGER_FALLING_EDGE, Switch1Handler);
}

void Switch1Handler(uint32_t pinMap)
{
	BaseType_t  xHigherPriorityTaskWoken = pdFALSE;
	/* Release a binary semaphore from ISR */
	xSemaphoreGiveFromISR(g_xBinarySemaphore, &xHigherPriorityTaskWoken);
	GPIO_DisableInterrupt(&SWITCH_1);
}

void vTask_TurnLEDsOn(void * param)
{
	UNUSED(param); /* Avoid warning about unused paramter from compiler */
	for(;;)
	{
		/* Wait until a binary semaphore is avaiable */
		if(xSemaphoreTake(g_xBinarySemaphore, portMAX_DELAY) == pdTRUE)
		{
			/* Turn on all LEDs */
			*g_RedLED = *g_BlueLED = *g_GreenLED = 1;
			/* Recover interrupt after disable from ISR */
			GPIO_RecoverInterrupt(&SWITCH_1);
		}
	}
}

void vTask_TurnLEDsOff(void * param)
{
	UNUSED(param); /* Avoid warning about unused paramter from compiler */
	for(;;)
	{
		/* Wait until a binary semaphore is avaiable */
		if(xSemaphoreTake(g_xBinarySemaphore, portMAX_DELAY) == pdTRUE)
		{
			/* Turn off all LEDs */
			*g_RedLED = *g_BlueLED = *g_GreenLED = 0;
			/* Recover interrupt after disable from ISR */
			GPIO_RecoverInterrupt(&SWITCH_1);
		}
	}
}




