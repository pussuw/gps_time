/*
 * usart.c
 *
 *  Created on: 23.4.2015
 *      Author: Ville
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "nrf.h"
#include "hal.h"
#include "usart.h"
#include "interrupt.h"
#include "myassert.h"

/* Serial RX callback */
static volatile usart_rx_cb_f   m_rx_callback;

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
    nrf_gpio_cfg_input(HAL_USART_RX_PIN, NRF_GPIO_PIN_NOPULL);
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
    NVIC_SetPriority(UART0_IRQn, HAL_USART_INTERRUPT_PRIO);
    NVIC_EnableIRQ(UART0_IRQn);
    NRF_UART0->ENABLE = UART_ENABLE_ENABLE_Enabled;
}

void Usart_send(const void * buffer, uint32_t length)
{
    uint8_t * p = (uint8_t *)buffer;
    assert(buffer != NULL);
    NRF_UART0->TASKS_STARTTX = 1;
    while(length--)
    {
        NRF_UART0->TXD = *p++;
        while(!NRF_UART0->EVENTS_TXDRDY)
        {
        }
        NRF_UART0->EVENTS_TXDRDY = 0;
    }
    NRF_UART0->TASKS_STOPTX = 1;
}

void Usart_setupReceiver(usart_rx_cb_f cb)
{
    assert(cb != NULL);
    Interrupt_disableAll();
    m_rx_callback = cb;
    NRF_UART0->TASKS_STARTRX = 1;
    Interrupt_enableAll();
}

void __attribute__((__interrupt__)) UART0_IRQHandler(void)
{
    // Character received ?
    if (NRF_UART0->EVENTS_RXDRDY != 0)
    {
        uint8_t ch;
        // Clear event first
        NRF_UART0->EVENTS_RXDRDY  = 0;
        ch = NRF_UART0->RXD;
        if(m_rx_callback)
        {
            m_rx_callback(ch);
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
