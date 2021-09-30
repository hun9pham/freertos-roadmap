#ifndef _HARDWARE_CONFIG_H_
#define _HARDWARE_CONFIG_H_

#ifdef __cplusplus
extern "C"{
#endif

#include <stdint.h>
#include <stdbool.h>

/* Avoid warning about unused parameters from compiler */
#define UNUSED(parameter)								do { (void)(parameter); }while(0);

#define LED_RED									(1U << 1) 													 /* GPIOF pin 1 */
#define LED_BLUE								(1U << 2) 													 /* GPIOF pin 2 */
#define LED_GREEN								(1U << 3) 													 /* GPIOF pin 3 */
#define SWITCH_1								(1U << 0) 													 /* GPIOF pin 0 */
#define GPIOF_INT_PRI_ADDRESS		(*((volatile uint32_t *)0xE000E41C)) /* Priority register GPIOF address */

/* 
* Enable the PLL and initialize the bus frequency to 80MHz
*	Config three LEDs on board
* Config enable interrupt on SW1 occurs
*/
void hardware_init(void);

/* GPIOF interrupt handler */
void GPIOF_Handler(void);
#define SwitchHandler			GPIOF_Handler

/* LEDs(State) function on GPIOF */
void LEDsOn(uint8_t ui8Pin); 
void LEDsOff(uint8_t ui8Pin);
void LEDsToggle(uint8_t ui8Pin);

#ifdef __cplusplus
}
#endif
	
#endif
