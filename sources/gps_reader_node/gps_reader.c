/*
 * gps_reader.c
 *
 *  Created on: 23.4.2015
 *      Author: Ville
 */

#include <stdint.h>
#include <stdbool.h>

#include "radio.h"
#include "usart.h"
#include "timer.h"
void Gps_moduleStart(void)
{
    /* Entry point for GPS time seed module */
    Radio_setFrequency(1);
    Radio_setPower(RADIO_POWER_MEDIUM);
    while(1)
    {
        Timer_delayMillis(2);
    }
}
