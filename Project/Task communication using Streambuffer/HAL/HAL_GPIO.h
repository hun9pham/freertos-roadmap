#ifndef GPIO_HAL_H
#define GPIO_HAL_H

#include <stdint.h>

// Names for output drive strength
typedef enum { DRIVE_2MA, DRIVE_4MA, DRIVE_8MA } GPIO_Drive_t;

// Names for the internal pull-up resistor types.
typedef enum { PULL_NONE, PULL_UP, PULL_DOWN } GPIO_Pull_t;

// Names for the internal pull-up resistor types.
typedef enum { 
	INT_TRIGGER_NONE, 
	INT_TRIGGER_LOW_LEVEL,
	INT_TRIGGER_HIGH_LEVEL,
	INT_TRIGGER_RISING_EDGE,
	INT_TRIGGER_FALLING_EDGE,
	INT_TRIGGER_BOTH_EDGES
} GPIO_IntTrigger_t;

// Signature of the GPIO callback function 
typedef void (*PFN_GPIOCallback)(uint32_t pinMap);

//------------------------- GPIO_InitPort ----------------------------
// Initializes a GPIO port for IO usage.
// Inputs:  port - the name of the port to initialize.
// Outputs:  none.
void GPIO_InitPort(PortName_t port);

//------------------------- GPIO_EnableDO ----------------------------
// Enables a set of pins on a GPIO port for digital output.
// Inputs:  port - the name of the port to initialize.
//	        pinMap - a map of pins to enable.
//          drive - the output drive strength to configure
//          pull - the internal pull-up resistor type to configure.
// Outputs:  none.
void GPIO_EnableDO(PortName_t port, uint8_t pinMap, GPIO_Drive_t drive, GPIO_Pull_t pull);

//------------------------- GPIO_EnableDI ----------------------------
// Enables a set of pins on a GPIO port for digital input.
// Inputs:  port - the name of the port to enable.
//	        pinMap - a map of pins to enable.
//          pull - the internal pull-up resistor type to configure.
// Outputs:  none.
void GPIO_EnableDI(PortName_t port, uint8_t pinMap, GPIO_Pull_t pull);


//------------------------- GPIO_EnableAltDigital --------------------
// Enables a set of pins on a GPIO port for an alternate digital function.
// Inputs:  port - the name of the port to enable.
//	        pinMap - a map of pins to enable.
//          ctl - the alternate digital function id (see datasheet).
// Outputs:  none.
void GPIO_EnableAltDigital(PortName_t port, uint8_t pinMap, uint8_t ctl);

//------------------------- GPIO_EnableAltAnalog --------------------
// Enables a set of pins on a GPIO port for analog usage.
// Inputs:  port - the name of the port to enable.
//	        pinMap - a map of pins to enable.
// Outputs:  none.
void GPIO_EnableAltAnalog(PortName_t port, uint8_t pinMap);

//------------------------- GPIO_GetBitBandIOAddress --------------------
// Gets the bit-band IO address for a pin definition.
// Inputs:  pinDef - pin definition
// Outputs:  pointer with value of bit band address.
volatile uint32_t* GPIO_GetBitBandIOAddress(const PinDef_t* pinDef);

//------------------------- GPIO_EnableInterrupt ----------------------
// Enables an interrupt for the given port.  
// Inputs:
// Outputs: 
int GPIO_EnableInterrupt(const PinDef_t* pinDef, uint8_t priority, GPIO_IntTrigger_t trigger, PFN_GPIOCallback callback);

//------------------------- GPIO_DisarmInterrupt ----------------------
// Disarms an interrupt for the given port after being enabled and armed.  
// Inputs:
// Outputs: 
void GPIO_DisarmInterrupt(const PinDef_t* pinDef); 

//------------------------- GPIO_RearmInterrupt ----------------------
// Rearms an interrupt for the given port after being enabled and disarmed.  
// Inputs:
// Outputs: 
void GPIO_RearmInterrupt(const PinDef_t* pinDef); 


#endif
