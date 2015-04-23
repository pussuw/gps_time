/*
 * start.c
 *
 *  Created on: 20.3.2015
 *      Author: Ville
 */

/* Global variables */
extern unsigned int __data_src_start__;
extern unsigned int __data_start__;
extern unsigned int __data_end__;
extern unsigned int __bss_start__;
extern unsigned int __bss_end__;

/* Application entry point */
extern int main(void);

void _start(void)
{
    unsigned int * src, * dst;

    for(src = &__data_src_start__,
        dst = &__data_start__;
        dst != &__data_end__;)
    {
        *dst++ = *src++;
    }
    for(dst = &__bss_start__; dst != &__bss_end__;)
    {
        *dst++ = 0;
    }
    /* Call user entry point */
    (void)main();

    /* Main should not return, but lets not play with faith */
    while(1);
}
