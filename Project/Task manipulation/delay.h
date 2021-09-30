/*
* This file contains two modules function DelayMS() and DelayUS() using SysCtlDelay() 
* from TivaWare™ Peripheral Driver Library. The calculation formula has been explained 
* in detail in delay.c
* TivaWare™ Peripheral Driver Library references: https://www.ti.com/lit/ug/spmu298e/spmu298e.pdf
*/

#ifndef _DELAY_H_
#define _DELAY_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Delay milisecond function by SysCtlDelay() */
void DelayMS(uint32_t ui32MStime);

/* Delay microsecond function by SysCtlDelay() */
void DelayUS(uint32_t ui32UStime);

#ifdef __cplusplus
}
#endif

#endif
