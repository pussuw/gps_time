/*
 * system_time.h
 *
 *  Created on: 23.4.2015
 *      Author: Ville
 */

#ifndef SYSTEM_TIME_H_
#define SYSTEM_TIME_H_

#include <stdint.h>
#include <stdbool.h>

/** Do not modify structure order, time setting assumes certain form */
typedef struct
{
    uint32_t    hours;
    uint32_t    minutes;
    uint32_t    seconds;
}system_time_t;

void System_timeInit(void);
void System_timeSetGps(const uint8_t * gps_time, int32_t bias);
void System_timeSet(system_time_t * time, int32_t bias);
bool System_timeGet(system_time_t * time, int32_t * bias);

#endif /* SYSTEM_TIME_H_ */
