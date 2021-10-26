#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "tm4c_lib.h"


/* Base addresses of the ports defined in the file header tm4c123gh6pm.h */
const volatile uint32_t * GPIOBaseAddress[] = {
	GPIO_PORTA_DATA_BITS_R,
	GPIO_PORTB_DATA_BITS_R,
	GPIO_PORTC_DATA_BITS_R,
	GPIO_PORTD_DATA_BITS_R,
	GPIO_PORTE_DATA_BITS_R,
	GPIO_PORTF_DATA_BITS_R
};


// These hold the function pointers to the callback functions invoked in the interrupt handlers.
static GPIOCallback PortF_Callback;

/* Initializes a GPIO port for IO usage */
void GPIO_InitPort(PortName_t port)
{
	uint32_t portBit = (0x1 << (uint8_t)port);
	if (!(SYSCTL_PRGPIO_R & portBit)) {
		/* Open clock gating control register enables the GPIO function for the port */
		SYSCTL_RCGCGPIO_R |= portBit; 
		/* Wait until perpheral ready register */
		while (!(SYSCTL_PRGPIO_R & portBit ));
	}
}

/* Enables a set of pins on a GPIO port for digital output */
void GPIO_EnableDO(PortName_t port, uint8_t pinMap, GPIO_Drive_t driveType, GPIO_Pull_t pullType)
{
	GPIO_InitPort(port);
	volatile GPIORegs_t* gpio = (volatile GPIORegs_t*)GPIOBaseAddress[port];
	
	if (port == PORTF && pinMap & 0x01) {
		/* PORTF PIN_0 is normally locked so it need to unlock to use */
		gpio->LOCK = GPIO_LOCK_KEY;
		gpio->CR = 0x01;
	}

	/* Set pins as output */
	gpio->DIR |= pinMap;
	
	/* Disable alternate functions */
	gpio->AFSEL &= (~pinMap);	
	
	/* Set the output drive strength */
	switch (driveType) {
		case DRIVE_2MA:
			gpio->DR2R |= pinMap;
			break;
		case DRIVE_4MA:
			gpio->DR4R |= pinMap;
			break;
		case DRIVE_8MA:
			gpio->DR8R |= pinMap;
			break;
	}
	
	/* Set the pull up/down mode */
	switch (pullType) {
		case PULL_NONE:
			gpio->PUR &= (~pinMap);	
		  gpio->PDR &= (~pinMap);	
			break;
		
		case PULL_UP:
			/* Enable weak pull-up resistors */
			gpio->PUR |= pinMap;	
			break;
		
		case PULL_DOWN:
			/* Enable weak pull-down resistors */
			gpio->PDR |= pinMap;	
			break;
	}
	
	/* Enable pins for digital I/O and clear for analog I/O */
	gpio->DEN |= pinMap;
	gpio->AMSEL &= (~pinMap);
}

/* Get a specified pin */
static uint8_t GetPinBitBandPheripheral(const PinDef_t * pinDef)
{
	uint8_t i = 0;
	for (; i < 8; i++)
	{
		if (((pinDef->pin) >> i) & 0x01)
		{
			return i;
		}
	}
	return 0;
}

/*
* Access to bit-band region peripherals
* FORMULA:
* bit_word_offset = (byte_offset x 32) + (bit_number x 4)
* bit_word_addr = bit_band_base + bit_word_offset
*/
volatile uint32_t * GPIO_BitBandPheripheralAddress(const PinDef_t *pinDef)
{
	const volatile uint32_t * baseAddress = GPIOBaseAddress[pinDef->port];
	uint8_t bit_number = GetPinBitBandPheripheral(pinDef);
	uint32_t byte_offset = ((uint32_t)baseAddress + 0x3FC - PERIPHERAL_BIT_BAND_REGION);
	uint32_t bit_word_offset =  (byte_offset * 32) + (bit_number * 4);
	
	uint32_t bit_word_addr = (uint32_t)PERIPHERAL_BIT_BAND_ALIAS + bit_word_offset;
	return (volatile uint32_t *)bit_word_addr;
}

/* Wirte value to the specified pin */
void GPIO_PinWriteValue(PortName_t port, uint32_t pinMap, uint8_t value)
{
	volatile GPIORegs_t * gpio = (volatile GPIORegs_t *)GPIOBaseAddress[port];
	gpio->DATA_BITS[pinMap] = value;
}

/* Enables a set of pins on a GPIO port for digital input */
void GPIO_EnableDI(PortName_t port, uint8_t pinMap, GPIO_Pull_t pullType)
{
	GPIO_InitPort(port);
	
	volatile GPIORegs_t* gpio = (volatile GPIORegs_t*)GPIOBaseAddress[port];
	
	if (port == PORTF && pinMap & 0x01) {
		/* PORTF PIN_0 is normally locked so it need to unlock to use */
		gpio->LOCK = GPIO_LOCK_KEY;
		gpio->CR = 0x01;
	}
	
	/* Set pins as input */
	gpio->DIR &= (~pinMap);
	
	/* Disable alternate functions */
	gpio->AFSEL &= (~pinMap);	
	
	/* Set the pull up/down mode */
	switch (pullType) {
		case PULL_NONE:
			gpio->PUR &= (~pinMap);	
		  gpio->PDR &= (~pinMap);	
			break;
		
		case PULL_UP:
			gpio->PUR |= pinMap;	
			break;
		
		case PULL_DOWN:
			gpio->PDR |= pinMap;	
			break;
	}
	
	/* Enable pins for digital I/O and clear for analog I/O */
	gpio->DEN |= pinMap;
	gpio->AMSEL &= (~pinMap);
	
}

void GPIO_EnableAltDigital(PortName_t port, uint8_t pinMap, uint8_t ctl)
{
	GPIO_InitPort(port);
	
	volatile GPIORegs_t* gpio = (volatile GPIORegs_t*)GPIOBaseAddress[port];
	uint32_t pctlValue = 0;
	int i;
	
	/* Enable pins for digital and disable for analog */
	gpio->DEN |= pinMap;
	gpio->AMSEL &= ~pinMap;
	
	/* Enable pins for alternate function */
	gpio->AFSEL |= pinMap;
	
	for (i = 0; i < 8; i++) {
		if (pinMap & (0x1 << i)) {
			pctlValue |= (ctl << (4*i));
		}
	}
	
	// Set the pin control function for each pin.
	gpio->PCTL |= pctlValue;

}


void GPIO_EnableAltAnalog(PortName_t port, uint8_t pinMap)
{
	GPIO_InitPort(port);
	
	volatile GPIORegs_t* gpio = (volatile GPIORegs_t*)GPIOBaseAddress[port];

	/* Enable pins for analog and disable for digital */
	gpio->DEN &= ~pinMap;
	gpio->AMSEL |= pinMap;
	
	/* Not seemingly necessary, but disable pull ups/downs anyway */
	gpio->PUR &= ~pinMap;	
	gpio->PDR &= ~pinMap;	
	
	/* Enable pins for alternate function */
	gpio->AFSEL |= pinMap;
}


void GPIO_EnableInterrupt(const PinDef_t* pinDef, uint8_t priority, GPIO_IntTrigger_t triggerType, GPIOCallback callback)
{
	volatile GPIORegs_t* gpio = (volatile GPIORegs_t*)GPIOBaseAddress[pinDef->port];
	
	switch (triggerType) 
	{
		case INT_TRIGGER_LOW_LEVEL:
			gpio->IS |= pinDef->pin;
			gpio->IEV &= (~pinDef->pin);
			gpio->IBE &= ~pinDef->pin;		
			break;
		
		case INT_TRIGGER_HIGH_LEVEL:
			gpio->IS |= pinDef->pin;
			gpio->IEV |= pinDef->pin;
			gpio->IBE &= (~pinDef->pin);		
			break;
		
		case INT_TRIGGER_FALLING_EDGE:
			gpio->IS &= (~pinDef->pin);
			gpio->IEV &= (~pinDef->pin);
			gpio->IBE &= (~pinDef->pin);		
			break;
		
		case INT_TRIGGER_RISING_EDGE:
			gpio->IS &= (~pinDef->pin);
			gpio->IEV |= (pinDef->pin);
			gpio->IBE &= (~pinDef->pin);		
			break;
		
		case INT_TRIGGER_BOTH_EDGES:
			gpio->IS &= (~pinDef->pin);
			gpio->IBE |= pinDef->pin;		
			break;		
		default:
			break;
	}

	/* Clear any prior interrupt and unmask interrupt for the specified pin */
	gpio->ICR |= pinDef->pin;
	gpio->IM |= pinDef->pin;
		
	switch (pinDef->port) {
		
		case PORTF:
			/* IRQ numbers are 16 less than the INT number */
			NVIC_EnableIRQ(INT_GPIOF - 16, priority);
			PortF_Callback = callback;
			break;
		
		default:
			break;
	}
}


/* Disable interrupt after set enable */
void GPIO_DisableInterrupt(const PinDef_t * pinDef)
{
	volatile GPIORegs_t * gpio = (volatile GPIORegs_t *)GPIOBaseAddress[pinDef->port];
	/* Write bit 0 to mask the interrupt on a specified pin */
	gpio->IM &= (~pinDef->pin); 
}

/* Recover interrupt after disable */
void GPIO_RecoverInterrupt(const PinDef_t * pinDef)
{
	volatile GPIORegs_t * gpio = (volatile GPIORegs_t *)GPIOBaseAddress[pinDef->port];
	/* Write bit 1 to unmask the interrupt on a specified pin */
	gpio->IM |= pinDef->pin; 
}

/* 
* General handler (called by the interrupt handles) which clears the 
* interrupt flags and invokes the user callback as needed 
*/
static void HandleInterrupt(volatile GPIORegs_t* gpio, GPIOCallback callback)
{
	/* Save a copy of the status register before clearing */
	uint32_t status = gpio->MIS;

	/* Clear the interrupt flag for the pin in the MIS */
	gpio->ICR |= status;
		
/*
* Check if any of the bits in the masked interrupt status register are set.
* If so, then one of the pins on the GPIO port has triggered an interrupt.
* Invoke the call back if registered
*/
	if ((status & 0xFF) && callback) {
			callback(status);
	}
}


/* PortF interrupt handler.  INT# 46, IRQ 30.  Must be assigned in the IRQ vector table */
void GPIO_PortFHandler(void)
{
	volatile GPIORegs_t* gpio = (volatile GPIORegs_t*)GPIOBaseAddress[PORTF];
	HandleInterrupt(gpio, PortF_Callback);	
}



