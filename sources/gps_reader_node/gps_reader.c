/*
 * gps_reader.c
 *
 *  Created on: 23.4.2015
 *      Author: Ville
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "nrf.h"
#include "radio.h"
#include "usart.h"
#include "timer.h"
#include "system_time.h"
#include "radio_pdu.h"
#include "debug.h"
#include "myassert.h"

/** GPIO
 *  28: GPS ON/OFF  : OUT PP
 *  04: GPS RST     : OUT OD
 *  27: GPS_1PPS    : IN FLOAT
 */
#define GPIO_PIN_ON_OFF     28
#define GPIO_PIN_RST        4
#define GPIO_PIN_1PPS       27
static void                 init_gpio(void);

/**
 * GPS module
 * $GPZDA,hhmmss.ss,dd,mm,yyyy,xx,yy*CC
 * $GPZDA,201530.00,04,07,2002,00,00*60
 * where:
 * hhmmss    HrMinSec(UTC)
 * dd,mm,yyy Day,Month,Year
 * xx        local zone hours -13..13
 * yy        local zone minutes 0..59
 * *CC       checksum
 */
#define GPS_MODULE_PWR_CYC  250u
#define GPS_MINIMUM_MSG_LEN 20u
#define GPS_TIME_MNEMONIC   "GPZDA"
#define GPS_TIME_START      '$'
#define GPS_TIME_SEPARATOR  ','
#define GPS_CHECKSUM_ID     '*'
#define GPS_TIME_TERMINATOR '\n'
static volatile uint32_t    m_msg_timestamp;
static void                 init_gps_module(void);
static bool                 verify_checksum(uint8_t * data);
static void                 frame_completed(void);

/** UART RX (intermediate buffer for NMEA parsing) */
#define UART_BUFFER_SIZE    128
static volatile uint8_t     m_uart_buffer[UART_BUFFER_SIZE];
static volatile uint8_t     m_uart_buffer_idx;
static void                 uart_rx_callback(uint8_t ch);

/** Time sharing via radio */
#define TIME_SHARE_INTERVAL 1000000u // 1 second
static uint32_t             m_last_tx_time;
static radio_pdu_t          m_radio_pdu;

/* Entry point for GPS time seed module */
void Gps_moduleStart(void)
{
    /* Initialize USART */
    Usart_init();
    Usart_setupReceiver(uart_rx_callback);
    /* Initialize proper GPIO modes for GPS module */
    init_gpio();
    /* Initialize GPS module */
    init_gps_module();
    /* Set correct radio frequency and power */
    Radio_setFrequency(1);
    Radio_setPower(RADIO_POWER_MEDIUM);
    while(1)
    {
        uint32_t now = Timer_getCount();
        /* Don't share time if system time is not valid */
        if(System_timeGet(&m_radio_pdu.time, &m_radio_pdu.bias))
        {
            if((now - m_last_tx_time) >= TIME_SHARE_INTERVAL)
            {
                (void)Radio_send((void *)&m_radio_pdu, sizeof(radio_pdu_t));
                m_last_tx_time = now;
            }
        }
    }
}

static void init_gpio(void)
{
    nrf_gpio_cfg_output(GPIO_PIN_ON_OFF);
    nrf_gpio_pin_clear(GPIO_PIN_ON_OFF);
    /* Reset, set output and assert */
    nrf_gpio_cfg_output(GPIO_PIN_RST);
    nrf_gpio_pin_clear(GPIO_PIN_RST);
    /* 1PPS is insignificant: might have a lot of bias */
    nrf_gpio_cfg_input(GPIO_PIN_1PPS, NRF_GPIO_PIN_NOPULL);
}

static void init_gps_module(void)
{
    /* Release from reset */
    nrf_gpio_pin_set(GPIO_PIN_RST);
    /* Power cycle chip */
    nrf_gpio_pin_set(GPIO_PIN_ON_OFF);
    Timer_delayMillis(GPS_MODULE_PWR_CYC);
    nrf_gpio_pin_clear(GPIO_PIN_ON_OFF);
}

static bool verify_checksum(uint8_t * data)
{
    uint8_t c1 = 0, c2 = 0xFF;
    uint8_t * p;
    assert(data != NULL);
    /* Try to find delimiter */
    p = (uint8_t *)strchr((char *)data, GPS_CHECKSUM_ID);
    if(p != NULL)
    {
        /* Found: initialize checksum */
        c2 = strtol((char *)p + 1, NULL, 16);
        while(data != p)
        {
            c1 ^= *data++;
        }
    }
    return (bool)(c1 == c2);
}

static void frame_completed(void)
{
    if(m_uart_buffer_idx >= GPS_MINIMUM_MSG_LEN)
    {
        uint8_t * mnemonic;
        uint8_t * msg = (uint8_t *)m_uart_buffer;
        /* First calculate checksum */
        if(verify_checksum(msg))
        {
            /* Checksum OK, start parsing */
            mnemonic = (uint8_t *)strchr((char *)msg, GPS_TIME_SEPARATOR);
            if(mnemonic != NULL)
            {
                *mnemonic = '\0';
                if(strcmp((char *)msg, GPS_TIME_MNEMONIC) == 0)
                {
                    /* Correct mnemonic: set system time*/
                    mnemonic++;
                    System_timeSetGps(mnemonic, NULL, m_msg_timestamp);
                }
            }
        }
    }
}

static void uart_rx_callback(uint8_t ch)
{
    if(ch == GPS_TIME_START)
    {
        /* New frame is starting */
        m_uart_buffer_idx = 0;
    }
    else if(ch == GPS_TIME_TERMINATOR)
    {
        /* Frame is done now: terminate string */
        m_msg_timestamp = Timer_getCount();
        m_uart_buffer[m_uart_buffer_idx] = '\0';
        frame_completed();
    }
    else if(m_uart_buffer_idx < UART_BUFFER_SIZE)
    {
        m_uart_buffer[m_uart_buffer_idx++] = ch;
    }
}
