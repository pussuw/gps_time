/*
 * app.h
 *
 *  Created on: 23.4.2015
 *      Author: Ville
 */

#ifndef APP_H_
#define APP_H_

#if defined (GPS_TIME_MODULE)
#include "gps_reader.h"
#define App_start() Gps_moduleStart()
#elif defined (CLOCK_MODULE)
#include "clock_module.h"
#define App_start() Clock_moduleStart()
#else
#error "No target specified!"
#endif
#endif /* APP_H_ */
