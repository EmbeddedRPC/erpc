/*
 * The Clear BSD License
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted (subject to the limitations in the disclaimer below) provided
 * that the following conditions are met:
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
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE.
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

#include "inter_thread_buffer_transport.h"
#include <cassert>

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

InterThreadBufferTransport::~InterThreadBufferTransport() {}

void InterThreadBufferTransport::linkWithPeer(InterThreadBufferTransport *peer)
{
    m_peer = peer;
    peer->m_peer = this;

    if (!m_state)
    {
        if (peer->m_state)
        {
            m_state = peer->m_state;
        }
        else
        {
            m_state = new SharedState;
            peer->m_state = m_state;
        }
    }
}

erpc_status_t InterThreadBufferTransport::receive(MessageBuffer *message)
{
    assert(m_state && m_peer);

    m_inSem.get();

    m_state->m_mutex.lock();

    assert(m_inBuffer);
    message->copy(m_inBuffer);
    m_inBuffer = NULL;

    m_outSem.put();

    m_state->m_mutex.unlock();

    return kErpcStatus_Success;
}

erpc_status_t InterThreadBufferTransport::send(const MessageBuffer *message)
{
    assert(m_state && m_peer);

    m_peer->m_outSem.get();

    m_state->m_mutex.lock();

    assert(m_peer->m_inBuffer == NULL);
    m_peer->m_inBuffer = const_cast<MessageBuffer *>(message);
    m_peer->m_inSem.put();

    m_state->m_mutex.unlock();

    return kErpcStatus_Success;
}
