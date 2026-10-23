
#include "stm32f103xb.h"
#include "debug.h"
#include <stdio.h>
#include <stdarg.h>
/*
 * USART2 DEBUG
 * PA2 = TX -> RXD CP2102
 * PA3 = RX <- TXD CP2102
 * Baud rate: 9600
 *
 * Gia dinh PCLK1 = 8 MHz (HSI mac dinh,
 * chua cau hinh PLL va prescaler APB1).
 */

void DEBUG_Init(void)
{
    /* 1. Enable clock GPIOA va USART2 */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    /* Doc lai thanh ghi de dam bao clock da duoc cap */
    (void)RCC->APB1ENR;

    /* 2. PA2: Alternate Function Push-Pull, 50 MHz */
    GPIOA->CRL &= ~(GPIO_CRL_MODE2 | GPIO_CRL_CNF2);
    GPIOA->CRL |= GPIO_CRL_MODE2_1 | GPIO_CRL_MODE2_0;
    GPIOA->CRL |= GPIO_CRL_CNF2_1;

    /* 3. PA3: Input Floating */
    GPIOA->CRL &= ~(GPIO_CRL_MODE3 | GPIO_CRL_CNF3);
    GPIOA->CRL |= GPIO_CRL_CNF3_0;

    /* 4. USART2: 9600 baud @ PCLK1 = 8 MHz */
    USART2->CR1 = 0;
    USART2->CR2 = 0;
    USART2->CR3 = 0;

    USART2->BRR = 0x0341;

    /* 8 data bits, no parity, 1 stop bit */
    USART2->CR1 = USART_CR1_TE | USART_CR1_UE;
}

void DEBUG_SendChar(char c)
{
    while (!(USART2->SR & USART_SR_TXE))
    {
    }

    USART2->DR = (uint8_t)c;
}

void DEBUG_SendString(const char *s)
{
    while (*s)
    {
        DEBUG_SendChar(*s++);
    }

    /* Cho ky tu cuoi cung truyen xong */
    while (!(USART2->SR & USART_SR_TC))
    {
    }
}

void DEBUG_Printf(const char *format, ...)
{
    char buffer[256];
    va_list args;

    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    DEBUG_SendString(buffer);
}
