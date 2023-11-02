/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_i2c_slave_transport.hpp"

#include <cstdio>
#include <new>

extern "C" {
#include "board.h"
#include "fsl_gpio.h"
#include "fsl_i2c.h"
}

using namespace std;
using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

#ifndef ERPC_BOARD_I2C_INT_GPIO
#error "Please define the ERPC_BOARD_I2C_INT_GPIO used to notify when the I2C Slave is ready to transmit"
#endif

#define I2C_SLAVE_TRANSPORT_ADDR_7BIT (0x7EU)

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

static i2c_slave_handle_t s_handle;
static volatile bool s_isTransferCompleted = false;
static I2cSlaveTransport *s_i2c_slave_instance = NULL;

typedef struct i2c_clb_user_data
{
    uint8_t *tx_buffer;
    uint32_t tx_size;
    uint8_t *rx_buffer;
    uint32_t rx_size;
} I2C_CLB_USER_DATA, *I2C_CLB_USER_DATA_PTR;
static volatile I2C_CLB_USER_DATA s_callback_user_data = { NULL, 0, NULL, 0 };

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

/* @brief Initialize the GPIO used to notify the I2C Master */
static inline void I2cSlaveTransport_NotifyTransferGpioInit()
{
    gpio_pin_config_t gpioConfig;

    gpioConfig.pinDirection = kGPIO_DigitalOutput;
    gpioConfig.outputLogic = 1U;

#ifdef ERPC_BOARD_I2C_INT_PORT
    /* NXP LPC parts with the MCUXpressoSDK LPC GPIO driver */
    GPIO_PinInit(ERPC_BOARD_I2C_INT_GPIO, ERPC_BOARD_I2C_INT_PORT, ERPC_BOARD_I2C_INT_PIN, &gpioConfig);
#else
    /* NXP Kinetis/iMX parts with the MCUXpressoSDK GPIO driver */
    GPIO_PinInit(ERPC_BOARD_I2C_INT_GPIO, ERPC_BOARD_I2C_INT_PIN, &gpioConfig);
#endif
}

/* @brief Notify the I2C Master that the Slave is ready for a new transfer */
static inline void I2cSlaveTransport_NotifyTransferGpioReady()
{
#ifdef ERPC_BOARD_I2C_INT_PORT
    /* NXP LPC parts with the MCUXpressoSDK LPC GPIO driver */
    GPIO_PortClear(ERPC_BOARD_I2C_INT_GPIO, ERPC_BOARD_I2C_INT_PORT, 1U << ERPC_BOARD_I2C_INT_PIN);
#else
    /* NXP Kinetis/iMX parts with the MCUXpressoSDK GPIO driver */
    GPIO_PortClear(ERPC_BOARD_I2C_INT_GPIO, 1U << ERPC_BOARD_I2C_INT_PIN);
#endif
}

/* @brief Notify the I2C Master that the Slave has finished the transfer */
static inline void I2cSlaveTransport_NotifyTransferGpioCompleted()
{
#ifdef ERPC_BOARD_I2C_INT_PORT
    /* NXP LPC parts with the MCUXpressoSDK LPC GPIO driver */
    GPIO_PortSet(ERPC_BOARD_I2C_INT_GPIO, ERPC_BOARD_I2C_INT_PORT, 1U << ERPC_BOARD_I2C_INT_PIN);
#else
    /* NXP Kinetis/iMX parts with the MCUXpressoSDK GPIO driver */
    GPIO_PortSet(ERPC_BOARD_I2C_INT_GPIO, 1U << ERPC_BOARD_I2C_INT_PIN);
#endif
}

void I2cSlaveTransport::transfer_cb(void)
{
#if ERPC_THREADS
    m_txrxSemaphore.putFromISR();
#else
    s_isTransferCompleted = true;
#endif
}

static void I2C_SlaveUserCallback(I2C_Type *base, volatile i2c_slave_transfer_t *transfer, void *userData)
{
    I2cSlaveTransport *transport = s_i2c_slave_instance;
    switch (transfer->event)
    {
        /*  Address match event */
        case kI2C_SlaveAddressMatchEvent:
            transfer->rxData = NULL;
            transfer->rxSize = 0;
            break;
        /*  Transmit request */
        case kI2C_SlaveTransmitEvent:
            /*  Update information for transmit process */
            transfer->txData = ((I2C_CLB_USER_DATA *)userData)->tx_buffer;
            transfer->txSize = ((I2C_CLB_USER_DATA *)userData)->tx_size;
            transfer->rxData = NULL;
            transfer->rxSize = 0;
            break;

        /* Setup the slave receive buffer */
        case kI2C_SlaveReceiveEvent:
            /*  Update information for received process */
            transfer->rxData = ((I2C_CLB_USER_DATA *)userData)->rx_buffer;
            transfer->rxSize = ((I2C_CLB_USER_DATA *)userData)->rx_size;
            transfer->txData = NULL;
            transfer->txSize = 0;
            break;

        /* The master has sent a stop transition on the bus */
        case kI2C_SlaveCompletionEvent:
            transport->transfer_cb();
            transfer->rxData = NULL;
            transfer->rxSize = 0;
            transfer->txData = NULL;
            transfer->txSize = 0;
            break;

        default:
            s_isTransferCompleted = false;
            break;
    }
}

I2cSlaveTransport::I2cSlaveTransport(I2C_Type *i2cBaseAddr, uint32_t baudRate, uint32_t srcClock_Hz) :
m_i2cBaseAddr(i2cBaseAddr), m_baudRate(baudRate), m_srcClock_Hz(srcClock_Hz), m_isInited(false)
#if ERPC_THREADS
,
m_txrxSemaphore()
#endif
{
    s_i2c_slave_instance = this;
}

I2cSlaveTransport::~I2cSlaveTransport(void)
{
    if (m_isInited)
    {
        I2cSlaveTransport_NotifyTransferGpioCompleted();
        I2C_SlaveDeinit(m_i2cBaseAddr);
        m_isInited = false;
    }
}

erpc_status_t I2cSlaveTransport::init(void)
{
    i2c_slave_config_t i2cConfig;

    I2C_SlaveGetDefaultConfig(&i2cConfig);
    i2cConfig.address0.address = (I2C_SLAVE_TRANSPORT_ADDR_7BIT);

    I2C_SlaveInit(m_i2cBaseAddr, &i2cConfig, m_srcClock_Hz);
    I2C_SlaveTransferCreateHandle(m_i2cBaseAddr, &s_handle, I2C_SlaveUserCallback, (void *)&s_callback_user_data);

    I2cSlaveTransport_NotifyTransferGpioInit();

    m_isInited = true;
    return kErpcStatus_Success;
}

erpc_status_t I2cSlaveTransport::underlyingReceive(uint8_t *data, uint32_t size)
{
    status_t status;
    s_isTransferCompleted = false;

    s_callback_user_data.rx_buffer = data;
    s_callback_user_data.rx_size = size;
    s_callback_user_data.tx_buffer = NULL;
    s_callback_user_data.tx_size = 0;

    status =
        I2C_SlaveTransferNonBlocking(m_i2cBaseAddr, &s_handle, kI2C_SlaveAddressMatchEvent | kI2C_SlaveCompletionEvent);

    if (kStatus_Success == status)
    {
        I2cSlaveTransport_NotifyTransferGpioReady();

/* wait until the receiving is finished */
#if ERPC_THREADS
        m_txrxSemaphore.get();
#else
        while (!s_isTransferCompleted)
        {
        }
#endif

        I2cSlaveTransport_NotifyTransferGpioCompleted();
    }

    return (status != kStatus_Success) ? kErpcStatus_ReceiveFailed : kErpcStatus_Success;
}

erpc_status_t I2cSlaveTransport::underlyingSend(const uint8_t *data, uint32_t size)
{
    status_t status;
    s_isTransferCompleted = false;

    s_callback_user_data.rx_buffer = NULL;
    s_callback_user_data.rx_size = 0;
    s_callback_user_data.tx_buffer = (uint8_t *)data;
    s_callback_user_data.tx_size = size;

    {
        status = I2C_SlaveTransferNonBlocking(m_i2cBaseAddr, &s_handle,
                                              kI2C_SlaveAddressMatchEvent | kI2C_SlaveCompletionEvent);

        if (kStatus_Success == status)
        {
            I2cSlaveTransport_NotifyTransferGpioReady();

/* wait until the sending is finished */
#if ERPC_THREADS
            m_txrxSemaphore.get();
#else
            while (!s_isTransferCompleted)
            {
            }
#endif

            I2cSlaveTransport_NotifyTransferGpioCompleted();
        }
    }

    return (status != kStatus_Success) ? kErpcStatus_SendFailed : kErpcStatus_Success;
}
