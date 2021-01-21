/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * Copyright 2021 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_inter_thread_buffer_transport.h"

#include <cassert>

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

InterThreadBufferTransport::~InterThreadBufferTransport(void) {}

void InterThreadBufferTransport::linkWithPeer(InterThreadBufferTransport *peer)
{
    m_peer = peer;
    peer->m_peer = this;

    if (!m_state)
    {
        if (peer->m_state == NULL)
        {
            m_state = new SharedState;
            peer->m_state = m_state;
        }
        else
        {
            m_state = peer->m_state;
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
