/*
 * clock_module.c
 *
 *  Created on: 24.4.2015
 *      Author: Ville
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "nrf.h"
#include "radio.h"
#include "timer.h"
#include "system_time.h"
#include "radio_pdu.h"
#include "myassert.h"
#include "debug.h"

#include "usart.h" // @todo

static radio_pdu_t  m_radio_pdu;
static bool         m_time_valid;
static void         packet_received(void * data,
                                    uint32_t len,
                                    uint32_t timestamp);

void Clock_moduleStart(void)
{
    /* Initialize USART */
    Usart_init();
    /* Set correct radio frequency and power */
    Radio_setFrequency(1);
    Radio_setPower(RADIO_POWER_MEDIUM);
    /* Turn radio receiver on */
    Radio_receiveAsynch(packet_received, sizeof(m_radio_pdu));
    while(true)
    {
        m_time_valid = System_timeGet(&m_radio_pdu.time, &m_radio_pdu.bias);
        Debug_printf("TIME: %02u:%02u:%02u | bias: %u | valid = %u\n",
                     m_radio_pdu.time.hours,
                     m_radio_pdu.time.minutes,
                     m_radio_pdu.time.seconds,
                     m_radio_pdu.bias,
                     m_time_valid);
        Timer_delayMillis(1000);
    }
}

static void packet_received(void * data, uint32_t len, uint32_t timestamp)
{
    /* Got new time from radio */
    assert(len == sizeof(m_radio_pdu));
    memcpy((void *)&m_radio_pdu, data, sizeof(m_radio_pdu));
    System_timeSet(&m_radio_pdu.time, 0, timestamp, m_radio_pdu.bias);
}
