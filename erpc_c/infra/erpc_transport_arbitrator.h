/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#if !defined(__embedded_rpc__transport_arbitrator__)
#define __embedded_rpc__transport_arbitrator__

#include "erpc_client_manager.h"
#include "erpc_codec.h"
#include "erpc_threading.h"
#include "erpc_transport.h"

/*!
 * @addtogroup infra_transport
 * @{
 * @file
 */

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpc {
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

    /*!
     * @brief Constructor.
     */
    TransportArbitrator(void);

    /*!
     * @brief Destructor.
     */
    virtual ~TransportArbitrator(void);

    /*!
     * @brief This function set shared client/server transport.
     *
     * @param[in] shared Shared client/server transport.
     */
    void setSharedTransport(Transport *shared) { m_sharedTransport = shared; }

    /*!
     * @brief This function set codec.
     *
     * @param[in] codec Codec.
     */
    void setCodec(Codec *codec) { m_codec = codec; }

    //! @brief Receive method for the server.
    virtual erpc_status_t receive(MessageBuffer *message);

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
    erpc_status_t clientReceive(client_token_t token);

    //! @brief Shared client/server send method.
    virtual erpc_status_t send(MessageBuffer *message);

    /*!
     * @brief This functions sets the CRC-16 implementation.
     *
     * @param[in] crcImpl Object containing crc-16 compute function.
     */
    virtual void setCrc16(Crc16 *crcImpl);

    /*!
     * @brief Check if the underlying shared transport has a message
     *
     * @retval The underlying transport is expected to return true when a message is available to
     *         process and false otherwise.
     */
    virtual bool hasMessage(void);

protected:
    Transport *m_sharedTransport; //!< Transport being shared through this arbitrator.
    Codec *m_codec;               //!< Codec used to read incoming message headers.

    /*!
     * @brief Request info for a client trying to receive a response.
     */
    struct PendingClientInfo
    {
        RequestContext *m_request; /*!< Client request context. */
        Semaphore m_sem;           /*!< Client semaphore. */
        bool m_isValid;            /*!< This struct validation. */
        PendingClientInfo *m_next; /*!< Next client pending information. */

        /*!
         * @brief Constructor.
         */
        PendingClientInfo(void);

        /*!
         * @brief Destructor.
         */
        ~PendingClientInfo(void);
    };

    PendingClientInfo *m_clientList;     //!< Active client receive requests.
    PendingClientInfo *m_clientFreeList; //!< Unused client receive info structs.
    Mutex m_clientListMutex;             //!< Mutex guarding the client active and free lists.

    /*!
     * @brief This function adds pending client.
     *
     * @return Pending client information.
     */
    PendingClientInfo *addPendingClient(void);

    /*!
     * @brief This function removes pending client.
     *
     * @param[in] info Pending client info to remove.
     */
    void removePendingClient(PendingClientInfo *info);

    /*!
     * @brief This function removes pending client list.
     *
     * @param[in] list Pending client list to remove.
     */
    void freeClientList(PendingClientInfo *list);
};

} // namespace erpc

/*! @} */

#endif // defined(__embedded_rpc__transport_arbitrator__)
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
