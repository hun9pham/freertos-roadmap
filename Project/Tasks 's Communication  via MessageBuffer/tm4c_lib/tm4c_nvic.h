#ifndef TM4C_NVIC_H
#define TM4C_NVIC_H

#include <stdint.h>

void NVIC_EnableIRQ(uint8_t irq, uint8_t priority);

void NVIC_DisableIRQ(uint8_t irq);


#endif

