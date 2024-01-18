/*
 * Copyright 2017 NXP
 * Copyright 2021 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_rpmsg_linux_transport.hpp"

#include <unistd.h>

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

RPMsgLinuxTransport::RPMsgLinuxTransport(RPMsgEndpoint *endPoint, int16_t remote) :
Transport(), m_endPoint(endPoint), m_remote(remote)
{
}

RPMsgLinuxTransport::~RPMsgLinuxTransport(void) {}

erpc_status_t RPMsgLinuxTransport::init(void)
{
    erpc_assert(m_endPoint != NULL);

    return (m_endPoint->init()) ? kErpcStatus_Fail : kErpcStatus_Success;
}

erpc_status_t RPMsgLinuxTransport::send(MessageBuffer *message)
{
    int retval;

    retval = m_endPoint->send(message->get(), m_remote, message->getUsed());

    return (retval > 0) ? kErpcStatus_Success : kErpcStatus_Fail;
}
erpc_status_t RPMsgLinuxTransport::receive(MessageBuffer *message)
{
    int32_t ret;
    erpc_status_t err;

    while (true)
    {
        ret = m_endPoint->receive(message->get(), message->getLength());
        if (ret == 0)
        {
            usleep(1); // 1us sleep
        }
        else
        {
            if (ret > 0)
            {
                err = kErpcStatus_Success;
            }
            else
            {
                err = kErpcStatus_Fail;
            }
            break;
        }
    }

    return ret;
}
