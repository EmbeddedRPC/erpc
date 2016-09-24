/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
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
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
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

#include "rpmsg_rtos_transport.h"
#include <cassert>
#include <new>

#if !(__embedded_cplusplus)
using namespace std;
#endif

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

/* Remote device */
struct remote_device *RPMsgRTOSTransport::m_rdev = NULL;
struct rpmsg_channel *RPMsgRTOSTransport::m_app_rp_chnl = NULL;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

RPMsgRTOSTransport::RPMsgRTOSTransport()
: Transport()
{
}

RPMsgRTOSTransport::~RPMsgRTOSTransport()
{
    rpmsg_rtos_deinit(m_rdev);
}

erpc_status_t RPMsgRTOSTransport::init(int dev_id, int role)
{
    int ret_value;
    ret_value = rpmsg_rtos_init(dev_id, &m_rdev, role, &m_app_rp_chnl);

    return ret_value != RPMSG_SUCCESS ? kErpcStatus_InitFailed : kErpcStatus_Success;
}

erpc_status_t RPMsgRTOSTransport::receive(MessageBuffer *message)
{
    uint32_t length = message->getLength();
    uint8_t *freeBuffer = message->get();
    int readLength;
    int ret_value;

    ret_value = rpmsg_rtos_recv(m_app_rp_chnl->rp_ept, freeBuffer, &readLength, length, NULL, -1);
    if (ret_value != RPMSG_SUCCESS)
    {
        return kErpcStatus_ReceiveFailed;
    }

    message->setUsed(readLength);

    return kErpcStatus_Success;
}

erpc_status_t RPMsgRTOSTransport::send(const MessageBuffer *message)
{
    int ret_value;
    ret_value = rpmsg_rtos_send(m_app_rp_chnl->rp_ept, (void *)message->get(), message->getUsed(), m_app_rp_chnl->dst);

    return RPMSG_SUCCESS != ret_value ? kErpcStatus_SendFailed : kErpcStatus_Success;
}

MessageBuffer RPMsgMessageBufferFactory::create()
{
    uint8_t *buf = new (nothrow) uint8_t[kRpmsgMessageBufferSize];
    assert(NULL != buf);
    return MessageBuffer(buf, kRpmsgMessageBufferSize);
}

void RPMsgMessageBufferFactory::dispose(MessageBuffer *buf)
{
    assert(buf);
    if (buf->get())
    {
        delete[] buf->get();
    }
}
