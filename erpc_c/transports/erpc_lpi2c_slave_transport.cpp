/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_lpi2c_slave_transport.hpp"

#include <cstdio>
#include <new>

extern "C" {
#include "board.h"
#include "fsl_gpio.h"
#include "fsl_lpi2c.h"
}

using namespace std;
using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

#ifndef ERPC_BOARD_LPI2C_INT_GPIO
#error "Please define the ERPC_BOARD_LPI2C_INT_GPIO used to notify when the LPI2C Slave is ready to transmit"
#endif

#define LPI2C_SLAVE_TRANSPORT_ADDR_7BIT (0x7EU)

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

static lpi2c_slave_handle_t s_handle;
static volatile bool s_isTransferCompleted = false;
static LPI2cSlaveTransport *s_lpi2c_slave_instance = NULL;

typedef struct lpi2c_clb_user_data
{
    uint8_t *tx_buffer;
    uint32_t tx_size;
    uint8_t *rx_buffer;
    uint32_t rx_size;
} LPI2C_CLB_USER_DATA, *LPI2C_CLB_USER_DATA_PTR;
static volatile LPI2C_CLB_USER_DATA s_callback_user_data = { NULL, 0, NULL, 0 };

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

/* @brief Initialize the GPIO used to notify the LPI2C Master */
static inline void LPI2cSlaveTransport_NotifyTransferGpioInit()
{
    gpio_pin_config_t gpioConfig;

    gpioConfig.pinDirection = kGPIO_DigitalOutput;
    gpioConfig.outputLogic = 1U;

#ifdef ERPC_BOARD_LPI2C_INT_PORT
    /* NXP LPC parts with the MCUXpressoSDK LPC GPIO driver */
    GPIO_PinInit(ERPC_BOARD_LPI2C_INT_GPIO, ERPC_BOARD_LPI2C_INT_PORT, ERPC_BOARD_LPI2C_INT_PIN, &gpioConfig);
#else
    /* NXP Kinetis/iMX parts with the MCUXpressoSDK GPIO driver */
    GPIO_PinInit(ERPC_BOARD_LPI2C_INT_GPIO, ERPC_BOARD_LPI2C_INT_PIN, &gpioConfig);
#endif
}

/* @brief Notify the LPI2C Master that the Slave is ready for a new transfer */
static inline void LPI2cSlaveTransport_NotifyTransferGpioReady()
{
#ifdef ERPC_BOARD_LPI2C_INT_PORT
    /* NXP LPC parts with the MCUXpressoSDK LPC GPIO driver */
    GPIO_PortClear(ERPC_BOARD_LPI2C_INT_GPIO, ERPC_BOARD_LPI2C_INT_PORT, 1U << ERPC_BOARD_LPI2C_INT_PIN);
#else
    /* NXP Kinetis/iMX parts with the MCUXpressoSDK GPIO driver */
    GPIO_PortClear(ERPC_BOARD_LPI2C_INT_GPIO, 1U << ERPC_BOARD_LPI2C_INT_PIN);
#endif
}

/* @brief Notify the LPI2C Master that the Slave has finished the transfer */
static inline void LPI2cSlaveTransport_NotifyTransferGpioCompleted()
{
#ifdef ERPC_BOARD_LPI2C_INT_PORT
    /* NXP LPC parts with the MCUXpressoSDK LPC GPIO driver */
    GPIO_PortSet(ERPC_BOARD_LPI2C_INT_GPIO, ERPC_BOARD_LPI2C_INT_PORT, 1U << ERPC_BOARD_LPI2C_INT_PIN);
#else
    /* NXP Kinetis/iMX parts with the MCUXpressoSDK GPIO driver */
    GPIO_PortSet(ERPC_BOARD_LPI2C_INT_GPIO, 1U << ERPC_BOARD_LPI2C_INT_PIN);
#endif
}

void LPI2cSlaveTransport::transfer_cb(void)
{
#if ERPC_THREADS
    m_txrxSemaphore.putFromISR();
#else
    s_isTransferCompleted = true;
#endif
}

static void LPI2C_SlaveUserCallback(LPI2C_Type *base, lpi2c_slave_transfer_t *transfer, void *userData)
{
    LPI2cSlaveTransport *transport = s_lpi2c_slave_instance;
    switch (transfer->event)
    {
        break;
        /*  Transmit request */
        case kLPI2C_SlaveTransmitEvent:
            /*  Update information for transmit process */
            transfer->data = ((LPI2C_CLB_USER_DATA *)userData)->tx_buffer;
            transfer->dataSize = ((LPI2C_CLB_USER_DATA *)userData)->tx_size;
            break;

        /* Setup the slave receive buffer */
        case kLPI2C_SlaveReceiveEvent:
            /*  Update information for received process */
            transfer->data = ((LPI2C_CLB_USER_DATA *)userData)->rx_buffer;
            transfer->dataSize = ((LPI2C_CLB_USER_DATA *)userData)->rx_size;
            break;

        /* The master has sent a stop transition on the bus */
        case kLPI2C_SlaveCompletionEvent:
            /* Filter out dummy transaction completions (additional dummy recv./transmit) */
            if (!(transfer->transferredCount == 0 && transfer->dataSize == 0) && transfer->data != NULL)
            {
                transport->transfer_cb();
                transfer->data = NULL;
                transfer->dataSize = 0;
            }
            break;

        default:
            s_isTransferCompleted = false;
            break;
    }
}

LPI2cSlaveTransport::LPI2cSlaveTransport(LPI2C_Type *lpi2cBaseAddr, uint32_t baudRate, uint32_t srcClock_Hz)
: m_lpi2cBaseAddr(lpi2cBaseAddr)
, m_baudRate(baudRate)
, m_srcClock_Hz(srcClock_Hz)
, m_isInited(false)
#if ERPC_THREADS
, m_txrxSemaphore()
#endif
{
    s_lpi2c_slave_instance = this;
}

LPI2cSlaveTransport::~LPI2cSlaveTransport(void)
{
    if (m_isInited)
    {
        LPI2cSlaveTransport_NotifyTransferGpioCompleted();
        LPI2C_SlaveDeinit(m_lpi2cBaseAddr);
        m_isInited = false;
    }
}

erpc_status_t LPI2cSlaveTransport::init(void)
{
    lpi2c_slave_config_t lpi2cConfig;

    LPI2C_SlaveGetDefaultConfig(&lpi2cConfig);
    lpi2cConfig.address0 = (LPI2C_SLAVE_TRANSPORT_ADDR_7BIT);

    LPI2C_SlaveInit(m_lpi2cBaseAddr, &lpi2cConfig, m_srcClock_Hz);
    LPI2C_SlaveTransferCreateHandle(m_lpi2cBaseAddr, &s_handle, LPI2C_SlaveUserCallback, (void *)&s_callback_user_data);

    LPI2cSlaveTransport_NotifyTransferGpioInit();

    m_isInited = true;
    return kErpcStatus_Success;
}

erpc_status_t LPI2cSlaveTransport::underlyingReceive(uint8_t *data, uint32_t size)
{
    status_t status;
    s_isTransferCompleted = false;
    uint8_t dummy = 0x0;

    s_callback_user_data.rx_buffer = data;
    s_callback_user_data.rx_size = size;
    s_callback_user_data.tx_buffer = &dummy;
    s_callback_user_data.tx_size = 1;

    status = LPI2C_SlaveTransferNonBlocking(m_lpi2cBaseAddr, &s_handle, kLPI2C_SlaveCompletionEvent);

    if (kStatus_Success == status)
    {
        LPI2cSlaveTransport_NotifyTransferGpioReady();

/* wait until the receiving is finished */
#if ERPC_THREADS
        m_txrxSemaphore.get();
#else
        while (!s_isTransferCompleted)
        {
        }
#endif

        LPI2cSlaveTransport_NotifyTransferGpioCompleted();
    }

    return (status != kStatus_Success) ? kErpcStatus_ReceiveFailed : kErpcStatus_Success;
}

erpc_status_t LPI2cSlaveTransport::underlyingSend(const uint8_t *data, uint32_t size)
{
    status_t status;
    s_isTransferCompleted = false;

    s_callback_user_data.rx_buffer = NULL;
    s_callback_user_data.rx_size = 0;
    s_callback_user_data.tx_buffer = (uint8_t *)data;
    s_callback_user_data.tx_size = size;

    {
        status = LPI2C_SlaveTransferNonBlocking(m_lpi2cBaseAddr, &s_handle, kLPI2C_SlaveCompletionEvent);

        if (kStatus_Success == status)
        {
            LPI2cSlaveTransport_NotifyTransferGpioReady();

/* wait until the sending is finished */
#if ERPC_THREADS
            m_txrxSemaphore.get();
#else
            while (!s_isTransferCompleted)
            {
            }
#endif

            LPI2cSlaveTransport_NotifyTransferGpioCompleted();
        }
    }

    return (status != kStatus_Success) ? kErpcStatus_SendFailed : kErpcStatus_Success;
}
