/*
 * Copyright 2020-2021 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cstdio>
#include "erpc_usb_cdc_transport.hpp"

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

static volatile bool s_isTransferReceiveCompleted = false;
static volatile bool s_isTransferSendCompleted = false;
static UsbCdcTransport *s_usbcdc_instance = NULL;

SDK_ALIGN(static uint8_t s_serialWriteHandleBuffer[SERIAL_MANAGER_WRITE_HANDLE_SIZE], 4);
static serial_write_handle_t s_serialWriteHandle = &s_serialWriteHandleBuffer[0]; /*!< serial manager write handle */

SDK_ALIGN(static uint8_t s_serialReadHandleBuffer[SERIAL_MANAGER_READ_HANDLE_SIZE], 4);
static serial_read_handle_t s_serialReadHandle = &s_serialReadHandleBuffer[0]; /*!< serial manager read handle */
////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

static void ERPC_SerialManagerTxCallback(void *callbackParam, serial_manager_callback_message_t *message,
                                         serial_manager_status_t status)
{
    UsbCdcTransport *transport = s_usbcdc_instance;
    if ((NULL != callbackParam) && (NULL != message))
    {
        if (kStatus_SerialManager_Success == status)
        {
            transport->tx_cb();
        }
        else
        {
            /* Handle other status if needed */
        }
    }
}

static void ERPC_SerialManagerRxCallback(void *callbackParam, serial_manager_callback_message_t *message,
                                         serial_manager_status_t status)
{
    UsbCdcTransport *transport = s_usbcdc_instance;
    if ((NULL != callbackParam) && (NULL != message))
    {
        if (kStatus_SerialManager_Success == status)
        {
            transport->rx_cb();
        }
        else
        {
            /* Handle other status if needed */
        }
    }
}

void UsbCdcTransport::tx_cb(void)
{
#if !ERPC_THREADS_IS(NONE)
    m_txSemaphore.putFromISR();
#else
    s_isTransferSendCompleted = true;
#endif
}

void UsbCdcTransport::rx_cb(void)
{
#if !ERPC_THREADS_IS(NONE)
    m_rxSemaphore.putFromISR();
#else
    s_isTransferReceiveCompleted = true;
#endif
}

UsbCdcTransport::UsbCdcTransport(serial_handle_t serialHandle, serial_manager_config_t *serialConfig,
                                 serial_port_usb_cdc_config_t *usbCdcConfig, uint8_t *usbRingBuffer,
                                 uint32_t usbRingBufferLength)
: m_serialHandle(serialHandle)
, m_serialConfig(serialConfig)
, m_usbCdcConfig(usbCdcConfig)
, m_usbRingBuffer(usbRingBuffer)
, m_usbRingBufferLength(usbRingBufferLength)
#if !ERPC_THREADS_IS(NONE)
, m_rxSemaphore()
, m_txSemaphore()
#endif
{
    s_usbcdc_instance = this;
}

UsbCdcTransport::~UsbCdcTransport(void)
{
    /* Cleanup */
    SerialManager_CloseWriteHandle(s_serialWriteHandle);
    SerialManager_CloseReadHandle(s_serialReadHandle);
    SerialManager_Deinit(m_serialHandle);
}

erpc_status_t UsbCdcTransport::init(void)
{
    erpc_status_t status = kErpcStatus_InitFailed;

    /* Init Serial Manager for USB CDC */
    m_serialConfig->type = kSerialPort_UsbCdc;
    m_serialConfig->ringBuffer = m_usbRingBuffer;
    m_serialConfig->ringBufferSize = m_usbRingBufferLength;
    m_serialConfig->portConfig = m_usbCdcConfig;

    if (kStatus_SerialManager_Success == SerialManager_Init(m_serialHandle, m_serialConfig))
    {
        if (kStatus_SerialManager_Success == SerialManager_OpenWriteHandle(m_serialHandle, s_serialWriteHandle))
        {
            if (kStatus_SerialManager_Success == SerialManager_OpenReadHandle(m_serialHandle, s_serialReadHandle))
            {
                if (kStatus_SerialManager_Success == SerialManager_InstallTxCallback(s_serialWriteHandle,
                                                                                     ERPC_SerialManagerTxCallback,
                                                                                     s_serialWriteHandle))
                {
                    if (kStatus_SerialManager_Success == SerialManager_InstallRxCallback(s_serialReadHandle,
                                                                                         ERPC_SerialManagerRxCallback,
                                                                                         s_serialReadHandle))
                    {
                        status = kErpcStatus_Success;
                    }
                }
            }
        }
    }

    return status;
}

erpc_status_t UsbCdcTransport::underlyingReceive(uint8_t *data, uint32_t size)
{
    erpc_status_t status = kErpcStatus_ReceiveFailed;

    s_isTransferReceiveCompleted = false;

    if (kStatus_SerialManager_Success == SerialManager_ReadNonBlocking(s_serialReadHandle, data, size))
    {
/* wait until the receiving is finished */
#if !ERPC_THREADS_IS(NONE)
        (void)m_rxSemaphore.get();
#else
        while (!s_isTransferReceiveCompleted)
        {
        }
#endif
        status = kErpcStatus_Success;
    }

    return status;
}

erpc_status_t UsbCdcTransport::underlyingSend(const uint8_t *data, uint32_t size)
{
    erpc_status_t status = kErpcStatus_SendFailed;

    s_isTransferSendCompleted = false;

    if (kStatus_SerialManager_Success == SerialManager_WriteNonBlocking(s_serialWriteHandle, data, size))
    {
/* wait until the sending is finished */
#if !ERPC_THREADS_IS(NONE)
        (void)m_txSemaphore.get();
#else
        while (!s_isTransferSendCompleted)
        {
        }
#endif
        status = kErpcStatus_Success;
    }

    return status;
}
