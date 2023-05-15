/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * Copyright 2021 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_config_internal.h"
#include "erpc_manually_constructed.hpp"
#include "erpc_mbf_setup.h"
#include "erpc_message_buffer.hpp"
#include "erpc_rpmsg_lite_base_transport.hpp"

extern "C" {
#include "rpmsg_lite.h"
}

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

/*!
 * @brief RPMsg zero copy message buffer factory
 */
class RPMsgMessageBufferFactory : public MessageBufferFactory
{
public:
    /*!
     * @brief Constructor.
     *
     * @param [in] rpmsg Pointer to instance of RPMSG lite.
     */
    explicit RPMsgMessageBufferFactory(struct rpmsg_lite_instance *rpmsg) { m_rpmsg = rpmsg; }

    /*!
     * @brief Destructor
     */
    virtual ~RPMsgMessageBufferFactory(void) {}

    /*!
     * @brief This function creates new message buffer.
     *
     * @return MessageBuffer New created MessageBuffer.
     */
    virtual MessageBuffer create(void)
    {
        void *buf = NULL;
        uint32_t size = 0;
        buf = rpmsg_lite_alloc_tx_buffer(m_rpmsg, &size, RL_BLOCK);

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
            int32_t ret;
            ret = rpmsg_lite_release_rx_buffer(m_rpmsg, tmp);
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

ERPC_MANUALLY_CONSTRUCTED_STATIC(RPMsgMessageBufferFactory, s_msgFactory);

erpc_mbf_t erpc_mbf_rpmsg_init(erpc_transport_t transport)
{
    RPMsgMessageBufferFactory *msgFactory;

#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_STATIC
    if (s_msgFactory.isUsed())
    {
        msgFactory = NULL;
    }
    else
    {
        s_msgFactory.construct(reinterpret_cast<RPMsgBaseTransport *>(transport)->get_rpmsg_lite_instance());
        msgFactory = s_msgFactory.get();
    }
#elif ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_DYNAMIC
    msgFactory =
        new RPMsgMessageBufferFactory(reinterpret_cast<RPMsgBaseTransport *>(transport)->get_rpmsg_lite_instance());
#else
#error "Unknown eRPC allocation policy!"
#endif

    return reinterpret_cast<erpc_mbf_t>(msgFactory);
}

void erpc_mbf_rpmsg_deinit(erpc_mbf_t mbf)
{
#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_STATIC
    (void)mbf;
    s_msgFactory.destroy();
#elif ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_DYNAMIC
    erpc_assert(mbf != NULL);

    RPMsgMessageBufferFactory *msgFactory = reinterpret_cast<RPMsgMessageBufferFactory *>(mbf);

    delete msgFactory;
#endif
}
