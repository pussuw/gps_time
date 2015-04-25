/*
 * display.h
 *
 *  Created on: 25.4.2015
 *      Author: Ville
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <stdint.h>

/** Segments */
#define DISPLAY_SEG1_PIN    1
#define DISPLAY_SEG2_PIN    2
#define DISPLAY_SEG3_PIN    3
#define DISPLAY_SEG4_PIN    5
#define DISPLAY_SEG5_PIN    6
#define DISPLAY_SEG6_PIN    7
#define DISPLAY_SEG7_PIN    8
#define NUMBER_OF_SEGMENTS  7

typedef union
{
    struct
    {
        uint32_t a;
        uint32_t b;
        uint32_t c;
        uint32_t d;
        uint32_t e;
        uint32_t f;
        uint32_t g;
    };
    uint32_t raw[7];
}seven_seg_matrix_t;

/** Displays */
#define DISPLAY_1_SEL       11
#define DISPLAY_2_SEL       4
#define DISPLAY_3_SEL       29
#define DISPLAY_4_SEL       28
#define DISPLAY_5_SEL       22
#define DISPLAY_6_SEL       21
#define NUMBER_OF_DISPLAYS  6

typedef union
{
    struct
    {
        uint32_t disp1;
        uint32_t disp2;
        uint32_t disp3;
        uint32_t disp4;
        uint32_t disp5;
        uint32_t disp6;
    };
    uint32_t raw[NUMBER_OF_DISPLAYS];
}display_matrix_t;

#endif /* DISPLAY_H_ */
