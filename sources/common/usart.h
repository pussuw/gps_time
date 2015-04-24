/*
 * usart.h
 *
 *  Created on: 23.4.2015
 *      Author: Ville
 */

#ifndef USART_H_
#define USART_H_

#include <stdint.h>
#include <stdbool.h>

typedef void(*usart_rx_cb_f)(uint8_t ch);

void Usart_init(void);
void Usart_send(const void * buffer, uint32_t length);
void Usart_setupReceiver(usart_rx_cb_f cb);

#endif /* USART_H_ */
