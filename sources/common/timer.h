/*
 * timer.h
 *
 *  Created on: 23.4.2015
 *      Author: Ville
 */

#ifndef TIMER_H_
#define TIMER_H_

#include <stdint.h>
#include <stdbool.h>

typedef void(*timer_callback_f)(void);

void Timer_init(void);
uint32_t Timer_getCount(void);
void Timer_setTimeout(uint32_t time);
bool Timer_getTimeout(void);
void Timer_clearTimeout(void);
void Timer_waitTimeout(void);
/**
 * Interval in us, callback is mandatory */
bool Timer_registerIntervalCounter(uint32_t interval, timer_callback_f cb);
/** Get bias from timer (time to next tick in microseconds) */
int32_t Timer_timeToIntervalTick(void);
/** Use bias to move first interval back / forth (calibrate system time) */
void Timer_resetIntervalCounter(int32_t bias);
void Timer_delayMicros(uint32_t us);
void Timer_delayMillis(uint32_t ms);

#endif /* TIMER_H_ */
