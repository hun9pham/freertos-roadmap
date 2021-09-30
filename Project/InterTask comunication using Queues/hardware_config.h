#ifndef _HARDWARE_CONFIG_H_
#define _HARDWARE_CONFIG_H_

#ifdef __cplusplus
extern "C"{
#endif

#include <stdint.h>
#include <stdbool.h>

/* Avoid warning about unused parameters from compiler */
#define UNUSED(parameter)								do { (void)(parameter); }while(0);

#define LED_RED						(1U << 1)
#define LED_BLUE					(1U << 2)
#define LED_GREEN					(1U << 3)

/* 
* Enable the PLL and initialize the bus frequency to 80MHz
*	Config three LEDs on board
* Config enable interrupt on SW1, SW2 occurs
*/
void hardware_init(void);

/* GPIOF interrupt handler */
void GPIOF_Handler(void);

/* LEDs(State) function on GPIOF */
void LEDsOn(uint8_t ui8Pin);
void LEDsOff(uint8_t ui8Pin);


#ifdef __cplusplus
}
#endif
	
#endif
