/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
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

#ifndef _EMBEDDED_RPC__CLIENT_MANAGER_H_
#define _EMBEDDED_RPC__CLIENT_MANAGER_H_

#ifdef __cplusplus
#include "erpc_config_internal.h"
#include "codec.h"
#if ERPC_NESTED_CALLS
#include "server.h"
#endif

/*!
 * @addtogroup infra_client
 * @{
 * @file
 */

extern "C" {
#endif

typedef void (*client_error_handler_t)(erpc_status_t err); /*!< eRPC error handler function type. */

#ifdef __cplusplus
}

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpc {
class RequestContext;
#if ERPC_NESTED_CALLS
class Server;
#endif

/*!
 * @brief Base client implementation.
 *
 * @ingroup infra_client
 */
class ClientManager
{
public:
    /*!
     * @brief Constructor.
     *
     * This function initializes object attributes.
     */
    ClientManager()
    : m_messageFactory(NULL)
    , m_codecFactory(NULL)
    , m_transport(NULL)
    , m_sequence(0)
    , m_errorHandler(NULL)
#if ERPC_NESTED_CALLS
    , m_server(NULL)
#endif
    {
    }

    /*!
     * @brief ClientManager destructor
     */
    virtual ~ClientManager() {}

    /*!
     * @brief This function sets message buffer factory to use.
     *
     * @param[in] factory Message buffer factory to use.
     */
    void setMessageBufferFactory(MessageBufferFactory *factory) { m_messageFactory = factory; }

    /*!
     * @brief This function sets codec factory to use.
     *
     * @param[in] factory Codec factory to use.
     */
    void setCodecFactory(CodecFactory *factory) { m_codecFactory = factory; }

    /*!
     * @brief This function sets transport layer to use.
     *
     * It also set messageBufferFactory to the same as in transport layer.
     *
     * @param[in] transport Transport layer to use.
     */
    void setTransport(Transport *transport);

    /*!
     * @brief This function creates request context.
     *
     * @param[in] isOneway True if need send data only, else false.
     */
    virtual RequestContext createRequest(bool isOneway);

    /*!
     * @brief This function performs request.
     *
     * @param[in] request Request context to perform.
     */
    virtual erpc_status_t performRequest(RequestContext &request);

    /*!
     * @brief This function releases request context.
     *
     * @param[in] request Request context to release.
     */
    virtual void releaseRequest(RequestContext &request);

    /*!
     * @brief This functions sets error handler function for infrastructure errors.
     *
     * @param[in] error_handler Pointer to error handler function.
     */
    void setErrorHandler(client_error_handler_t error_handler) { m_errorHandler = error_handler; }

    /*!
     * @brief This functions returns error handler function for infrastructure errors.
     *
     * @return Pointer to error handler function.
     */
    client_error_handler_t getErrorHandler() { return m_errorHandler; }

#if ERPC_NESTED_CALLS
    /*!
     * @brief This function performs nested request.
     *
     * Used when from eRPC function server implementation context is called new eRPC function.
     *
     * @param[in] request Request context to perform.
     */
    virtual erpc_status_t performNestedRequest(RequestContext &request);

    /*!
     * @brief This function sets server used for nested calls.
     *
     * @param[in] server Server used for nested calls.
     */
    void setServer(Server *server) { m_server = server; }
#endif

protected:
    MessageBufferFactory *m_messageFactory; //!< Message buffer factory to use.
    CodecFactory *m_codecFactory;           //!< Codec to use.
    Transport *m_transport;                 //!< Transport layer to use.
    uint32_t m_sequence;                    //!< Sequence number.
    client_error_handler_t m_errorHandler;  //!< Pointer to function error handler.
#if ERPC_NESTED_CALLS
    Server *m_server; //!< Server used for nested calls.
#endif

    //! @brief Validate that an incoming message is a reply.
    virtual erpc_status_t verifyReply(RequestContext &request);

    /*!
     * @brief Create message buffer and codec.
     *
     * The new codec is set to use the new message buffer. Both codec and buffer are allocated
     * with the relevant factories.
     *
     * @return Pointer to created codec with message buffer.
     */
    Codec *createBufferAndCodec();

private:
    ClientManager(const ClientManager &);            //!< Disable copy ctor.
    ClientManager &operator=(const ClientManager &); //!< Disable copy ctor.
};

/*!
 * @brief Encapsulates all information about a request.
 *
 * @ingroup infra_client
 */
class RequestContext
{
public:
    /*!
     * @brief Constructor.
     *
     * This function sets request context attributes.
     *
     * @param[in] sequence Sequence number.
     * @param[in] codec Set in inout codec.
     * @param[in] isOneway Set information if codec is only oneway or bidirectional.
     */
    RequestContext(uint32_t sequence, Codec *codec, bool isOneway)
    : m_sequence(sequence)
    , m_codec(codec)
    , m_oneway(isOneway)
    {
    }

    /*!
     * @brief Get inout codec (for writing).
     *
     * @return Inout codec.
     */
    Codec *getCodec() { return m_codec; }

    /*!
     * @brief Get sequence number (be sure that reply belong to current request).
     *
     * @return Sequence number.
     */
    uint32_t getSequence() const { return m_sequence; }

    /*!
     * @brief Returns information if request context is oneway or not.
     *
     * @retval True when request context is oneway direction, else false.
     */
    bool isOneway() const { return m_oneway; }

    /*!
     * @brief Set request context to be oneway type (only send data).
     *
     * @return Set request context to be oneway.
     */
    void setIsOneway(bool oneway) { m_oneway = oneway; }

protected:
    uint32_t m_sequence; //!< Sequence number. To be sure that reply belong to current request.
    Codec *m_codec;      //!< Inout codec. Codec for receiving and sending data.
    bool m_oneway;       //!< When true, request context will be oneway type (only send data).
};

} // namespace erpc

/*! @} */

#endif

#endif // _EMBEDDED_RPC__CLIENT_MANAGER_H_
