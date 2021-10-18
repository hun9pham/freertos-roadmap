/* Set configUSE_COUNTING_SEMAPHORES in FreeRTOSConfig.h header file to 1 for usage */
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

#define REDLED										PINDEF(PORTF, PIN1)
#define BLUELED										PINDEF(PORTF, PIN2)
#define GREENLED									PINDEF(PORTF, PIN3)

/* Priorities at which the tasks are created. */
#define mainTASK_LED_RED					(tskIDLE_PRIORITY + 1)
#define mainTASK_LED_BLUE					(tskIDLE_PRIORITY + 1)
#define mainTASK_LED_GREEN				(tskIDLE_PRIORITY + 1)

/* The maximum count value for the semaphore can be reached */
#define mainMAX_COUNT							((UBaseType_t ) 3)
/* The count value assigned to the semaphore when it is created */
#define mainINITIAL_COUNT					((UBaseType_t ) 0)

/*--------------------------------DECLARE GLOBAL VARIABLES--------------------------------*/
uint32_t SystemCoreClock = 16000000U;

SemaphoreHandle_t g_xCountEvtSemaphore = NULL; /* The semaphore types counting event  */

volatile uint32_t * g_ui32BitBand_RedLED = NULL; /* Pointer to bit-band address the red LEDs */
volatile uint32_t * g_ui32BitBand_BlueLED = NULL; /* Pointer to bit-band address the blue LEDs */
volatile uint32_t * g_ui32BitBand_GreenLED = NULL; /* Pointer to bit-band address the green LEDs */

volatile uint32_t * g_ui32BitBandReadSW1 = NULL; /* Pointer to bit-band address the green LEDs */

volatile uint8_t g_ui8CounterDisableISR = 0; /* CounterDisableISR = mainMAX_COUNT then ISR from SW1 will be disable */
volatile UBaseType_t cnt;
/*--------------------------------END DECLARE GLOBAL VARIABLES-----------------------------*/

/*--------------------------------DECLARE PROTOTYPE FUNCTION-----------------------------*/

/* 
* Enable the PLL and initialize the bus frequency to 80MHz
*	Config three LEDs on board
* Config enable interrupt on SW1 occurs
*/
void Hardware_Config(void);

/* Interrupt handler function */
void SwitchHandler(uint32_t pinMap);

void vTask_TurnRedLEDsOn(void * param); 		/* Wait for semaphore from ISR to turn on red LEDs */
void vTask_TurnBlueLEDsOn(void * param); 		/* Wait for semaphore from ISR to turn on blue LEDs */
void vTask_TurnGreenLEDsOn(void * param); 	/* Wait for semaphore from ISR to turn on green LEDs */

/*--------------------------------END DECLARE PROTOTYPE FUNCTION---------------------------*/

int main(){
	Hardware_Config();
	SystemCoreClock = PLL_GetBusClockFreq(); /* SystemCoreClock as 80MHz */
	g_xCountEvtSemaphore = xSemaphoreCreateCounting(mainMAX_COUNT, mainINITIAL_COUNT);
	/* Create task */
	xTaskCreate(vTask_TurnRedLEDsOn,			/* Function that implements the task */
						(const char *)"RedLED", 		/* Task name */
						configMINIMAL_STACK_SIZE, 	/* The size of the stack to allocate to the task */
						NULL, 											/* Parameter passed into the task - not used */
						mainTASK_LED_RED,  					/* The priority assigned to the task */
						NULL);											/* Task's handle is not used*/
						
	xTaskCreate(vTask_TurnBlueLEDsOn, (const char *)"BlueLED", configMINIMAL_STACK_SIZE, 
												NULL, mainTASK_LED_BLUE, NULL);
	
	xTaskCreate(vTask_TurnGreenLEDsOn, (const char *)"GreenLED", configMINIMAL_STACK_SIZE, 
												NULL, mainTASK_LED_GREEN, NULL);
	/* Start the tasks and timer running */
	vTaskStartScheduler();
	
	/* This code following will be never executed */
	for(;;);
}

void Hardware_Config(void)
{
	/* Initializes the PLL to a bus clock frequency of 80MHz */
	PLL_Init80MHz();
	GPIO_EnableDO(PORTF, PIN1 | PIN2 | PIN3, DRIVE_2MA, PULL_DOWN); /* Enable 3 LEDs on board as output */
	/* Point to the bid-band address alias region */
	g_ui32BitBand_RedLED = GPIO_BitBandPheripheralAddress(&REDLED);
	g_ui32BitBand_BlueLED = GPIO_BitBandPheripheralAddress(&BLUELED);
	g_ui32BitBand_GreenLED = GPIO_BitBandPheripheralAddress(&GREENLED);
	
	g_ui32BitBandReadSW1 = GPIO_BitBandPheripheralAddress(&PINDEF(PORTF, PIN4));
	/* Turn off all LEDs */
	*g_ui32BitBand_RedLED = *g_ui32BitBand_BlueLED = *g_ui32BitBand_GreenLED = (uint32_t)0;
	
	/* Initialize ISR on SW1 */
	GPIO_EnableDI(PORTF, PIN4 | PIN0, PULL_UP); 
	GPIO_EnableInterrupt(&PINDEF(PORTF, PIN4), (uint8_t)7, INT_TRIGGER_FALLING_EDGE, SwitchHandler);

}

/* Interrupt handler function */
void SwitchHandler(uint32_t pinMap)
{
	GPIO_DisableInterrupt(&PINDEF(PORTF, PIN4));
	while (!(*g_ui32BitBandReadSW1)); /* Prevention of the switch bounce */
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	/* Release a counting semaphore from ISR */
	xSemaphoreGiveFromISR(g_xCountEvtSemaphore, &xHigherPriorityTaskWoken);
}

	
/* Task implement function */
void vTask_TurnRedLEDsOn(void * param)
{
	static bool s_flagRed = true;
	UNUSED(param); /* Avoid warning about unused paramter from compiler */
	for(;;)
	{
		/* Wait until a counting semaphore is avaiable */
		xSemaphoreTake(g_xCountEvtSemaphore, portMAX_DELAY);
		if (s_flagRed)
		{
			/* Turn on the red LED */
			*g_ui32BitBand_RedLED = 1;
			s_flagRed = false;
		}
		else
		{
			/* Turn off the red LED */
			*g_ui32BitBand_RedLED = 0;
			s_flagRed = true;
		}
		GPIO_RecoverInterrupt(&PINDEF(PORTF, PIN4));
	}
}

void vTask_TurnBlueLEDsOn(void * param)
{
	static bool s_flagBlue = true;
	UNUSED(param); /* Avoid warning about unused paramter from compiler */
	for(;;)
	{
		/* Wait until a counting semaphore is avaiable */
		xSemaphoreTake(g_xCountEvtSemaphore, portMAX_DELAY);
		if (s_flagBlue)
		{
			/* Turn on the blue LED */
			*g_ui32BitBand_BlueLED = 1;
			s_flagBlue = false;
		}
		else
		{
			/* Turn off the blue LED */
			*g_ui32BitBand_BlueLED = 0;
			s_flagBlue = true;
		}
		GPIO_RecoverInterrupt(&PINDEF(PORTF, PIN4));
	}
}

void vTask_TurnGreenLEDsOn(void * param)
{
	static bool s_flagGreen = true;
	UNUSED(param); /* Avoid warning about unused paramter from compiler */
	for(;;)
	{
		/* Wait until a counting semaphore is avaiable */
		xSemaphoreTake(g_xCountEvtSemaphore, portMAX_DELAY);
		if (s_flagGreen)
		{
			/* Turn on the green LED */
			*g_ui32BitBand_GreenLED = 1;
			s_flagGreen = false;
		}
		else
		{
			/* Turn off the green LED */
			*g_ui32BitBand_GreenLED = 0;
			s_flagGreen = true;
		}
		GPIO_RecoverInterrupt(&PINDEF(PORTF, PIN4));
	}
}




