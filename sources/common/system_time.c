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
#define SYSTEM_TIME_TICK        1000000u
static volatile system_time_t   m_current_time;
static volatile bool            m_time_acquired;
static void                     second_timer(void);
static void                     increment_time(system_time_t * time);
static uint32_t                 calibrate_time(system_time_t * time,
                                               int32_t system_bias);
static void                     utc_to_local_time(int32_t hours);

void System_timeInit(void)
{
    memset((void *)&m_current_time, 0, sizeof(m_current_time));
    Timer_registerIntervalCounter(SYSTEM_TIME_TICK, second_timer);
}

void System_timeSet(const system_time_t * time,
                    int32_t  local_time,
                    uint32_t timestamp,
                    uint32_t bias)
{
    assert(gps_time != NULL);
    Interrupt_disableAll();
    m_current_time.seconds = time->seconds;
    m_current_time.minutes = time->minutes;
    m_current_time.hours = time->hours;
    utc_to_local_time(local_time);
    /* How much time has gone since timestamp acquired ? */
    timestamp = Timer_getCount() - timestamp;
    /* How much bias with given time ? */
    timestamp += bias;
    timestamp = calibrate_time((system_time_t *)&m_current_time, timestamp);
    Timer_resetIntervalCounter(timestamp);
    m_time_acquired = true;
    Interrupt_enableAll();
}

bool System_timeGet(system_time_t * time, uint32_t * bias)
{
    assert(time != NULL);
    assert(bias != NULL);
    Interrupt_disableAll();
    time->seconds = m_current_time.seconds;
    time->minutes = m_current_time.minutes;
    time->hours = m_current_time.hours;
    *bias = calibrate_time(time, Timer_timeToIntervalTick());
    Interrupt_enableAll();
    return m_time_acquired;
}

static void second_timer(void)
{
    increment_time((system_time_t *)&m_current_time);
}

static void increment_time(system_time_t * time)
{
    time->seconds++;
    if(time->seconds == 60)
    {
        time->seconds = 0;
        time->minutes++;
    }
    if(time->minutes == 60)
    {
        time->minutes = 0;
        time->hours++;
    }
    if(time->hours == 24)
    {
        time->hours = 0;
    }
}

static uint32_t calibrate_time(system_time_t * time, int32_t system_bias)
{
    if(system_bias < 0)
    {
        /* If system is in a long critical, bias can be negative */
        while(system_bias < 0)
        {
            /* Worst case: bias is over 1 second (clock is late) */
            increment_time(time);
            system_bias += SYSTEM_TIME_TICK;
        }
    }
    else if(system_bias > 0)
    {
        /* Normally bias is just added to the clock (propagation delay) */
        while(system_bias >= SYSTEM_TIME_TICK)
        {
            increment_time(time);
            system_bias -= SYSTEM_TIME_TICK;
        }
    }
    return system_bias;
}

static void utc_to_local_time(int32_t hours)
{
    if(hours < 0)
    {
        m_current_time.hours -= hours;
        if(m_current_time.hours < 0)
        {
            m_current_time.hours += 24;
        }
    }
    else if(hours > 0)
    {
        m_current_time.hours += hours;
        if(m_current_time.hours >= 24)
        {
            m_current_time.hours -= 24;
        }
    }
}
