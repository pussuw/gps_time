/*
 * fonts.c
 *
 *  Created on: 25.4.2015
 *      Author: Ville
 */

#include "display.h"

seven_seg_matrix_t   m_fonts[] =
{
    /* 0: Display nothing */
    {  },
    /* 1: BC */
    { .b = 1, .c = 1 },
    /* 2: ABDEG */
    { .a = 1, .b = 1, .d = 1, .e = 1, .g = 1 },
    /* 3: ABCDG */
    { .a = 1, .b = 1, .c = 1, .d = 1, .g = 1 },
    /* 4: BCFG */
    { .b = 1, .c = 1, .f = 1, .g = 1 },
    /* 5: ACDFG */
    { .a = 1, .c = 1, .d = 1, .f = 1, .g = 1 },
    /* 6: ACDEFG */
    { .a = 1, .c = 1, .d = 1, .e = 1, .f = 1, .g = 1 },
    /* 7: ABC */
    { .a = 1, .b = 1, .c = 1 },
    /* 8: ALL */
    { .a = 1, .b = 1, .c = 1, .d = 1, .e = 1, .f = 1, .g = 1 },
    /* 9: ABCDFG */
    { .a = 1, .b = 1, .c = 1, .d = 1, .f = 1, .g = 1},
};
