/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2023 NXP
 * Copyright 2021 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_spi_slave_transport.hpp"

#include <cstdio>
#include <new>

extern "C" {
#include "board.h"
#include "fsl_gpio.h"
#include "fsl_spi.h"
}

using namespace std;
using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

#ifndef ERPC_BOARD_SPI_SLAVE_READY_USE_GPIO
#define ERPC_BOARD_SPI_SLAVE_READY_MARKER_LEN 2U
#define ERPC_BOARD_SPI_SLAVE_READY_MARKER1 0xABU
#define ERPC_BOARD_SPI_SLAVE_READY_MARKER2 0xCDU
#else
#ifndef ERPC_BOARD_SPI_INT_GPIO
#error "Please define the ERPC_BOARD_SPI_INT_GPIO used to notify when the SPI Slave is ready to transmit"
#endif
#endif

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

static spi_slave_handle_t s_handle;
static volatile bool s_isTransferCompleted = false;
static SpiSlaveTransport *s_spi_slave_instance = NULL;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

#ifdef ERPC_BOARD_SPI_SLAVE_READY_USE_GPIO
/* @brief Initialize the GPIO used to notify the SPI Master */
static inline void SpiSlaveTransport_NotifyTransferGpioInit(void)
{
    gpio_pin_config_t gpioConfig;

    gpioConfig.pinDirection = kGPIO_DigitalOutput;
    gpioConfig.outputLogic = 1U;

#ifdef ERPC_BOARD_SPI_INT_GPIO_LPC
    /* NXP LPC parts with the MCUXpressoSDK LPC GPIO driver */
    GPIO_PinInit(ERPC_BOARD_SPI_INT_GPIO, ERPC_BOARD_SPI_INT_PORT, ERPC_BOARD_SPI_INT_PIN, &gpioConfig);
#else
    /* NXP Kinetis/iMX parts with the MCUXpressoSDK GPIO driver */
    GPIO_PinInit(ERPC_BOARD_SPI_INT_GPIO, ERPC_BOARD_SPI_INT_PIN, &gpioConfig);
#endif
}

/* @brief Notify the SPI Master that the Slave is ready for a new transfer */
static inline void SpiSlaveTransport_NotifyTransferGpioReady(void)
{
#ifdef ERPC_BOARD_SPI_INT_GPIO_LPC
    /* NXP LPC parts with the MCUXpressoSDK LPC GPIO driver */
    GPIO_PortClear(ERPC_BOARD_SPI_INT_GPIO, ERPC_BOARD_SPI_INT_PORT, 1U << ERPC_BOARD_SPI_INT_PIN);
#else
    /* NXP Kinetis/iMX parts with the MCUXpressoSDK GPIO driver */
    GPIO_PortClear(ERPC_BOARD_SPI_INT_GPIO, 1U << ERPC_BOARD_SPI_INT_PIN);
#endif
}

/* @brief Notify the SPI Master that the Slave has finished the transfer */
static inline void SpiSlaveTransport_NotifyTransferGpioCompleted(void)
{
#ifdef ERPC_BOARD_SPI_INT_GPIO_LPC
    /* NXP LPC parts with the MCUXpressoSDK LPC GPIO driver */
    GPIO_PortSet(ERPC_BOARD_SPI_INT_GPIO, ERPC_BOARD_SPI_INT_PORT, 1U << ERPC_BOARD_SPI_INT_PIN);
#else
    /* NXP Kinetis/iMX parts with the MCUXpressoSDK GPIO driver */
    GPIO_PortSet(ERPC_BOARD_SPI_INT_GPIO, 1U << ERPC_BOARD_SPI_INT_PIN);
#endif
}
#endif

void SpiSlaveTransport::transfer_cb(void)
{
#if ERPC_THREADS
    m_txrxSemaphore.putFromISR();
#else
    s_isTransferCompleted = true;
#endif
}

static void SPI_SlaveUserCallback(SPI_Type *base, spi_slave_handle_t *handle, status_t status, void *userData)
{
    (void)base;
    (void)handle;
    (void)status;
    (void)userData;

    SpiSlaveTransport *transport = s_spi_slave_instance;

    transport->transfer_cb();
}

SpiSlaveTransport::SpiSlaveTransport(SPI_Type *spiBaseAddr, uint32_t baudRate, uint32_t srcClock_Hz) :
m_spiBaseAddr(spiBaseAddr), m_baudRate(baudRate), m_srcClock_Hz(srcClock_Hz), m_isInited(false)
#if ERPC_THREADS
,
m_txrxSemaphore()
#endif
{
    s_spi_slave_instance = this;
}

SpiSlaveTransport::~SpiSlaveTransport(void)
{
    if (m_isInited)
    {
#ifdef ERPC_BOARD_SPI_SLAVE_READY_USE_GPIO
        SpiSlaveTransport_NotifyTransferGpioCompleted();
#endif
        SPI_Deinit(m_spiBaseAddr);
        m_isInited = false;
    }
}

erpc_status_t SpiSlaveTransport::init(void)
{
    spi_slave_config_t spiConfig;

    SPI_SlaveGetDefaultConfig(&spiConfig);

    (void)SPI_SlaveInit(m_spiBaseAddr, &spiConfig);
    (void)SPI_SlaveTransferCreateHandle(m_spiBaseAddr, &s_handle, SPI_SlaveUserCallback, NULL);

#ifdef ERPC_BOARD_SPI_SLAVE_READY_USE_GPIO
    SpiSlaveTransport_NotifyTransferGpioInit();
#endif

    m_isInited = true;
    return kErpcStatus_Success;
}

erpc_status_t SpiSlaveTransport::underlyingReceive(uint8_t *data, uint32_t size)
{
    status_t status;
    spi_transfer_t slaveXfer = { 0 };

    slaveXfer.txData = NULL;
    slaveXfer.rxData = data;
    slaveXfer.dataSize = size;
    s_isTransferCompleted = false;

    status = SPI_SlaveTransferNonBlocking(m_spiBaseAddr, &s_handle, &slaveXfer);

    if (kStatus_Success == status)
    {
#ifdef ERPC_BOARD_SPI_SLAVE_READY_USE_GPIO
        SpiSlaveTransport_NotifyTransferGpioReady();
#endif

/* wait until the receiving is finished */
#if ERPC_THREADS
        m_txrxSemaphore.get();
#else
        while (!s_isTransferCompleted)
        {
        }
#endif

#ifdef ERPC_BOARD_SPI_SLAVE_READY_USE_GPIO
        SpiSlaveTransport_NotifyTransferGpioCompleted();
#endif
    }

    return (status != kStatus_Success) ? kErpcStatus_ReceiveFailed : kErpcStatus_Success;
}

erpc_status_t SpiSlaveTransport::underlyingSend(const uint8_t *data, uint32_t size)
{
    status_t status;
    spi_transfer_t slaveXfer = { 0 };
    s_isTransferCompleted = false;
    uint32_t header_size = reserveHeaderSize();

    /* send the header first */
#ifdef ERPC_BOARD_SPI_SLAVE_READY_USE_GPIO
    slaveXfer.txData = (uint8_t *)data;
    slaveXfer.rxData = NULL;
    slaveXfer.dataSize = header_size;
    {
#else
    uint8_t *spiData = new (nothrow) uint8_t[header_size + ERPC_BOARD_SPI_SLAVE_READY_MARKER_LEN];
    if (spiData != NULL)
    {
        spiData[0] = ERPC_BOARD_SPI_SLAVE_READY_MARKER1;
        spiData[1] = ERPC_BOARD_SPI_SLAVE_READY_MARKER2;
        (void)memcpy(&spiData[ERPC_BOARD_SPI_SLAVE_READY_MARKER_LEN], data, header_size);
        slaveXfer.txData = spiData;
        slaveXfer.rxData = NULL;
        slaveXfer.dataSize = header_size + ERPC_BOARD_SPI_SLAVE_READY_MARKER_LEN;
#endif

        status = SPI_SlaveTransferNonBlocking(m_spiBaseAddr, &s_handle, &slaveXfer);

        if (kStatus_Success == status)
        {
#ifdef ERPC_BOARD_SPI_SLAVE_READY_USE_GPIO
            SpiSlaveTransport_NotifyTransferGpioReady();
#endif

/* wait until the sending is finished */
#if ERPC_THREADS
            m_txrxSemaphore.get();
#else
            while (!s_isTransferCompleted)
            {
            }
#endif

#ifdef ERPC_BOARD_SPI_SLAVE_READY_USE_GPIO
            SpiSlaveTransport_NotifyTransferGpioCompleted();
#endif
        }
#ifdef ERPC_BOARD_SPI_SLAVE_READY_USE_GPIO
    }
#else
        delete[] spiData;
    }
    else
    {
        status = kErpcStatus_SendFailed;
    }
#endif

    /* send the payload now */
    s_isTransferCompleted = false;

#ifdef ERPC_BOARD_SPI_SLAVE_READY_USE_GPIO
    slaveXfer.txData = (uint8_t *)data + header_size;
    slaveXfer.rxData = NULL;
    slaveXfer.dataSize = size - header_size;
    {
#else
    uint8_t *spiData = new (nothrow) uint8_t[size - header_size + ERPC_BOARD_SPI_SLAVE_READY_MARKER_LEN];
    if (spiData != NULL)
    {
        spiData[0] = ERPC_BOARD_SPI_SLAVE_READY_MARKER1;
        spiData[1] = ERPC_BOARD_SPI_SLAVE_READY_MARKER2;
        (void)memcpy(&spiData[ERPC_BOARD_SPI_SLAVE_READY_MARKER_LEN], data + header_size, size - header_size);
        slaveXfer.txData = spiData;
        slaveXfer.rxData = NULL;
        slaveXfer.dataSize = size - header_size + ERPC_BOARD_SPI_SLAVE_READY_MARKER_LEN;
#endif

        status = SPI_SlaveTransferNonBlocking(m_spiBaseAddr, &s_handle, &slaveXfer);

        if (kStatus_Success == status)
        {
#ifdef ERPC_BOARD_SPI_SLAVE_READY_USE_GPIO
            SpiSlaveTransport_NotifyTransferGpioReady();
#endif

/* wait until the sending is finished */
#if ERPC_THREADS
            m_txrxSemaphore.get();
#else
            while (!s_isTransferCompleted)
            {
            }
#endif

#ifdef ERPC_BOARD_SPI_SLAVE_READY_USE_GPIO
            SpiSlaveTransport_NotifyTransferGpioCompleted();
#endif
        }
#ifdef ERPC_BOARD_SPI_SLAVE_READY_USE_GPIO
    }
#else
        delete[] spiData;
    }
    else
    {
        status = kErpcStatus_SendFailed;
    }
#endif

    return (status != kStatus_Success) ? kErpcStatus_SendFailed : kErpcStatus_Success;
}
