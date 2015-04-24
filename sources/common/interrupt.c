/*
 * interrupt.c
 *
 *  Created on: 24.4.2015
 *      Author: Ville
 */

#include <stdint.h>
#include "interrupt.h"
#include "nrf.h"
#include "myassert.h"

static volatile uint32_t    m_interrupt_mutex;

void Interrupt_disableAll(void)
{
    assert(m_interrupt_mutex < 0xFF);
    __disable_irq();
    m_interrupt_mutex++;
}

void Interrupt_enableAll(void)
{
    assert(m_interrupt_mutex > 0);
    m_interrupt_mutex--;
    if(m_interrupt_mutex == 0)
    {
        __enable_irq();
    }
}
