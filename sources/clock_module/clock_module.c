/*
 * clock_module.c
 *
 *  Created on: 24.4.2015
 *      Author: Ville
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "nrf.h"
#include "radio.h"
#include "timer.h"
#include "system_time.h"
#include "radio_pdu.h"
#include "myassert.h"
#include "debug.h"
#include "display.h"

/** Segments and displays */
static seven_seg_matrix_t   m_7seg_matrix =
{
    .a = DISPLAY_SEG1_PIN,
    .b = DISPLAY_SEG2_PIN,
    .c = DISPLAY_SEG3_PIN,
    .d = DISPLAY_SEG4_PIN,
    .e = DISPLAY_SEG5_PIN,
    .f = DISPLAY_SEG6_PIN,
    .g = DISPLAY_SEG7_PIN,
};

/** Fonts (numbers <=> pins) */
extern seven_seg_matrix_t   m_fonts[];

static display_matrix_t     m_disp_matrix =
{
    .disp1 = DISPLAY_1_SEL,
    .disp2 = DISPLAY_2_SEL,
    .disp3 = DISPLAY_3_SEL,
    .disp4 = DISPLAY_4_SEL,
    .disp5 = DISPLAY_5_SEL,
    .disp6 = DISPLAY_6_SEL,
};

/** This shows the time on the 7-seg displays */
static system_time_t        m_display_time;
static uint32_t             m_time_bias;
static void                 init_gpio(void);
static void                 segment_pin_config(uint32_t pin_number);
static void                 display_time(const system_time_t * time);

/** Radio (GPS time synch) */
static radio_pdu_t          m_radio_pdu;
static void                 packet_received(void * data,
                                            uint32_t len,
                                            uint32_t timestamp);

void Clock_moduleStart(void)
{
    /* Initialize GPIO */
    init_gpio();
    /* Set correct radio frequency and power */
    Radio_setFrequency(1);
    Radio_setPower(RADIO_POWER_MEDIUM);
    /* Turn radio receiver on */
    Radio_receiveAsynch(packet_received, sizeof(m_radio_pdu));
    while(true)
    {
        if(System_timeGet(&m_display_time, &m_time_bias))
        {
            display_time(&m_display_time);
        }
        // else display no clock
    }
}

static void segment_pin_config(uint32_t pin_number)
{
    NRF_GPIO->PIN_CNF[pin_number] =
        (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos) |
        (GPIO_PIN_CNF_DRIVE_H0D1 << GPIO_PIN_CNF_DRIVE_Pos)     |
        (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos)   |
        (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos)  |
        (GPIO_PIN_CNF_DIR_Output << GPIO_PIN_CNF_DIR_Pos);
}

static void init_gpio(void)
{
    uint32_t a;
    for(a = 0; a < NUMBER_OF_SEGMENTS; a++)
    {
        segment_pin_config(m_7seg_matrix.raw[a]);
        nrf_gpio_pin_set(m_7seg_matrix.raw[a]);
    }
    /* Initialize as outputs and pull high (clear all displays) */
    for(a = 0; a < NUMBER_OF_DISPLAYS; a++)
    {
        nrf_gpio_cfg_output(m_disp_matrix.raw[a]);
        nrf_gpio_pin_set(m_disp_matrix.raw[a]);
    }
}

static void select_display(uint32_t display)
{
    uint32_t a;
    for(a = 0; a < NUMBER_OF_DISPLAYS; a++)
    {
        if(a == display)
        {
            nrf_gpio_pin_clear(m_disp_matrix.raw[a]);
        }
        else
        {
            nrf_gpio_pin_set(m_disp_matrix.raw[a]);
        }
    }
}

static void display_time(const system_time_t * time)
{
    uint8_t a,b,c;
    uint8_t numbers[6];
    seven_seg_matrix_t * font;
    /* Convert time to display format: avoid modulo (double div)
     * Cortex-M0 has no DIV instruction => MUL is faster */
    numbers[0] = time->hours / 10;
    numbers[1] = time->hours - (numbers[0] * 10);
    /* Minutes */
    numbers[2] = time->minutes / 10;
    numbers[3] = time->minutes - (numbers[2] * 10);
    /* Seconds */
    numbers[4] = time->seconds / 10;
    numbers[5] = time->seconds - (numbers[4] * 10);
    /* Start scanning from hours */
    for(a = 0; a < NUMBER_OF_DISPLAYS; a++)
    {
        /* First setup 7-seg correctly */
        c = numbers[a];
        assert(c < 10);
        font = &m_fonts[c];
        /* Clear all displays */
        select_display(NUMBER_OF_DISPLAYS);
        for(b = 0; b < NUMBER_OF_SEGMENTS; b++)
        {
            if(font->raw[b])
            {
                nrf_gpio_pin_clear(m_7seg_matrix.raw[b]);
            }
            else
            {
                nrf_gpio_pin_set(m_7seg_matrix.raw[b]);
            }
        }
        select_display(a);
    }
    /* Clear all displays */
    select_display(NUMBER_OF_DISPLAYS);
}

static void packet_received(void * data, uint32_t len, uint32_t timestamp)
{
    /* Got new time from radio */
    assert(len == sizeof(m_radio_pdu));
    memcpy((void *)&m_radio_pdu, data, sizeof(m_radio_pdu));
    System_timeSet(&m_radio_pdu.time, 0, timestamp, m_radio_pdu.bias);
}
