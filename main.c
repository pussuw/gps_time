/*
 * main.c
 *
 *  Created on: 20.3.2015
 *      Author: Ville
 */

volatile int test_global = 0xdeadbeef;
volatile int test_global_nul;
volatile int test_global_inc;

int main(void)
{
    (void)test_global;
    (void)test_global_nul;
    (void)test_global_inc;
    while(1)
    {
        test_global_inc++;
        /* Stop main thread here */
    }
    return 0;
}
