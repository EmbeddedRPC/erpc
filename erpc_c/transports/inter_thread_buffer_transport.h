/*
 * The Clear BSD License
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted (subject to the limitations in the disclaimer below) provided
 *  that the following conditions are met:
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

#ifndef _EMBEDDED_RPC__INTER_THREAD_BUFFER_TRANSPORT_H_
#define _EMBEDDED_RPC__INTER_THREAD_BUFFER_TRANSPORT_H_

#include "erpc_threading.h"
#include "message_buffer.h"
#include "transport.h"

/*!
 * @addtogroup itbp_transport
 * @{
 * @file
 */

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpc {
/*!
 * @brief Very basic transport to send/receive messages between threads.
 *
 * Can send and receive message buffers to/from another instance of the same class
 * associated with another thread. Only one other thread may be communicated with. To set
 * things up first create two instances. Then call the linkWithPeer() method on the
 * first one, passing the second. The two instances of this class do not have to be
 * created on their respective threads, but the send() and receive() calls must be
 * invoked on the appropriate thread.
 *
 * Only a single message may be pending for each of the two threads. If a message is
 * pending for a thread and another is sent, then the sender will block until the
 * currently pending message is received.
 *
 * @ingroup itbp_transport
 */
class InterThreadBufferTransport : public Transport
{
public:
    InterThreadBufferTransport()
    : Transport()
    , m_state(NULL)
    , m_peer(NULL)
    , m_inSem()
    , m_outSem(1)
    , m_inBuffer(NULL)
    {
    }
    virtual ~InterThreadBufferTransport();

    void linkWithPeer(InterThreadBufferTransport *peer);

    virtual erpc_status_t receive(MessageBuffer *message);
    virtual erpc_status_t send(const MessageBuffer *message);

    virtual int32_t getAvailable() const { return 0; }

protected:
    struct SharedState
    {
        Mutex m_mutex;
    };

    SharedState *m_state;               /*!< */
    InterThreadBufferTransport *m_peer; /*!< */
    Semaphore m_inSem;                  /*!< */
    Semaphore m_outSem;                 /*!< */
    MessageBuffer *m_inBuffer;          /*!< */
};

} // namespace erpc

/*! @} */

#endif // _EMBEDDED_RPC__INTER_THREAD_BUFFER_TRANSPORT_H_
