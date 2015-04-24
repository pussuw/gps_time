/*
 * debug.c
 *
 *  Created on: 24.4.2015
 *      Author: Ville
 */

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>

#include "usart.h"

/* Simple and quite light formatted print routines for debugging */
#define DEBUG_BUFFER_SIZE   1024
static char                 m_print_buffer[DEBUG_BUFFER_SIZE];

void * _malloc_r(size_t size)
{
    return NULL;
}

void _free_r(void * ptr)
{

}

void * _realloc_r(struct _reent * a, void * b, size_t c)
{
    return NULL;
}

int Debug_printf(const char * fmt, ...)
{
    int ret;
    va_list args;
    va_start(args, fmt);
    ret = vsnprintf(m_print_buffer, DEBUG_BUFFER_SIZE, fmt, args);
    va_end(args);
    Usart_send((void *)m_print_buffer, ret);
    return ret;
}
