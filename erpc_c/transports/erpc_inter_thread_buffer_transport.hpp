/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__INTER_THREAD_BUFFER_TRANSPORT_H_
#define _EMBEDDED_RPC__INTER_THREAD_BUFFER_TRANSPORT_H_

#include "erpc_message_buffer.hpp"
#include "erpc_threading.h"
#include "erpc_transport.hpp"

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
    InterThreadBufferTransport(void)
    : Transport()
    , m_state(NULL)
    , m_peer(NULL)
    , m_inSem()
    , m_outSem(1)
    , m_inBuffer(NULL)
    {
    }
    virtual ~InterThreadBufferTransport(void);

    void linkWithPeer(InterThreadBufferTransport *peer);

    virtual erpc_status_t receive(MessageBuffer *message)override;
    virtual erpc_status_t send(MessageBuffer *message)override;

    virtual int32_t getAvailable(void) const { return 0; }

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
