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
#include "system_time.h"
#include "radio_pdu.h"

static radio_pdu_t          m_radio_pdu;
static bool foobar = false;
void Clock_moduleStart(void)
{
    /* Set correct radio frequency and power */
    Radio_setFrequency(1);
    Radio_setPower(RADIO_POWER_MEDIUM);
    while(true)
    {
        uint32_t size = sizeof(m_radio_pdu);
        if(Radio_receive((void *)&m_radio_pdu, &size, 10000000))
        {
            if(!foobar)
            {
                foobar = !foobar;
                System_timeSet(&m_radio_pdu.time, m_radio_pdu.bias);
            }
        }
    }
}

