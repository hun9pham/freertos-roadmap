#include "hardware_config.h"
/* Include library driver */
#include "inc/TM4C123GH6PM.h"
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"

/* Enable the interrupt occurs on GPIOF when SW1 is pressed */
static void ConfigurationSwitchInt(void);

void hardware_init(void){
	/* Enable the PLL and initialize the bus frequency to 80MHz */
	SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ); /* 200MHz * 2 / 5 */
	/* Enable Clock - gating peripherals */
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF)); /* wait for a peripheral is ready */
	/* GPIOF - Pin(1, 2, 3) as output */
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3 | GPIO_PIN_2 | GPIO_PIN_1);	
	/* Configuration of the interrupt */
	ConfigurationSwitchInt();
	/* Turn off all LEDs */
	LEDsOff(LED_RED | LED_BLUE | LED_GREEN);
}

static void ConfigurationSwitchInt(void){
	/* Unlock GPIOF pin 0 */
	GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY;
	GPIO_PORTF_CR_R = 0x1;
	/* GPIOF pin 0 as input with pull-up register - SW1 */
	GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_0);
	GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
	/* Interrupt configuration */
	GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_FALLING_EDGE);
	GPIOIntRegister(GPIO_PORTF_BASE, SwitchHandler);
	GPIOF_INT_PRI_ADDRESS = 7 << 21; /* Set interrupt priority GPIOF to 7 (lowest priority) */
	GPIOIntEnable(GPIO_PORTF_BASE, GPIO_INT_PIN_0);
}


void LEDsOn(uint8_t ui8Pin){
	GPIOPinWrite(GPIO_PORTF_BASE, ui8Pin, ui8Pin);
}

void LEDsOff(uint8_t ui8Pin){
	uint8_t ui8PinOff = ~ui8Pin;
	ui8PinOff &= ui8Pin;
	GPIOPinWrite(GPIO_PORTF_BASE, ui8Pin, ui8PinOff);
}

void LEDsToggle(uint8_t ui8Pin){
	static uint8_t s_ui8Flag = true;
	if (s_ui8Flag){
		LEDsOn(ui8Pin);
		s_ui8Flag = false;
	}
	else{
		LEDsOff(ui8Pin);
		s_ui8Flag = true;
	}
}
