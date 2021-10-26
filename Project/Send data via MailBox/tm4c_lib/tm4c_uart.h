#ifndef TM4C_UART_LIB_H
#define TM4C_UART_LIB_H

#include <stdint.h>

// Signature of the Receive (Rx) character callback function 
typedef void (*RxCallback)(char c);
	
// IDs for the ports.
typedef enum {
	UART0,
	UART1,
	UART2,
	UART3,
	UART4,
	UART5,
	UART6,
	UART7
} UART_ID_t;

/* Enables the specified UART for 8,N,1 and the specified baud rate */
uint16_t UART_Enable(UART_ID_t uartId, uint32_t baud);

/* Write a character to a UART */
void UART_WriteChar(UART_ID_t uartId, char c);

/* Writes a null terminated string to a UART */
void UART_WriteString(UART_ID_t uartId, char* sz);

/* Reads a character from a UART.  The function blocks until a character */
char UART_ReadChar(UART_ID_t uartId);

/* 
* Enables a receive (Rx) interrupt for the given UART.  The callback
*	function is invoked whenever a character is received and an interrupt
*	occurs
*/
uint16_t UART_EnableRxInterrupt(UART_ID_t uartId, uint8_t priority, RxCallback callback);

#endif


