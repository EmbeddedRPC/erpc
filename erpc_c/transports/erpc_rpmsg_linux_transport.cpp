/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_rpmsg_linux_transport.h"
#include <cassert>
#include <unistd.h>

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

RPMsgLinuxTransport::RPMsgLinuxTransport(RPMsgEndpoint *endPoint, int16_t remote)
: Transport()
, m_endPoint(endPoint)
, m_remote(remote)
{
}

RPMsgLinuxTransport::~RPMsgLinuxTransport(void) {}

erpc_status_t RPMsgLinuxTransport::init(void)
{
    assert(m_endPoint != NULL);

    if (!m_endPoint->init())
    {
        return kErpcStatus_Success;
    }
    else
    {
        return kErpcStatus_Fail;
    }
}

erpc_status_t RPMsgLinuxTransport::send(MessageBuffer *message)
{

    int retval;
    retval = m_endPoint->send(message->get(), m_remote, message->getUsed());
    if (retval > 0)
        return kErpcStatus_Success;
    else
        return kErpcStatus_Fail;
}
erpc_status_t RPMsgLinuxTransport::receive(MessageBuffer *message)
{
    int32_t ret;

    while (true)
    {
        ret = m_endPoint->receive(message->get(), message->getLength());

        if (ret > 0)
            return kErpcStatus_Success;
        if (ret < 0)
            return kErpcStatus_Fail;
        else
            usleep(1); // 1us sleep
    }
}
