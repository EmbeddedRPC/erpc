/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "erpc_config_internal.h"
#include "erpc_mbf_setup.h"
#include "framed_transport.h"
#include "manually_constructed.h"
#include "message_buffer.h"
#include "rpmsg_lite.h"
#include "rpmsg_lite_base_transport.h"
#include <assert.h>

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
    RPMsgTTYMessageBufferFactory(struct rpmsg_lite_instance *rpmsg)
    {
        m_rpmsg = rpmsg;
    }

    /*!
   * @brief Destructor
   */
    virtual ~RPMsgTTYMessageBufferFactory() {}

    /*!
   * @brief This function creates new message buffer.
   *
   * @return MessageBuffer New created MessageBuffer.
   */
    virtual MessageBuffer create()
    {
        void *buf = NULL;
        unsigned long size = 0;
        buf = rpmsg_lite_alloc_tx_buffer(m_rpmsg, &size, TIMEOUT_MS);

        assert(NULL != buf);
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
        assert(buf);
        void *tmp = (void *)buf->get();
        if (tmp)
        {
            int ret;
            ret = rpmsg_lite_release_rx_buffer(
                m_rpmsg,
                (void *)(((uint8_t *)tmp) - sizeof(FramedTransport::Header)));
            if (ret != RL_SUCCESS)
            {
                // error
            }
        }
    }

    virtual erpc_status_t prepareServerBufferForSend(MessageBuffer *message)
    {
        dispose(message);
        *message = create();
        if (message->get() != NULL)
        {
            return kErpcStatus_Success;
        }
        else
        {
            return kErpcStatus_MemoryError;
        }
    }

    virtual bool createServerBuffer() { return false; }

protected:
    struct rpmsg_lite_instance
        *m_rpmsg; /*!< Pointer to instance of RPMSG lite. */
};

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

static ManuallyConstructed<RPMsgTTYMessageBufferFactory> s_msgFactory;

erpc_mbf_t erpc_mbf_rpmsg_tty_init(erpc_transport_t transport)
{
    s_msgFactory.construct(reinterpret_cast<RPMsgBaseTransport *>(transport)
                               ->get_rpmsg_lite_instance());
    return reinterpret_cast<erpc_mbf_t>(s_msgFactory.get());
}
