/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * Copyright 2020 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__CLIENT_MANAGER_H_
#define _EMBEDDED_RPC__CLIENT_MANAGER_H_

#ifdef __cplusplus
#include "erpc_client_server_common.h"
#include "erpc_codec.h"
#include "erpc_config_internal.h"
#if ERPC_NESTED_CALLS
#include "erpc_server.h"
#include "erpc_threading.h"
#endif

/*!
 * @addtogroup infra_client
 * @{
 * @file
 */

extern "C" {
#endif

typedef void (*client_error_handler_t)(erpc_status_t err,
                                       uint32_t functionID); /*!< eRPC error handler function type. */

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
class ClientManager : public ClientServerCommon
{
public:
    /*!
     * @brief Constructor.
     *
     * This function initializes object attributes.
     */
    ClientManager(void)
    : ClientServerCommon()
    , m_messageFactory(NULL)
    , m_codecFactory(NULL)
    , m_transport(NULL)
    , m_sequence(0)
    , m_errorHandler(NULL)
#if ERPC_NESTED_CALLS
    , m_server(NULL)
    , m_serverThreadId(NULL)
#endif
    {
    }

    /*!
     * @brief ClientManager destructor
     */
    virtual ~ClientManager(void) {}

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
    virtual void performRequest(RequestContext &request);

    /*!
     * @brief This function releases request context.
     *
     * @param[in] request Request context to release.
     */
    virtual void releaseRequest(RequestContext &request);

    /*!
     * @brief This function sets error handler function for infrastructure errors.
     *
     * @param[in] error_handler Pointer to error handler function.
     */
    void setErrorHandler(client_error_handler_t error_handler) { m_errorHandler = error_handler; }

    /*!
     * @brief This function calls error handler callback function with given status.
     *
     * Function is called in client shim code at the end of function when error occurred.
     *
     * @param[in] err Specify function status at the end of eRPC call.
     * @param[in] functionID Specify eRPC function call.
     */
    void callErrorHandler(erpc_status_t err, uint32_t functionID);

#if ERPC_NESTED_CALLS
    /*!
     * @brief This function sets server used for nested calls.
     *
     * @param[in] server Server used for nested calls.
     */
    void setServer(Server *server) { m_server = server; }

    /*!
     * @brief This function sets server thread id.
     *
     * @param[in] serverThreadId Id of thread where server run function is executed.
     */
    void setServerThreadId(Thread::thread_id_t serverThreadId) { m_serverThreadId = serverThreadId; }
#endif

protected:
    MessageBufferFactory *m_messageFactory; //!< Message buffer factory to use.
    CodecFactory *m_codecFactory;           //!< Codec to use.
    Transport *m_transport;                 //!< Transport layer to use.
    uint32_t m_sequence;                    //!< Sequence number.
    client_error_handler_t m_errorHandler;  //!< Pointer to function error handler.
#if ERPC_NESTED_CALLS
    Server *m_server;                     //!< Server used for nested calls.
    Thread::thread_id_t m_serverThreadId; //!< Thread in which server run function is called.
#endif

    /*!
     * @brief This function performs request.
     *
     * Should be called in non server context (do not call another eRPC function in server
     * remote call implementation).
     *
     * @param[in] request Request context to perform.
     */
    virtual void performClientRequest(RequestContext &request);

#if ERPC_NESTED_CALLS
    /*!
     * @brief This function performs nested request.
     *
     * Used when from eRPC function server implementation context is called new eRPC function.
     *
     * @param[in] request Request context to perform.
     */
    virtual void performNestedClientRequest(RequestContext &request);
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
    Codec *createBufferAndCodec(void);

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
    RequestContext(uint32_t sequence, Codec *codec, bool argIsOneway)
    : m_sequence(sequence)
    , m_codec(codec)
    , m_oneway(argIsOneway)
    {
    }

    /*!
     * @brief Get inout codec (for writing).
     *
     * @return Inout codec.
     */
    Codec *getCodec(void) { return m_codec; }

    /*!
     * @brief Get sequence number (be sure that reply belong to current request).
     *
     * @return Sequence number.
     */
    uint32_t getSequence(void) const { return m_sequence; }

    /*!
     * @brief Returns information if request context is oneway or not.
     *
     * @retval True when request context is oneway direction, else false.
     */
    bool isOneway(void) const { return m_oneway; }

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
