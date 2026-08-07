/*
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC_RTTHREAD_UART_TRANSPORT_H_
#define _EMBEDDED_RPC_RTTHREAD_UART_TRANSPORT_H_

#include "erpc_config_internal.h"
#include "erpc_framed_transport.hpp"

#include <rtdevice.h>
#include <rtthread.h>

#ifndef ERPC_RTTHREAD_UART_TRANSPORT_COUNT
#define ERPC_RTTHREAD_UART_TRANSPORT_COUNT (1U)
#endif /* ERPC_RTTHREAD_UART_TRANSPORT_COUNT */

/*!
 * @addtogroup uart_transport
 * @{
 * @file
 */

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpc {
/*!
 * @brief Transport to send and receive eRPC framed messages over an RT-Thread UART device.
 *
 * @ingroup uart_transport
 *
 * @note This transport owns the UART receive callback while active. Applications should not set another
 * rx_indicate callback on the same UART device during the transport lifetime.
 *
 * @note The UART transport registry is updated only during init/deinit. Runtime RX callbacks read the registry
 * without locking. Applications must not deinitialize a transport while it is being used by an eRPC client/server
 * or while another thread may access the same transport.
 */
class RtThreadUartTransport : public FramedTransport
{
public:
    /*!
     * @brief Constructor.
     *
     * @param[in] devName RT-Thread UART device name, for example "uart2".
     * @param[in] baudRate UART baud rate.
     */
    RtThreadUartTransport(const char *devName, uint32_t baudRate);

    /*!
     * @brief Destructor.
     */
    virtual ~RtThreadUartTransport(void);

    /*!
     * @brief Initialize the RT-Thread UART device.
     *
     * @retval kErpcStatus_InitFailed UART device initialization failed.
     * @retval kErpcStatus_Success UART device initialization succeeded.
     */
    virtual erpc_status_t init(void);

private:
    using FramedTransport::underlyingReceive;
    using FramedTransport::underlyingSend;

    /*!
     * @brief Receive bytes from the UART device.
     *
     * @param[out] data Preallocated buffer for receiving data.
     * @param[in] size Number of bytes to receive.
     *
     * @retval kErpcStatus_ReceiveFailed UART read failed.
     * @retval kErpcStatus_Success All requested bytes were received.
     */
    virtual erpc_status_t underlyingReceive(uint8_t *data, uint32_t size) override;

    /*!
     * @brief Write bytes to the UART device.
     *
     * @param[in] data Buffer to send.
     * @param[in] size Number of bytes to send.
     *
     * @retval kErpcStatus_SendFailed UART write failed.
     * @retval kErpcStatus_Success All requested bytes were sent.
     */
    virtual erpc_status_t underlyingSend(const uint8_t *data, uint32_t size) override;

    /*!
     * @brief RT-Thread UART receive notification callback.
     *
     * @param[in] dev UART device that received data.
     * @param[in] size Number of received bytes reported by the driver.
     *
     * @return RT_EOK.
     */
    static rt_err_t rxIndicate(rt_device_t dev, rt_size_t size);

    /*!
     * @brief Register this transport in the fixed device-to-transport registry.
     *
     * @return true when the transport was registered successfully.
     */
    bool registerTransport(void);

    /*!
     * @brief Remove this transport from the fixed device-to-transport registry.
     */
    void unregisterTransport(void);

    /*!
     * @brief Find the transport bound to a UART device.
     *
     * The registry is updated only during init/deinit. Runtime RX callbacks read it without locking.
     *
     * @param[in] dev RT-Thread UART device handle.
     *
     * @return Matching transport or NULL.
     */
    static RtThreadUartTransport *findByDevice(rt_device_t dev);

    /*!
     * @brief Find an empty fixed registry slot.
     *
     * The registry is updated only during init/deinit. This helper is not thread-safe.
     *
     * @return Slot index or -1 when the registry is full.
     */
    static int findEmptySlot(void);

private:
    const char *m_devName;             /*!< RT-Thread UART device name. */
    uint32_t m_baudRate;               /*!< UART baud rate. */
    rt_device_t m_dev;                 /*!< RT-Thread UART device handle. */
    struct rt_semaphore m_rxSemaphore; /*!< Semaphore released by the UART RX callback. */
    bool m_isOpen;                     /*!< Whether the UART device has been opened by this transport. */
    bool m_isSemInitialized;           /*!< Whether m_rxSemaphore has been initialized. */
    bool m_isRxIndicateSet;            /*!< Whether this transport has installed the UART RX callback. */
    bool m_isRegistered;               /*!< Whether this transport is in the fixed device registry. */

    /*!
     * @brief Fixed registry entry that maps one UART device to one transport.
     */
    struct RegistryEntry
    {
        rt_device_t dev;                  /*!< Registered RT-Thread UART device. */
        RtThreadUartTransport *transport; /*!< Transport bound to the UART device. */
    };

    static RegistryEntry s_registry[ERPC_RTTHREAD_UART_TRANSPORT_COUNT]; /*!< Fixed device-to-transport registry. */
};

} // namespace erpc

/*! @} */

#endif /* _EMBEDDED_RPC_RTTHREAD_UART_TRANSPORT_H_ */
