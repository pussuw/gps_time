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
/** Option for setting local time offset to UTC */
void System_timeSet(const system_time_t * time,
                    int32_t  local_time,
                    uint32_t timestamp,
                    uint32_t bias);
bool System_timeGet(system_time_t * time, uint32_t * bias);

#endif /* SYSTEM_TIME_H_ */
