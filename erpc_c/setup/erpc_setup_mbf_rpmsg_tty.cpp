/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * Copyright 2021 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_config_internal.h"
#include "erpc_framed_transport.h"
#include "erpc_manually_constructed.h"
#include "erpc_mbf_setup.h"
#include "erpc_message_buffer.h"
#include "erpc_rpmsg_lite_base_transport.h"

#include "rpmsg_lite.h"

using namespace erpc;

#define TIMEOUT_MS 10

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

/*!
 * @brief RPMsg TTY message buffer factory
 */
class RPMsgTTYMessageBufferFactory : public MessageBufferFactory
{
public:
    /*!
     * @brief Constructor.
     *
     * @param [in] rpmsg Pointer to instance of RPMSG lite.
     */
    RPMsgTTYMessageBufferFactory(struct rpmsg_lite_instance *rpmsg) { m_rpmsg = rpmsg; }

    /*!
     * @brief Destructor
     */
    virtual ~RPMsgTTYMessageBufferFactory(void) {}

    /*!
     * @brief This function creates new message buffer.
     *
     * @return MessageBuffer New created MessageBuffer.
     */
    virtual MessageBuffer create(void)
    {
        void *buf = NULL;
        uint32_t size = 0;
        buf = rpmsg_lite_alloc_tx_buffer(m_rpmsg, &size, TIMEOUT_MS);

        erpc_assert(NULL != buf);
        return MessageBuffer(&((uint8_t *)buf)[sizeof(FramedTransport::Header)],
                             size - sizeof(FramedTransport::Header));
    }

    /*!
     * @brief This function disposes message buffer.
     *
     * @param[in] buf MessageBuffer to dispose.
     */
    virtual void dispose(MessageBuffer *buf)
    {
        erpc_assert(buf);
        void *tmp = (void *)buf->get();
        if (tmp != NULL)
        {
            int32_t ret;
            ret = rpmsg_lite_release_rx_buffer(m_rpmsg, (void *)(((uint8_t *)tmp) - sizeof(FramedTransport::Header)));
            if (ret != RL_SUCCESS)
            {
                // error
            }
        }
    }

    virtual erpc_status_t prepareServerBufferForSend(MessageBuffer *message)
    {
        erpc_status_t status;

        dispose(message);
        *message = create();
        if (message->get() != NULL)
        {
            status = kErpcStatus_Success;
        }
        else
        {
            status = kErpcStatus_MemoryError;
        }

        return status;
    }

    virtual bool createServerBuffer(void) { return false; }

protected:
    struct rpmsg_lite_instance *m_rpmsg; /*!< Pointer to instance of RPMSG lite. */
};

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

ERPC_MANUALLY_CONSTRUCTED(RPMsgTTYMessageBufferFactory, s_msgFactory);

erpc_mbf_t erpc_mbf_rpmsg_tty_init(erpc_transport_t transport)
{
    s_msgFactory.construct(reinterpret_cast<RPMsgBaseTransport *>(transport)->get_rpmsg_lite_instance());
    return reinterpret_cast<erpc_mbf_t>(s_msgFactory.get());
}
