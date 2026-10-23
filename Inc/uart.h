#ifndef UART_H
#define UART_H

#include "stm32f103xb.h"
#include <stdint.h>

void UART_Init(void);
void UART_SetBaud(uint32_t baud);
void UART_SendChar(char c);
void UART_SendString(const char *str);
int  UART_ReadChar(char *c);   /* 1 = co byte, 0 = rong */
void UART_Flush(void);

void     SysTick_Init(void);
uint32_t Millis(void);
void     Delay_ms(uint32_t ms);

#endif
