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

void Timer_init(void);
uint32_t Timer_getCount(void);
void Timer_setTimeout(uint32_t time);
bool Timer_getTimeout(void);
void Timer_clearTimeout(void);
void Timer_waitTimeout(void);
void Timer_delayMicros(uint32_t us);
void Timer_delayMillis(uint32_t ms);

#endif /* TIMER_H_ */
