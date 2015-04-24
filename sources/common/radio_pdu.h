/*
 * radio_pdu.h
 *
 *  Created on: 24.4.2015
 *      Author: Ville
 */

#ifndef RADIO_PDU_H_
#define RADIO_PDU_H_

#include <stdint.h>
#include "system_time.h"

typedef struct
{
    system_time_t   time;
    uint32_t        bias;
}radio_pdu_t;

#endif /* RADIO_PDU_H_ */
