/*
 * timer.c
 *
 *  Created on: 23.4.2015
 *      Author: Ville
 */

#include <stdint.h>
#include <stddef.h>
#include "nrf.h"
#include "timer.h"
#include "hal.h"
#include "interrupt.h"
#include "myassert.h"

/* Prescaler to achieve 1MHz frequency
 * f(timer) = HFCLK / 2^psc
 * Calculate prescaler and initialize timer
 * Two possible prescalers:
 * 2^psc = 16 if clock = 16MHz -> psc = 4
 * 2^psc = 32 if clock = 32MHz -> psc = 5 */
#if (__SYSTEM_CLOCK == 16000000)
#define TIMER_PRESCALER             4
#else
#define TIMER_PRESCALER             5
#endif

/* Capture / Compare channels */
#define TIMER_COUNT_CC              0
#define TIMER_TIMEOUT_CC            1
#define TIMER_INTERVAL_CC           2

/* Some margin for timeouts, so that waiting does not take forever (4300s) */
#define MINIMUM_TIMEOUT_TIME        5

/* Interval service */
#define MINIMUM_INTERVAL_TIME       1000
#define MINIMUM_BIAS_MARGIN         10
#define INTERVAL_TIMER_ENABLE       TIMER_INTENSET_COMPARE2_Msk
#define INTERVAL_TIMER_DISABLE      TIMER_INTENCLR_COMPARE2_Msk
static volatile uint32_t            m_interval_time;
static volatile timer_callback_f    m_interval_cb;

void Timer_init(void)
{
    uint32_t i;
    /* Stop timer and initialize */
    NRF_TIMER0->TASKS_STOP = 1;
    NRF_TIMER0->TASKS_CLEAR = 1;
    /* Clear all CC channels */
    for(i = 0; i < 4; i++)
    {
        NRF_TIMER0->EVENTS_COMPARE[i] = 0;
        NRF_TIMER0->CC[i] = 0;
    }
    /* Shortcuts between compare match and tasks disabled */
    NRF_TIMER0->SHORTS = 0;
    NRF_TIMER0->MODE = TIMER_MODE_MODE_Timer;
    NRF_TIMER0->PRESCALER = TIMER_PRESCALER;
    NRF_TIMER0->BITMODE = TIMER_BITMODE_BITMODE_32Bit;
    NRF_TIMER0->INTENCLR = 0xFFFFFFFF;
    /* Enable NVIC channel for timer */
    NVIC_ClearPendingIRQ(TIMER0_IRQn);
    NVIC_SetPriority(TIMER0_IRQn, HAL_TIMER_INTERRUPT_PRIO);
    NVIC_EnableIRQ(TIMER0_IRQn);
    /* Start timer */
    NRF_TIMER0->TASKS_START = 1;
}

uint32_t Timer_getCount(void)
{
    NRF_TIMER0->TASKS_CAPTURE[TIMER_COUNT_CC] = 1;
    return NRF_TIMER0->CC[TIMER_COUNT_CC];
}

void Timer_setTimeout(uint32_t time)
{
    /* Lets make sure timeout is not  too close */
    time += MINIMUM_TIMEOUT_TIME;
    NRF_TIMER0->CC[TIMER_TIMEOUT_CC] = time;
    NRF_TIMER0->EVENTS_COMPARE[TIMER_TIMEOUT_CC] = 0;
}

bool Timer_getTimeout(void)
{
    volatile uint32_t timeout;
    /* Lets use a latch so that stack unwinding doesn't do something magical */
    timeout = NRF_TIMER0->EVENTS_COMPARE[TIMER_TIMEOUT_CC];
    return (bool)(timeout != 0);
}

void Timer_clearTimeout(void)
{
    NRF_TIMER0->EVENTS_COMPARE[TIMER_TIMEOUT_CC] = 0;
}

void Timer_waitTimeout(void)
{
    while(!Timer_getTimeout())
    {
    }
    Timer_clearTimeout();
}

bool Timer_registerIntervalCounter(uint32_t interval, timer_callback_f cb)
{
    bool ret = false;
    Interrupt_disableAll();
    if((interval >= MINIMUM_INTERVAL_TIME) && (cb != NULL))
    {
        m_interval_time = interval;
        m_interval_cb = cb;
        interval = Timer_getCount() + interval;
        NRF_TIMER0->CC[TIMER_INTERVAL_CC] = interval;
        NRF_TIMER0->EVENTS_COMPARE[TIMER_INTERVAL_CC] = 0;
        NRF_TIMER0->INTENSET = INTERVAL_TIMER_ENABLE;
        ret = true;
    }
    Interrupt_enableAll();
    return ret;
}

int32_t Timer_timeSinceInterval(void)
{
    uint32_t time, set_time;
    assert(m_interval_cb != NULL);
    Interrupt_disableAll();
    time = Timer_getCount();
    /* Get current set time */
    set_time = NRF_TIMER0->CC[TIMER_INTERVAL_CC];
    /* Last tick is now - interval */
    set_time -= m_interval_time;
    /* Calculate diff: this can be negative */
    time = time - set_time;
    Interrupt_enableAll();
    return (int32_t)time;
}

void Timer_resetIntervalCounter(uint32_t bias)
{
    uint32_t time;
    assert(m_interval_cb != NULL);
    assert(bias <= m_interval_time);
    Interrupt_disableAll();
    time = Timer_getCount();
    /* Calculate how much time is left after removing bias*/
    bias = m_interval_time - bias;
    if(bias < MINIMUM_BIAS_MARGIN)
    {
        /* Must have some margin between current time and next ISR */
        bias = MINIMUM_BIAS_MARGIN;
    }
    /* Move time forward */
    time += bias;
    NRF_TIMER0->CC[TIMER_INTERVAL_CC] = time;
    NRF_TIMER0->EVENTS_COMPARE[TIMER_INTERVAL_CC] = 0;
    Interrupt_enableAll();
}

void __attribute__((optimize("-O0"))) Timer_delayMicros(uint32_t us)
{
    volatile uint32_t start = Timer_getCount();
    while ((Timer_getCount() - start) < us);
}

void __attribute__((optimize("-O0"))) Timer_delayMillis(uint32_t ms)
{
    while (ms-- > 0) Timer_delayMicros(999);
}

void __attribute__((__interrupt__)) TIMER0_IRQHandler(void)
{
    if(NRF_TIMER0->EVENTS_COMPARE[TIMER_INTERVAL_CC])
    {
        uint32_t time;
        NRF_TIMER0->EVENTS_COMPARE[TIMER_INTERVAL_CC] = 0;
        time = NRF_TIMER0->CC[TIMER_INTERVAL_CC];
        time += m_interval_time;
        NRF_TIMER0->CC[TIMER_INTERVAL_CC] = time;
        if(m_interval_cb)
        {
            m_interval_cb();
        }
    }
}
