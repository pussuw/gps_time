/*
 * main.c
 *
 *  Created on: 20.3.2015
 *      Author: Ville
 */

#include "nrf.h"
#include "app.h"
#include "timer.h"
#include "radio.h"

int main(void)
{
    /* Power on all RAM banks */
    NRF_POWER->RAMON |= POWER_RAMON_ONRAM0_Msk |
                        POWER_RAMON_ONRAM1_Msk;
    NRF_POWER->RAMONB |= POWER_RAMON_ONRAM2_Msk |
                         POWER_RAMON_ONRAM3_Msk;
    /* Use 16MHz external XTAL */
    NRF_CLOCK->XTALFREQ = CLOCK_XTALFREQ_XTALFREQ_16MHz;
    /* Start clock and wait for it to stabilize */
    NRF_CLOCK->TASKS_HFCLKSTART = 1;
    while (!NRF_CLOCK->EVENTS_HFCLKSTARTED)
    {
    }
    /* Initialize common hardware */
    Timer_init();
    Radio_init();
    /* Start application task */
    App_start();
    return 0;
}
