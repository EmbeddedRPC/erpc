/*
 * Copyright 2017 NXP
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

#include "rpmsg_linux_transport.h"
#include <cassert>
#include <new>
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

RPMsgLinuxTransport::~RPMsgLinuxTransport()
{
}

erpc_status_t RPMsgLinuxTransport::init()
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
            usleep(1); //1us sleep
    }
}
