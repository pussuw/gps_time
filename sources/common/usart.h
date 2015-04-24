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

void Usart_init(void);
void Usart_enableReceiver(void);
void Usart_disableReceiver(void);
void Usart_clearReceiver(void);
uint8_t Usart_read(bool * empty);

#endif /* USART_H_ */
