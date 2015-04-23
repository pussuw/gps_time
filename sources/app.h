/*
 * app.h
 *
 *  Created on: 23.4.2015
 *      Author: Ville
 */

#ifndef APP_H_
#define APP_H_

#include "gps_reader.h"

#if defined (GPS_TIME_MODULE)
#define App_start() Gps_moduleStart()
#elif defined (CLOCK_MODULE)
#define App_start() Clock_moduleStart()
#else
#error "No target specified!"
#endif
#endif /* APP_H_ */
