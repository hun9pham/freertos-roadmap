#ifndef TM4C_LIB
#define TM4C_LIB

#include <stdint.h>

typedef enum { 
	PORTA = 0, 
	PORTB = 1, 
	PORTC = 2, 
	PORTD = 3, 
	PORTE = 4, 
	PORTF = 5
} PortName_t;


typedef enum { 
	PIN0 = 0x01,
	PIN1 = 0x02,
	PIN2 = 0x04,
	PIN3 = 0x08,
	PIN4 = 0x10,
	PIN5 = 0x20,
	PIN6 = 0x40,
	PIN7 = 0x80
} PinName_t;	


typedef struct {
	PortName_t port;
	PinName_t pin;
}	PinDef_t;

#define PINDEF(port, pin)			((PinDef_t) { port, pin })


#include "tm4c_gpio.h"
#include "tm4c_nvic.h"
#include "tm4c_pll.h"

#endif


