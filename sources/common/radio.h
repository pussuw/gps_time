/*
 * radio.h
 *
 *  Created on: 23.4.2015
 *      Author: Ville
 */

#ifndef RADIO_H_
#define RADIO_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    RADIO_POWER_MINIMUM, // -30dBm
    RADIO_POWER_MEDIUM,  // -12dBm
    RADIO_POWER_MAX      //  +4dBm
}radio_power_levels_e;

void Radio_init(void);
bool Radio_send(const void * data, uint32_t len);
bool Radio_receive(void * data, uint32_t * len, uint32_t timeout);
void Radio_setFrequency(uint32_t frequency);
void Radio_setPower(radio_power_levels_e power);

#endif /* RADIO_H_ */
