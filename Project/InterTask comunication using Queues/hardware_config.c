#include "hardware_config.h"
/* Include library driver */
#include "inc/TM4C123GH6PM.h"
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"

void hardware_init(void){
	/* Enable the PLL and initialize the bus frequency to 80MHz */
	SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ); /* 200MHz * 2 / 5 */
	/* Enable Clock - gating peripherals */
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF)); /* wait for a peripheral is ready */
	/* GPIOF - Pin(1, 2, 3) as output */
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3 | GPIO_PIN_2 | GPIO_PIN_1);	
}

void LEDsOn(uint8_t ui8Pin){
	GPIOPinWrite(GPIO_PORTF_BASE, ui8Pin, ui8Pin);
}

void LEDsOff(uint8_t ui8Pin){
	uint8_t ui8PinOff = ~ui8Pin;
	ui8PinOff &= ui8Pin;
	GPIOPinWrite(GPIO_PORTF_BASE, ui8Pin, ui8PinOff);
}

