/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
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
#if !defined(__embedded_rpc__transport_arbitrator__)
#define __embedded_rpc__transport_arbitrator__

#include "transport.h"
#include "codec.h"
#include "client_manager.h"
#include "erpc_threading.h"

/*!
 * @addtogroup infra_transport
 * @{
 * @file
 */

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpc
{
class Codec;

/*!
 * @brief Interposer to share transport between client and server.
 *
 * Allocates memory for client receive request info. This memory is never freed. It is put on a
 * free list and is reused for future client receives. The class will allocate as many client
 * receive info structs as the maximum number of simultaneous client receive requests from different
 * threads.
 *
 * @ingroup infra_transport
 */
class TransportArbitrator : public Transport
{
public:

    //! @brief Represents a single client's receive request.
    typedef uintptr_t client_token_t;

    TransportArbitrator();
    virtual ~TransportArbitrator();

    void setSharedTransport(Transport * shared) { m_sharedTransport = shared; }
    void setCodec(Codec * codec) { m_codec = codec; }

    //! @brief Receive method for the server.
    virtual status_t receive(MessageBuffer *message);

    //! @brief Add a client request to the client list.
    //!
    //! This call is made by the client thread prior to sending the invocation to the server. It
    //! ensures that the transport arbitrator has the client's response message buffer ready in
    //! case it sees the response before the client even has a chance to call clientReceive().
    //!
    //! A token is returned to the client
    client_token_t prepareClientReceive(RequestContext &request);

    //! @brief Receive method for the client.
    //!
    //! Blocks until the a reply message is received with the expected sequence number that is
    //! associated with @a token. The client must have called prepareClientReceive() previously.
    //!
    //! @param token The token previously returned by prepareClientReceive().
    status_t clientReceive(client_token_t token);

    //! @brief Shared client/server send method.
    virtual status_t send(const MessageBuffer *message);

protected:
    Transport * m_sharedTransport;  //!< Transport being shared through this arbitrator.
    Codec * m_codec;                //!< Codec used to read incoming message headers.

    /*!
     * @brief Request info for a client trying to receive a response.
     */
    struct PendingClientInfo
    {
        RequestContext *m_request;
        Semaphore m_sem;
        bool m_isValid;
        PendingClientInfo *m_next;

        PendingClientInfo();
        ~PendingClientInfo();
    };

    PendingClientInfo *m_clientList;        //!< Active client receive requests.
    PendingClientInfo *m_clientFreeList;    //!< Unused client receive info structs.
    Mutex m_clientListMutex;                //!< Mutex guarding the client active and free lists.

    PendingClientInfo *addPendingClient();
    void removePendingClient(PendingClientInfo *info);
    void freeClientList(PendingClientInfo *list);
};

} // namespace erpc

/*! @} */

#endif // defined(__embedded_rpc__transport_arbitrator__)
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
