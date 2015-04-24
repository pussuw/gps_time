/*
 * interrupt.c
 *
 *  Created on: 24.4.2015
 *      Author: Ville
 */

#include <stdint.h>
#include "interrupt.h"
#include "nrf.h"

static volatile uint32_t    m_interrupt_mutex;

void Interrupt_disableAll(void)
{
    __disable_irq();
    m_interrupt_mutex++;
}

void Interrupt_enableAll(void)
{
    m_interrupt_mutex--;
    if(m_interrupt_mutex == 0)
    {
        __enable_irq();
    }
}
