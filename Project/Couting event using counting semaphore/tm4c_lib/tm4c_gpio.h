#ifndef TM4C_GPIO_H
#define TM4C_GPIO_H

#include <stdint.h>

/* The starting address of the periphrals bit-band alias region */
#define PERIPHERAL_BIT_BAND_REGION		0x40000000
/* The starting address of the periphrals bit-band region */
#define PERIPHERAL_BIT_BAND_ALIAS			0x42000000

/* List of the GPIO registers, in numerical order by address offset */
typedef struct {
	uint32_t DATA_BITS[255];
	uint32_t DATA;
	uint32_t DIR;
	uint32_t IS;
	uint32_t IBE;
	uint32_t IEV;
	uint32_t IM;
	uint32_t RIS;
	uint32_t MIS;
	uint32_t ICR;
	uint32_t AFSEL;
	uint32_t RESERVED[55];
	uint32_t DR2R;
	uint32_t DR4R;
	uint32_t DR8R;
	uint32_t ODR;
	uint32_t PUR;
	uint32_t PDR;
	uint32_t SLR;
	uint32_t DEN;
	uint32_t LOCK;
	uint32_t CR;
	uint32_t AMSEL;
	uint32_t PCTL;
	uint32_t ADCCTL;
	uint32_t DMACTL;
} GPIORegs_t;


/* Names for output drive strength */
typedef enum {
	DRIVE_2MA, 
	DRIVE_4MA, 
	DRIVE_8MA 
} GPIO_Drive_t;

/* Names for the internal pull-up resistor types */
typedef enum { 
	PULL_NONE, 
	PULL_UP, 
	PULL_DOWN 
} GPIO_Pull_t;

/* Type of trigger interrupt */
typedef enum { 
	INT_TRIGGER_NONE, 
	INT_TRIGGER_LOW_LEVEL,
	INT_TRIGGER_HIGH_LEVEL,
	INT_TRIGGER_RISING_EDGE,
	INT_TRIGGER_FALLING_EDGE,
	INT_TRIGGER_BOTH_EDGES
} GPIO_IntTrigger_t;

/* Pointer function of the GPIO callback function */
typedef void (* GPIOCallback)(uint32_t pinMap);

/* Enables a set of pins on a GPIO port for digital output */
void GPIO_EnableDO(PortName_t port, uint8_t pinMap, GPIO_Drive_t driveType, GPIO_Pull_t pullType);

/* Access to bit-band region peripherals */
volatile uint32_t * GPIO_BitBandPheripheralAddress(const PinDef_t *pinDef);

/* Wirte value to the specified pin */
void GPIO_PinWriteValue(PortName_t port, uint32_t pinMap, uint8_t value);

/* Enables a set of pins on a GPIO port for digital input. */
void GPIO_EnableDI(PortName_t port, uint8_t pinMap, GPIO_Pull_t pullType);

/* Enables an interrupt for the specified port.   */
void GPIO_EnableInterrupt(const PinDef_t* pinDef, uint8_t priority, GPIO_IntTrigger_t triggerType, GPIOCallback callback);

/* Disable interrupt after set enable */
void GPIO_DisableInterrupt(const PinDef_t * pinDef);

/* Recover interrupt after disable */
void GPIO_RecoverInterrupt(const PinDef_t * pinDef);

#endif
