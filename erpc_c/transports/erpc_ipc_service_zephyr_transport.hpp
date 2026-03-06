/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__IPC_SERVICE_ZEPHYR_TRANSPORT_H_
#define _EMBEDDED_RPC__IPC_SERVICE_ZEPHYR_TRANSPORT_H_

#include "erpc_config_internal.h"
#include "erpc_crc16.hpp"
#include "erpc_message_buffer.hpp"
#include "erpc_static_queue.hpp"
#include "erpc_transport.hpp"

extern "C" {
#include <zephyr/device.h>
#include <zephyr/ipc/ipc_service.h>
#include <zephyr/kernel.h>
}

/*!
 * @addtogroup ipc_service_zephyr_transport
 * @{
 * @file
 */

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpc {

/*!
 * @brief Transport that uses Zephyr IPC Service for interprocessor messaging.
 *
 * @ingroup ipc_service_zephyr_transport
 */
class ZephyrIPCTransport : public Transport
{
public:
    /*!
     * @brief Constructor.
     *
     * This function initializes object attributes.
     */
    ZephyrIPCTransport(void);

    /*!
     * @brief ZephyrIPCTransport destructor
     */
    virtual ~ZephyrIPCTransport(void);

    /*!
     * @brief Initialize the Zephyr IPC Service transport.
     *
     * @param[in] ipc_instance Pointer to the IPC service device instance.
     * @param[in] ept_name Name of the endpoint to register.
     *
     * @retval kErpcStatus_Success When initialization was successful.
     * @retval kErpcStatus_InitFailed When initialization failed.
     */
    virtual erpc_status_t init(const struct device *ipc_instance, const char *ept_name);

    /*!
     * @brief Receive a message.
     *
     * This function blocks until a message is received.
     *
     * @param[out] message Message buffer to store the received message.
     *
     * @retval kErpcStatus_Success Successfully received message.
     * @retval kErpcStatus_ReceiveFailed Failed to receive message.
     */
    virtual erpc_status_t receive(MessageBuffer *message) override;

    /*!
     * @brief Send a message.
     *
     * @param[in] message Message buffer to send.
     *
     * @retval kErpcStatus_Success Successfully sent message.
     * @retval kErpcStatus_SendFailed Failed to send message.
     */
    virtual erpc_status_t send(MessageBuffer *message) override;

    /*!
     * @brief Check if a message is available.
     *
     * @return True if a message is available, false otherwise.
     */
    virtual bool hasMessage(void) override;

    /*!
     * @brief Set the CRC-16 implementation.
     *
     * @param[in] crcImpl Object containing crc-16 compute function.
     */
    virtual void setCrc16(Crc16 *crcImpl) override;

    /*!
     * @brief Get the CRC-16 object.
     *
     * @return Pointer to CRC-16 object.
     */
    virtual Crc16 *getCrc16(void) override;

    /*!
     * @brief Get the IPC endpoint for buffer operations.
     *
     * @return Pointer to IPC endpoint.
     */
    struct ipc_ept *getEndpoint(void) { return &m_ept; }

protected:
    /*!
     * @brief Endpoint bound callback.
     *
     * @param[in] priv Private user data (this pointer).
     */
    static void ept_bound_cb(void *priv);

    /*!
     * @brief Endpoint received callback.
     *
     * @param[in] data Pointer to received data.
     * @param[in] len Length of received data.
     * @param[in] priv Private user data (this pointer).
     */
    static void ept_recv_cb(const void *data, size_t len, void *priv);

    /*!
     * @brief Endpoint error callback.
     *
     * @param[in] message Error message.
     * @param[in] priv Private user data (this pointer).
     */
    static void ept_error_cb(const char *message, void *priv);

    const struct device *m_ipc_instance; /*!< IPC service device instance. */
    struct ipc_ept m_ept;                /*!< IPC endpoint. */
    struct ipc_ept_cfg m_ept_cfg;        /*!< IPC endpoint Configuration. */
    struct k_sem m_bound_sem;            /*!< Semaphore for endpoint binding. */
    struct k_sem m_rx_sem;               /*!< Semaphore for received messages. */

    StaticQueue<MessageBuffer, 2U * ERPC_DEFAULT_BUFFERS_COUNT> m_messageQueue; /*!< Queue of received messages. */

    Crc16 *m_crcImpl;      /*!< CRC-16 implementation. */
    bool m_is_initialized; /*!< Initialization flag. */
};

} // namespace erpc

/*! @} */

#endif // _EMBEDDED_RPC__IPC_SERVICE_ZEPHYR_TRANSPORT_H_
