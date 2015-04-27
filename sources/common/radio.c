/*
 * radio.c
 *
 *  Created on: 23.4.2015
 *      Author: Ville
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>

#include "nrf.h"
#include "hal.h"
#include "radio.h"
#include "timer.h"
#include "myassert.h"
#include "interrupt.h"

/* Maximum size of radio payload (radio restricts this to 255) */
#define RADIO_MAX_PAYLOAD       16u
#define RADIO_PREAMBLE_LEN      1u
#define RADIO_CRC_LEN           2u
#define RADIO_SYMBOLRATE        1000000u

/* Synchron word */
static uint8_t                  m_radio_addr[3] = {0xDE, 0xAD, 0xBE};
#define RADIO_ADDR_SIZE         sizeof(m_radio_addr) / sizeof(m_radio_addr[0])

/* Radio DMA buffer contains length field + data */
typedef struct
{
    uint8_t len;
    uint8_t pld[255];
}radio_packet_t;

/* Data structure that contains the radio packet */
static radio_packet_t           m_radio_packet;

/* Internal frequency and power*/
static volatile uint32_t        m_frequency;
static volatile uint32_t        m_power;

/* Radio state handling */
typedef enum
{
    RADIO_STATE_IDLE,
    RADIO_STATE_TX,
    RADIO_STATE_RX
}radio_state_e;

#define RADIO_RAMP_UP_TIME      150u
#define RADIO_TTOA              ((((RADIO_PREAMBLE_LEN +         \
                                    RADIO_ADDR_SIZE +            \
                                    RADIO_CRC_LEN +              \
                                    RADIO_MAX_PAYLOAD)) *        \
                                    8000000) / RADIO_SYMBOLRATE)
static uint32_t                 m_tx_end_time;
static volatile radio_state_e   m_radio_state;
static volatile radio_rx_cb_f   m_rx_callback;
static volatile uint32_t        m_rx_max_size;
static void                     wait_tx_end(void);
static void                     disable_radio(void);

void Radio_init(void)
{
    /* Power cycle */
    NRF_RADIO->POWER = RADIO_POWER_POWER_Disabled;
    /* Some delay if clocks are not synchronized */
    Timer_delayMicros(500);
    NRF_RADIO->POWER = RADIO_POWER_POWER_Enabled;
    /* Enable automatic power off when all is done */
    NRF_RADIO->SHORTS = RADIO_SHORTS_READY_START_Msk |
                        RADIO_SHORTS_END_DISABLE_Msk;
    /* Select 1MBit symbolrate mode */
    NRF_RADIO->MODE = RADIO_MODE_MODE_Nrf_1Mbit;
    /* Select payload length field size */
    NRF_RADIO->PCNF0 = 8 << RADIO_PCNF0_LFLEN_Pos;
    /* Configure packet length and SYNC word */
    NRF_RADIO->PCNF1 = RADIO_MAX_PAYLOAD << RADIO_PCNF1_MAXLEN_Pos |
                       RADIO_ADDR_SIZE << RADIO_PCNF1_BALEN_Pos;
    /* Set SYNC word */
    NRF_RADIO->PREFIX0 = m_radio_addr[0];
    NRF_RADIO->BASE0 = m_radio_addr[1] | m_radio_addr[2];
    NRF_RADIO->BASE1 = 0;
    /* Disable BT specific crap */
    NRF_RADIO->TXADDRESS = 0;
    NRF_RADIO->RXADDRESSES = 1;
    /* 2 Byte CRC */
    NRF_RADIO->CRCCNF = RADIO_CRC_LEN;
    NRF_RADIO->CRCPOLY = 0x11021UL;
    NRF_RADIO->CRCINIT = 0xFFFFUL;
    /* Clear the rest */
    NRF_RADIO->FREQUENCY = 0;
    NRF_RADIO->TEST = 0;
    NRF_RADIO->TIFS = 0;
    NRF_RADIO->DACNF = 0;
    NRF_RADIO->BCC = 0;
    NVIC_DisableIRQ(RADIO_IRQn);
    NVIC_ClearPendingIRQ(RADIO_IRQn);
    NVIC_SetPriority(RADIO_IRQn, HAL_RADIO_INTERRUPT_PRIO);
    m_radio_state = RADIO_STATE_IDLE;
}

bool Radio_send(const void * data, uint32_t len)
{
    bool ret = false;
    assert(data != NULL);
    wait_tx_end();
    assert(m_radio_state == RADIO_STATE_IDLE);
    if((len > RADIO_MAX_PAYLOAD) || (len == 0))
    {
        goto tx_failed;
    }
    if(NRF_RADIO->STATE != RADIO_STATE_STATE_Disabled)
    {
        goto tx_failed;
    }
    /* Copy data to DMA structure */
    memset((void *)&m_radio_packet, 0, sizeof(m_radio_packet));
    memcpy((void *)m_radio_packet.pld, data, len);
    /* Radio sends only fixed sized packets */
    m_radio_packet.len = RADIO_MAX_PAYLOAD;
    /* Set packet address for DMA */
    NRF_RADIO->EVENTS_END = 0;
    NRF_RADIO->PACKETPTR = (uint32_t)&m_radio_packet;
    /* Enable shortcuts for disabling radio when TX is done */
    NRF_RADIO->SHORTS = RADIO_SHORTS_READY_START_Msk |
                        RADIO_SHORTS_END_DISABLE_Msk;
    NRF_RADIO->FREQUENCY = m_frequency;
    NRF_RADIO->TXPOWER = m_power;
    NRF_RADIO->TASKS_TXEN = 1;
    m_tx_end_time = Timer_getCount();
    m_tx_end_time += (RADIO_TTOA + RADIO_RAMP_UP_TIME);
    Timer_setTimeout(m_tx_end_time);
    m_radio_state = RADIO_STATE_TX;
    ret = true;
tx_failed:
    return ret;
}

bool Radio_receive(void * data, uint32_t * len, uint32_t timeout)
{
    bool ret = false;
    uint32_t now = Timer_getCount();
    assert(data != NULL);
    assert(len != NULL);
    wait_tx_end();
    assert(m_radio_state == RADIO_STATE_IDLE);
    if(NRF_RADIO->STATE != RADIO_STATE_STATE_Disabled)
    {
        goto rx_failed;
    }
    /* Set DMA buffer */
    NRF_RADIO->EVENTS_END = 0;
    NRF_RADIO->PACKETPTR = (uint32_t)&m_radio_packet;
    /* Enable automatic state transition after packet is received */
    NRF_RADIO->SHORTS = RADIO_SHORTS_READY_START_Msk |
                        RADIO_SHORTS_END_DISABLE_Msk;
    NRF_RADIO->FREQUENCY = m_frequency;
    NRF_RADIO->TASKS_RXEN = 1;
    timeout += (now + RADIO_RAMP_UP_TIME);
    Timer_setTimeout(timeout);
    while(!NRF_RADIO->EVENTS_END)
    {
        if(Timer_getTimeout())
        {
            goto rx_failed;
        }
    }
    if (NRF_RADIO->CRCSTATUS == RADIO_CRCSTATUS_CRCSTATUS_CRCError)
    {
        goto rx_failed;
    }
    if((m_radio_packet.len > *len) || (m_radio_packet.len == 0))
    {
        goto rx_failed;
    }
    memcpy(data, (void *)m_radio_packet.pld, m_radio_packet.len);
    *len = m_radio_packet.len;
    ret = true;
rx_failed:
    m_radio_state = RADIO_STATE_IDLE;
    disable_radio();
    Timer_clearTimeout();
    return ret;
}

bool Radio_receiveAsynch(radio_rx_cb_f cb, uint32_t max_size)
{
    bool ret = false;
    wait_tx_end();
    assert(m_radio_state == RADIO_STATE_IDLE);
    Interrupt_disableAll();
    if((cb != NULL) && (m_rx_callback == NULL))
    {
        m_rx_callback = cb;
        m_rx_max_size = max_size;
        NRF_RADIO->INTENSET = RADIO_INTENSET_END_Msk;
        NRF_RADIO->EVENTS_END = 0;
        NRF_RADIO->PACKETPTR = (uint32_t)&m_radio_packet;
        /* Enable automatic state transition after packet is received */
        NRF_RADIO->SHORTS = RADIO_SHORTS_READY_START_Msk |
                            RADIO_SHORTS_END_DISABLE_Msk;
        NRF_RADIO->FREQUENCY = m_frequency;
        NVIC_ClearPendingIRQ(RADIO_IRQn);
        NVIC_EnableIRQ(RADIO_IRQn);
        NRF_RADIO->TASKS_RXEN = 1;
        ret = true;
    }
    Interrupt_enableAll();
    return ret;
}

void Radio_stopReceiver(void)
{
    assert(m_rx_callback != NULL);
    Interrupt_disableAll();
    m_rx_callback = NULL;
    disable_radio();
    NVIC_DisableIRQ(RADIO_IRQn);
    NVIC_ClearPendingIRQ(RADIO_IRQn);
    NRF_RADIO->INTENCLR = 0xFFFFFFFF;
    m_radio_state = RADIO_STATE_IDLE;
    Interrupt_enableAll();
}

void Radio_setFrequency(uint32_t frequency)
{
    if(frequency == 0)
    {
        /* 0 is not a valid frequency */
        frequency = 1;
    }
    else if(frequency > 83)
    {
        /* Frequencies cannot go beyond base + 83 * 1MHz */
        frequency = 83;
    }
    m_frequency = frequency;
}

void Radio_setPower(radio_power_levels_e power)
{
    switch(power)
    {
        case RADIO_POWER_MINIMUM:
            power = RADIO_TXPOWER_TXPOWER_Neg30dBm;
            break;
        case RADIO_POWER_MEDIUM:
            power = RADIO_TXPOWER_TXPOWER_Neg12dBm;
            break;
        case RADIO_POWER_MAX:
            power = RADIO_TXPOWER_TXPOWER_Pos4dBm;
            break;
        default:
            power = RADIO_TXPOWER_TXPOWER_Neg30dBm;
            break;
    }
    m_power = power;
}

static void wait_tx_end(void)
{
    if(m_radio_state == RADIO_STATE_TX)
    {
        Timer_waitTimeout();
        m_radio_state = RADIO_STATE_IDLE;
    }
}

static void disable_radio(void)
{
    NRF_RADIO->TASKS_DISABLE = 1;
    while (NRF_RADIO->STATE != RADIO_STATE_STATE_Disabled)
    {
    }
}

void __attribute__((__interrupt__)) RADIO_IRQHandler(void)
{
    if (NRF_RADIO->EVENTS_END)
    {
        /** \todo Use timer CC[4] for this (polling creates a bit of bias) */
        uint32_t timestamp = Timer_getCount();
        /* Timestamp is when packet was sent: now - ramp_up - ttoa */
        timestamp -= RADIO_RAMP_UP_TIME - RADIO_TTOA;
        NRF_RADIO->EVENTS_END = 0;
        if (NRF_RADIO->CRCSTATUS == RADIO_CRCSTATUS_CRCSTATUS_CRCOk)
        {
            if((m_rx_callback != NULL) && (m_radio_packet.len <= m_rx_max_size))
            {
                m_rx_callback((void *)m_radio_packet.pld,
                              m_radio_packet.len,
                              timestamp);
            }
        }
        NRF_RADIO->TASKS_RXEN = 1;
    }
}
