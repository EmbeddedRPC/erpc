/*
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_uart_rtthread_transport.hpp"

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

RtThreadUartTransport::RegistryEntry RtThreadUartTransport::s_registry[ERPC_RTTHREAD_UART_TRANSPORT_COUNT] = {};

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

RtThreadUartTransport::RtThreadUartTransport(const char *devName, uint32_t baudRate) :
m_devName(devName), m_baudRate(baudRate), m_dev(RT_NULL), m_rxSemaphore(), m_isOpen(false), m_isSemInitialized(false),
m_isRxIndicateSet(false), m_isRegistered(false)
{
}

RtThreadUartTransport::~RtThreadUartTransport(void)
{
    if ((m_dev != RT_NULL) && m_isRxIndicateSet)
    {
        (void)rt_device_set_rx_indicate(m_dev, RT_NULL);
        m_isRxIndicateSet = false;
    }

    unregisterTransport();

    if ((m_dev != RT_NULL) && m_isOpen)
    {
        (void)rt_device_close(m_dev);
        m_isOpen = false;
    }

    if (m_isSemInitialized)
    {
        (void)rt_sem_detach(&m_rxSemaphore);
        m_isSemInitialized = false;
    }

    m_dev = RT_NULL;
}

erpc_status_t RtThreadUartTransport::init(void)
{
    erpc_status_t status = kErpcStatus_InitFailed;
    rt_err_t err;
    //serial v2
    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;

    if (m_devName == NULL)
    {
        return status;
    }

    m_dev = rt_device_find(m_devName);
    if (m_dev == RT_NULL)
    {
        return status;
    }

    config.rx_bufsz = ERPC_DEFAULT_BUFFER_SIZE;
    config.tx_bufsz = ERPC_DEFAULT_BUFFER_SIZE;
    config.dma_ping_bufsz = ERPC_DEFAULT_BUFFER_SIZE;
    config.baud_rate = m_baudRate;
    err = rt_device_control(m_dev, RT_DEVICE_CTRL_CONFIG, &config);
    if (err != RT_EOK)
    {
        return status;
    }

    if (rt_device_open(m_dev, RT_DEVICE_FLAG_RX_NON_BLOCKING | RT_DEVICE_FLAG_TX_NON_BLOCKING) != RT_EOK)
    {
        return status;
    }
    m_isOpen = true;

    if (!registerTransport())
    {
        return status;
    }

    err = rt_device_set_rx_indicate(m_dev, rxIndicate);
    if (err != RT_EOK)
    {
        return status;
    }
    m_isRxIndicateSet = true;

    err = rt_sem_init(&m_rxSemaphore, "erpc_rx", 0, RT_IPC_FLAG_PRIO);
    if (err != RT_EOK)
    {
        return status;
    }
    m_isSemInitialized = true;

    status = kErpcStatus_Success;
    return status;
}

erpc_status_t RtThreadUartTransport::underlyingReceive(uint8_t *data, uint32_t size)
{
    uint32_t received = 0;

    if ((m_dev == RT_NULL) || ((data == NULL) && (size > 0U)))
    {
        return kErpcStatus_ReceiveFailed;
    }

    while (received < size)
    {
        rt_size_t readSize = rt_device_read(m_dev, -1, &data[received], size - received);

        if (readSize > 0U)
        {
            received += readSize;
            continue;
        }

        if (rt_sem_take(&m_rxSemaphore, RT_WAITING_FOREVER) != RT_EOK)
        {
            return kErpcStatus_ReceiveFailed;
        }
    }

    return kErpcStatus_Success;
}

erpc_status_t RtThreadUartTransport::underlyingSend(const uint8_t *data, uint32_t size)
{
    uint32_t sent = 0;

    if ((m_dev == RT_NULL) || ((data == NULL) && (size > 0U)))
    {
        return kErpcStatus_SendFailed;
    }

    while (sent < size)
    {
        rt_size_t writeSize = rt_device_write(m_dev, 0, &data[sent], size - sent);

        if (writeSize == 0U)
        {
            return kErpcStatus_SendFailed;
        }

        sent += writeSize;
    }

    return kErpcStatus_Success;
}

rt_err_t RtThreadUartTransport::rxIndicate(rt_device_t dev, rt_size_t size)
{
    RtThreadUartTransport *transport;

    (void)size;

    transport = findByDevice(dev);
    if (transport != NULL)
    {
        (void)rt_sem_release(&transport->m_rxSemaphore);
    }

    return RT_EOK;
}

bool RtThreadUartTransport::registerTransport(void)
{
    bool registered = false;
    int emptySlot;

    if ((m_dev == RT_NULL) || m_isRegistered)
    {
        return false;
    }

    if (findByDevice(m_dev) == NULL)
    {
        emptySlot = findEmptySlot();
        if (emptySlot >= 0)
        {
            s_registry[emptySlot].transport = this;
            s_registry[emptySlot].dev = m_dev;
            m_isRegistered = true;
            registered = true;
        }
    }

    return registered;
}

void RtThreadUartTransport::unregisterTransport(void)
{
    if (!m_isRegistered)
    {
        return;
    }

    for (uint32_t i = 0U; i < ERPC_RTTHREAD_UART_TRANSPORT_COUNT; ++i)
    {
        if (s_registry[i].transport == this)
        {
            s_registry[i].dev = RT_NULL;
            s_registry[i].transport = NULL;
            m_isRegistered = false;
            break;
        }
    }
}

RtThreadUartTransport *RtThreadUartTransport::findByDevice(rt_device_t dev)
{
    for (uint32_t i = 0U; i < ERPC_RTTHREAD_UART_TRANSPORT_COUNT; ++i)
    {
        if (s_registry[i].dev == dev)
        {
            return s_registry[i].transport;
        }
    }

    return NULL;
}

int RtThreadUartTransport::findEmptySlot(void)
{
    for (uint32_t i = 0U; i < ERPC_RTTHREAD_UART_TRANSPORT_COUNT; ++i)
    {
        if (s_registry[i].dev == RT_NULL)
        {
            return (int)i;
        }
    }

    return -1;
}
