#ifndef TM4C_PLL_H
#define TM4C_PLL_H

#include <stdint.h>

/* Initializes the PLL to a bus clock frequency of 80MHz */
void PLL_Init80MHz(void);

/* Gets the PLL bus clock frequency */
uint32_t PLL_GetBusClockFreq(void);

#endif


