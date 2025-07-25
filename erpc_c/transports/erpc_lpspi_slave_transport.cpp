/*
 * Copyright 2022-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_lpspi_slave_transport.hpp"

#include <cstdio>
#include <new>

extern "C" {
#include "board.h"
#include "fsl_gpio.h"
#include "fsl_lpspi.h"
}

using namespace std;
using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

#ifndef ERPC_BOARD_LPSPI_SLAVE_READY_USE_GPIO
#define ERPC_BOARD_LPSPI_SLAVE_READY_MARKER_LEN 2U
#define ERPC_BOARD_LPSPI_SLAVE_READY_MARKER1 0xABU
#define ERPC_BOARD_LPSPI_SLAVE_READY_MARKER2 0xCDU
#else
#ifndef ERPC_BOARD_LPSPI_INT_GPIO
#error "Please define the ERPC_BOARD_LPSPI_INT_GPIO used to notify when the LPSPI Slave is ready to transmit"
#endif
#endif

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

static lpspi_slave_handle_t s_handle;
static volatile bool s_isTransferCompleted = false;
static LPSpiSlaveTransport *s_lpspi_slave_instance = NULL;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

#ifdef ERPC_BOARD_LPSPI_SLAVE_READY_USE_GPIO
/* @brief Initialize the GPIO used to notify the LPSPI Master */
static inline void LPSpiSlaveTransport_NotifyTransferGpioInit(void)
{
    gpio_pin_config_t gpioConfig;

    gpioConfig.pinDirection = kGPIO_DigitalOutput;
    gpioConfig.outputLogic = 1U;

#ifdef ERPC_BOARD_LPSPI_INT_GPIO_LPC
    /* NXP LPC parts with the MCUXpressoSDK LPC GPIO driver */
    GPIO_PinInit(ERPC_BOARD_LPSPI_INT_GPIO, ERPC_BOARD_LPSPI_INT_PORT, ERPC_BOARD_LPSPI_INT_PIN, &gpioConfig);
#else
    /* NXP Kinetis/iMX parts with the MCUXpressoSDK GPIO driver */
    GPIO_PinInit(ERPC_BOARD_LPSPI_INT_GPIO, ERPC_BOARD_LPSPI_INT_PIN, &gpioConfig);
#endif
}

/* @brief Notify the LPSPI Master that the Slave is ready for a new transfer */
static inline void LPSpiSlaveTransport_NotifyTransferGpioReady(void)
{
#ifdef ERPC_BOARD_LPSPI_INT_GPIO_LPC
    /* NXP LPC parts with the MCUXpressoSDK LPC GPIO driver */
    GPIO_PortClear(ERPC_BOARD_LPSPI_INT_GPIO, ERPC_BOARD_LPSPI_INT_PORT, 1U << ERPC_BOARD_LPSPI_INT_PIN);
#else
    /* NXP Kinetis/iMX parts with the MCUXpressoSDK GPIO driver */
    GPIO_PortClear(ERPC_BOARD_LPSPI_INT_GPIO, 1U << ERPC_BOARD_LPSPI_INT_PIN);
#endif
}

/* @brief Notify the LPSPI Master that the Slave has finished the transfer */
static inline void LPSpiSlaveTransport_NotifyTransferGpioCompleted(void)
{
#ifdef ERPC_BOARD_LPSPI_INT_GPIO_LPC
    /* NXP LPC parts with the MCUXpressoSDK LPC GPIO driver */
    GPIO_PortSet(ERPC_BOARD_LPSPI_INT_GPIO, ERPC_BOARD_LPSPI_INT_PORT, 1U << ERPC_BOARD_LPSPI_INT_PIN);
#else
    /* NXP Kinetis/iMX parts with the MCUXpressoSDK GPIO driver */
    GPIO_PortSet(ERPC_BOARD_LPSPI_INT_GPIO, 1U << ERPC_BOARD_LPSPI_INT_PIN);
#endif
}
#endif

void LPSpiSlaveTransport::transfer_cb(void)
{
#if ERPC_THREADS
    m_txrxSemaphore.putFromISR();
#else
    s_isTransferCompleted = true;
#endif
}

static void LPSPI_SlaveUserCallback(LPSPI_Type *base, lpspi_slave_handle_t *handle, status_t status, void *userData)
{
    (void)base;
    (void)handle;
    (void)status;
    (void)userData;

    LPSpiSlaveTransport *transport = s_lpspi_slave_instance;

    transport->transfer_cb();
}

LPSpiSlaveTransport::LPSpiSlaveTransport(LPSPI_Type *lpspiBaseAddr, uint32_t baudRate, uint32_t srcClock_Hz) :
m_lpspiBaseAddr(lpspiBaseAddr), m_baudRate(baudRate), m_srcClock_Hz(srcClock_Hz), m_isInited(false)
#if ERPC_THREADS
,
m_txrxSemaphore()
#endif
{
    s_lpspi_slave_instance = this;
}

LPSpiSlaveTransport::~LPSpiSlaveTransport(void)
{
    if (m_isInited)
    {
#ifdef ERPC_BOARD_LPSPI_SLAVE_READY_USE_GPIO
        LPSpiSlaveTransport_NotifyTransferGpioCompleted();
#endif
        LPSPI_Deinit(m_lpspiBaseAddr);
        m_isInited = false;
    }
}

erpc_status_t LPSpiSlaveTransport::init(void)
{
    lpspi_slave_config_t lpspiConfig;

    LPSPI_SlaveGetDefaultConfig(&lpspiConfig);
#if (defined(MIMXRT798S_cm33_core0_SERIES) || defined(MIMXRT798S_cm33_core1_SERIES) || \
    defined(MIMXRT758S_cm33_core0_SERIES) || defined(MIMXRT758S_cm33_core1_SERIES) || \
    defined(MIMXRT735S_cm33_core0_SERIES) || defined(MIMXRT735S_cm33_core1_SERIES)) && \
    defined(FSL_FEATURE_SILICON_VERSION_A)
    lpspiConfig.pinCfg = kLPSPI_SdoInSdiOut;
#endif

    (void)LPSPI_SlaveInit(m_lpspiBaseAddr, &lpspiConfig);
    (void)LPSPI_SlaveTransferCreateHandle(m_lpspiBaseAddr, &s_handle, LPSPI_SlaveUserCallback, NULL);

#ifdef ERPC_BOARD_LPSPI_SLAVE_READY_USE_GPIO
    LPSpiSlaveTransport_NotifyTransferGpioInit();
#endif

    m_isInited = true;
    return kErpcStatus_Success;
}

erpc_status_t LPSpiSlaveTransport::underlyingReceive(uint8_t *data, uint32_t size)
{
    status_t status;
    lpspi_transfer_t slaveXfer = { 0 };

    slaveXfer.txData = NULL;
    slaveXfer.rxData = data;
    slaveXfer.dataSize = size;
    s_isTransferCompleted = false;

    status = LPSPI_SlaveTransferNonBlocking(m_lpspiBaseAddr, &s_handle, &slaveXfer);

    if (kStatus_Success == status)
    {
#ifdef ERPC_BOARD_LPSPI_SLAVE_READY_USE_GPIO
        LPSpiSlaveTransport_NotifyTransferGpioReady();
#endif

/* wait until the receiving is finished */
#if ERPC_THREADS
        m_txrxSemaphore.get();
#else
        while (!s_isTransferCompleted)
        {
        }
#endif

#ifdef ERPC_BOARD_LPSPI_SLAVE_READY_USE_GPIO
        LPSpiSlaveTransport_NotifyTransferGpioCompleted();
#endif
    }

    return (status != kStatus_Success) ? kErpcStatus_ReceiveFailed : kErpcStatus_Success;
}

erpc_status_t LPSpiSlaveTransport::underlyingSend(const uint8_t *data, uint32_t size)
{
    status_t status;
    lpspi_transfer_t slaveXfer = { 0 };
    s_isTransferCompleted = false;

#ifdef ERPC_BOARD_LPSPI_SLAVE_READY_USE_GPIO
    slaveXfer.txData = (uint8_t *)data;
    slaveXfer.rxData = NULL;
    slaveXfer.dataSize = size;
    {
#else
    uint8_t *lpspiData = new (nothrow) uint8_t[size + ERPC_BOARD_LPSPI_SLAVE_READY_MARKER_LEN];
    if (lpspiData != NULL)
    {
        lpspiData[0] = ERPC_BOARD_LPSPI_SLAVE_READY_MARKER1;
        lpspiData[1] = ERPC_BOARD_LPSPI_SLAVE_READY_MARKER2;
        (void)memcpy(&lpspiData[ERPC_BOARD_LPSPI_SLAVE_READY_MARKER_LEN], data, size);
        slaveXfer.txData = lpspiData;
        slaveXfer.rxData = NULL;
        slaveXfer.dataSize = size + ERPC_BOARD_LPSPI_SLAVE_READY_MARKER_LEN;
#endif

        status = LPSPI_SlaveTransferNonBlocking(m_lpspiBaseAddr, &s_handle, &slaveXfer);

        if (kStatus_Success == status)
        {
#ifdef ERPC_BOARD_LPSPI_SLAVE_READY_USE_GPIO
            LPSpiSlaveTransport_NotifyTransferGpioReady();
#endif

/* wait until the sending is finished */
#if ERPC_THREADS
            m_txrxSemaphore.get();
#else
            while (!s_isTransferCompleted)
            {
            }
#endif

#ifdef ERPC_BOARD_LPSPI_SLAVE_READY_USE_GPIO
            LPSpiSlaveTransport_NotifyTransferGpioCompleted();
#endif
        }
#ifdef ERPC_BOARD_LPSPI_SLAVE_READY_USE_GPIO
    }
#else
        delete[] lpspiData;
    }
    else
    {
        status = kErpcStatus_SendFailed;
    }
#endif

    return (status != kStatus_Success) ? kErpcStatus_SendFailed : kErpcStatus_Success;
}
