/*
 * system_time.c
 *
 *  Created on: 23.4.2015
 *      Author: Ville
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>

#include "system_time.h"
#include "timer.h"
#include "interrupt.h"
#include "myassert.h"

/** Time keeping */
#define SYSTEM_TIME_TICK    1000000u
static system_time_t        m_current_time;
static bool                 m_time_acquired;
static void                 second_timer(void);

void System_timeInit(void)
{
    memset((void *)&m_current_time, 0, sizeof(m_current_time));
    Timer_registerIntervalCounter(SYSTEM_TIME_TICK, second_timer);
}

/* GPS format is HHMMSS */
void System_timeSetGps(const uint8_t * gps_time, int32_t bias)
{
    uint8_t * in = (uint8_t *)gps_time;
    uint32_t * out = (uint32_t *)&m_current_time;
    uint8_t dec01, dec10, index;
    assert(gps_time != NULL);
    Interrupt_disableAll();
    for(index = 0; index < 3; index++)
    {
        dec10 = *in++;
        dec01 = *in++;
        dec01 -= '0';
        dec10 -= '0';
        dec10 *= 10;
        dec10 += dec01;
        *out++ = dec10;
    }
    Timer_resetIntervalCounter(bias);
    m_time_acquired = true;
    Interrupt_enableAll();
}

void System_timeSet(const system_time_t * time, int32_t bias)
{
    assert(time != NULL);
    Interrupt_disableAll();
    m_current_time.seconds = time->seconds;
    m_current_time.minutes = time->minutes;
    m_current_time.hours = time->hours;
    Timer_resetIntervalCounter(bias);
    m_time_acquired = true;
    Interrupt_enableAll();
}

bool System_timeGet(system_time_t * time, int32_t * bias)
{
    assert(time != NULL);
    assert(bias != NULL);
    Interrupt_disableAll();
    time->seconds = m_current_time.seconds;
    time->minutes = m_current_time.minutes;
    time->hours = m_current_time.hours;
    *bias = Timer_timeToIntervalTick();
    Interrupt_enableAll();
    return m_time_acquired;
}

static void second_timer(void)
{
    m_current_time.seconds++;
    if(m_current_time.seconds == 60)
    {
        m_current_time.seconds = 0;
        m_current_time.minutes++;
    }
    if(m_current_time.minutes == 60)
    {
        m_current_time.minutes = 0;
        m_current_time.hours++;
    }
    if(m_current_time.hours == 24)
    {
        m_current_time.hours = 0;
    }
}
