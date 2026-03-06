/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_config_internal.h"
#include "erpc_manually_constructed.hpp"
#include "erpc_mbf_setup.h"
#include "erpc_message_buffer.hpp"
#include "erpc_ipc_service_zephyr_transport.hpp"

extern "C" {
#include <zephyr/ipc/ipc_service.h>
}

using namespace erpc;

#define TIMEOUT_MS 10

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

/*!
 * @brief IPC Service zero copy message buffer factory
 */
class IPCServiceMessageBufferFactory : public MessageBufferFactory
{
public:
    /*!
     * @brief Constructor.
     *
     * @param [in] ept Pointer to IPC Service endpoint.
     */
    explicit IPCServiceMessageBufferFactory(struct ipc_ept *ept) { m_ept = ept; }

    /*!
     * @brief Destructor
     */
    virtual ~IPCServiceMessageBufferFactory(void) {}

    /*!
     * @brief This function creates new message buffer.
     *
     * @return MessageBuffer New created MessageBuffer.
     */
    virtual MessageBuffer create(void)
    {
        void *buf = NULL;
        uint32_t size = 0;

        // Get TX buffer from IPC Service
        ipc_service_get_tx_buffer(m_ept, &buf, &size, K_MSEC(TIMEOUT_MS));

        erpc_assert(NULL != buf);
        return MessageBuffer(reinterpret_cast<uint8_t *>(buf), size);
    }

    /*!
     * @brief This function disposes message buffer.
     *
     * @param[in] buf MessageBuffer to dispose.
     */
    virtual void dispose(MessageBuffer *buf)
    {
        erpc_assert(buf != NULL);
        void *tmp = reinterpret_cast<void *>(buf->get());
        if (tmp != NULL)
        {
            // Release RX buffer back to IPC Service
            ipc_service_release_rx_buffer(m_ept, tmp);
        }
    }

    /*!
     * @brief Prepare server buffer for send.
     *
     * Disposes the old RX buffer and gets a new TX buffer.
     */
    virtual erpc_status_t prepareServerBufferForSend(MessageBuffer &message, uint8_t reserveHeaderSize = 0)
    {
        erpc_status_t status;

        dispose(&message);
        message = create();
        if (message.get() != NULL)
        {
            message.setUsed(reserveHeaderSize);
            status = kErpcStatus_Success;
        }
        else
        {
            status = kErpcStatus_MemoryError;
        }

        return status;
    }

    /*!
     * @brief Server doesn't create receive buffers - they come from callbacks.
     */
    virtual bool createServerBuffer(void) { return false; }

protected:
    struct ipc_ept *m_ept; /*!< Pointer to IPC Service endpoint. */
};

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

ERPC_MANUALLY_CONSTRUCTED_STATIC(IPCServiceMessageBufferFactory, s_msgFactory);

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

erpc_mbf_t erpc_mbf_ipc_service_init(erpc_transport_t transport)
{
    IPCServiceMessageBufferFactory *msgFactory;

    // Get the endpoint from the transport
    ZephyrIPCTransport *ipcTransport = reinterpret_cast<ZephyrIPCTransport *>(transport);
    struct ipc_ept *ept = ipcTransport->getEndpoint();

#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_STATIC
    if (s_msgFactory.isUsed())
    {
        msgFactory = NULL;
    }
    else
    {
        s_msgFactory.construct(ept);
        msgFactory = s_msgFactory.get();
    }
#elif ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_DYNAMIC
    msgFactory = new IPCServiceMessageBufferFactory(ept);
#else
#error "Unknown eRPC allocation policy!"
#endif

    return reinterpret_cast<erpc_mbf_t>(msgFactory);
}

void erpc_mbf_ipc_service_deinit(erpc_mbf_t mbf)
{
#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_STATIC
    (void)mbf;
    s_msgFactory.destroy();
#elif ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_DYNAMIC
    erpc_assert(mbf != NULL);

    IPCServiceMessageBufferFactory *msgFactory = reinterpret_cast<IPCServiceMessageBufferFactory *>(mbf);

    delete msgFactory;
#endif
}
