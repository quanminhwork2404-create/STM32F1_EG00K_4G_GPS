#include "uart.h"

/*
 * SYSCLK = PCLK2 = 8 MHz (HSI, SystemInit la ham weak)
 * USART1 = 115200 baud, RX dung ngat + ring buffer
 * SysTick = 1 ms, ngat
 */

#define RX_BUF_SIZE 1024u            /* phai la luy thua cua 2 */

static volatile uint8_t  rx_buf[RX_BUF_SIZE];
static volatile uint16_t rx_head = 0;
static volatile uint16_t rx_tail = 0;
static volatile uint32_t tick_ms = 0;

void UART_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_USART1EN;
    (void)RCC->APB2ENR;

    /* PA9: AF push-pull 50 MHz */
    GPIOA->CRH &= ~(GPIO_CRH_MODE9 | GPIO_CRH_CNF9);
    GPIOA->CRH |= GPIO_CRH_MODE9_0 | GPIO_CRH_MODE9_1 | GPIO_CRH_CNF9_1;

    /* PA10: input floating (hoac pull-up neu module co the de ho) */
    GPIOA->CRH &= ~(GPIO_CRH_MODE10 | GPIO_CRH_CNF10);
    GPIOA->CRH |= GPIO_CRH_CNF10_0;

    USART1->CR1 = 0;
    USART1->CR2 = 0;
    USART1->CR3 = 0;

    USART1->BRR = 0x0045;            /* 8 MHz / 115200 ~ 69.4 */

    rx_head = rx_tail = 0;

    USART1->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE | USART_CR1_UE;

    NVIC_SetPriority(USART1_IRQn, 1);
    NVIC_EnableIRQ(USART1_IRQn);
}

/* Moi byte den se duoc cat vao ring buffer, khong bi mat khi CPU ban in debug */
void USART1_IRQHandler(void)
{
    uint32_t sr = USART1->SR;

    if (sr & (USART_SR_RXNE | USART_SR_ORE))
    {
        uint8_t d = (uint8_t)(USART1->DR & 0xFF);   /* doc SR roi DR -> xoa RXNE/ORE */
        uint16_t next = (uint16_t)((rx_head + 1u) & (RX_BUF_SIZE - 1u));

        if (next != rx_tail)
        {
            rx_buf[rx_head] = d;
            rx_head = next;
        }
    }
}

/* Doi baud luc dang chay (PCLK2 = 8 MHz, oversampling 16) */
void UART_SetBaud(uint32_t baud)
{
    USART1->BRR = (uint16_t)((8000000u + baud / 2u) / baud);
}

void UART_SendChar(char c)
{
    while (!(USART1->SR & USART_SR_TXE)) { }
    USART1->DR = (uint8_t)c;
}

void UART_SendString(const char *str)
{
    while (*str)
    {
        UART_SendChar(*str++);
    }
    while (!(USART1->SR & USART_SR_TC)) { }
}

int UART_ReadChar(char *c)
{
    if (rx_head == rx_tail)
    {
        return 0;
    }

    *c = (char)rx_buf[rx_tail];
    rx_tail = (uint16_t)((rx_tail + 1u) & (RX_BUF_SIZE - 1u));
    return 1;
}

void UART_Flush(void)
{
    rx_tail = rx_head;
}

/* ---------------- SysTick 1 ms ---------------- */
void SysTick_Handler(void)
{
    tick_ms++;
}

void SysTick_Init(void)
{
    SysTick->CTRL = 0;
    SysTick->LOAD = 8000 - 1;
    SysTick->VAL  = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                    SysTick_CTRL_TICKINT_Msk   |
                    SysTick_CTRL_ENABLE_Msk;
}

uint32_t Millis(void)
{
    return tick_ms;
}

void Delay_ms(uint32_t ms)
{
    uint32_t start = tick_ms;
    while ((uint32_t)(tick_ms - start) < ms) { }
}
