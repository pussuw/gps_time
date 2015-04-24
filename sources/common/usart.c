/*
 * usart.c
 *
 *  Created on: 23.4.2015
 *      Author: Ville
 */

#include <stdint.h>
#include <stdbool.h>

#include "nrf.h"
#include "hal.h"
#include "usart.h"
#include "interrupt.h"

#define RECEIVE_BUFFER          1024

#if RECEIVE_BUFFER < 2
#error RECEIVE_BUFFER is too small.  It must be larger than 1.
#elif ((RECEIVE_BUFFER & (RECEIVE_BUFFER-1)) != 0)
#error RECEIVE_BUFFER must be a power of 2.
#endif

typedef struct
{
    uint16_t    in;                    // Next In Index
    uint16_t    out;                   // Next Out Index
    uint8_t     buf[RECEIVE_BUFFER];   // Buffer
} ring_buffer_t;

#define RINGBUFFER_ELEMENTS(p)  ((uint16_t)((p).in - (p).out))

/* Serial RX ringbuffer */
static ring_buffer_t            m_uart_rx_buf = {0, 0, };

/** Initialize serial port: baud = 9600 */
void Usart_init(void)
{
    /* GPIO init */
    NRF_UART0->PSELTXD = HAL_USART_TX_PIN;
    NRF_UART0->PSELRXD = HAL_USART_RX_PIN;
    NRF_UART0->TASKS_STOPTX = 1;
    NRF_UART0->TASKS_STOPRX = 1;
    NRF_UART0->ENABLE = UART_ENABLE_ENABLE_Disabled;
    /* Transmitter is always off so configure pin as output and pull high */
    nrf_gpio_cfg_output(HAL_USART_TX_PIN);
    nrf_gpio_pin_set(HAL_USART_TX_PIN);
    nrf_gpio_cfg_input(HAL_USART_RX_PIN, NRF_GPIO_PIN_PULLUP);
    /* Set baudrate at 9600 */
    NRF_UART0->BAUDRATE = (uint32_t)UART_BAUDRATE_BAUDRATE_Baud9600;
    NRF_UART0->EVENTS_RXDRDY = 0;
    NRF_UART0->EVENTS_TXDRDY = 0;
    /* Interrupt init */
    /* This bitmask is from nrf examples, because the reference manual
     * simply does not contain the register information... */
    NRF_UART0->INTENCLR = 0xffffffffUL;
    NRF_UART0->INTENSET =
    (UART_INTENSET_RXDRDY_Set << UART_INTENSET_RXDRDY_Pos) |
    (UART_INTENSET_ERROR_Set << UART_INTENSET_ERROR_Pos) |
    (UART_INTENSET_RXTO_Set << UART_INTENSET_RXTO_Pos);
    /* NVIC */
    NVIC_ClearPendingIRQ(UART0_IRQn);
    NVIC_SetPriority(UART0_IRQn, 0);
    NVIC_EnableIRQ(UART0_IRQn);
}

void Usart_enableReceiver(void)
{
    NRF_UART0->TASKS_STARTRX = 1;
}

void Usart_disableReceiver(void)
{
    NRF_UART0->TASKS_STOPRX = 1;
}

uint8_t Usart_read(bool * empty)
{
    uint8_t ret = 0;
    ring_buffer_t *p = &m_uart_rx_buf;
    *empty = false;
    Interrupt_disableAll();
    if (RINGBUFFER_ELEMENTS(*p) == 0)
    {
        /* Buffer empty */
        *empty = true;
    }
    else
    {
        /* Return item from tail */
        ret = (p->buf[(p->out++) & (RECEIVE_BUFFER - 1)]);
    }
    Interrupt_enableAll();
    return ret;
}

void __attribute__((__interrupt__)) UART0_IRQHandler(void)
{
    // Character received ?
    if (NRF_UART0->EVENTS_RXDRDY != 0)
    {
        ring_buffer_t * p = &m_uart_rx_buf;
        uint8_t ch;
        // Clear event first
        NRF_UART0->EVENTS_RXDRDY  = 0;
        ch = NRF_UART0->RXD;
        if (((p->in - p->out) & ~(RECEIVE_BUFFER - 1)) == 0)
        {
            /* Insert new item to head */
            p->buf[p->in & (RECEIVE_BUFFER - 1)] = (uint8_t) (ch & 0xFF);
            p->in++;
        }
    }
    // Handle errors.
    if (NRF_UART0->EVENTS_ERROR != 0)
    {
        NRF_UART0->EVENTS_ERROR = 0;
    }
    if (NRF_UART0->EVENTS_RXTO != 0)
    {
        NRF_UART0->EVENTS_RXTO = 0;
    }
}
