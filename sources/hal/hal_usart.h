/*
 * hal_usart.h
 *
 *  Created on: 23.4.2015
 *      Author: Ville
 */

#ifndef HAL_USART_H_
#define HAL_USART_H_

#if defined GPS_TIME_MODULE
#define HAL_USART_TX_PIN    29
#define HAL_USART_RX_PIN    0
#elif defined CLOCK_MODULE
// Both are NC atm
#define HAL_USART_TX_PIN    10
#define HAL_USART_RX_PIN    0
#else
#error "HAL cannot determine platform"
#endif

#endif /* HAL_USART_H_ */
