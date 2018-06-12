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

#ifndef _EMBEDDED_RPC__SERVER_H_
#define _EMBEDDED_RPC__SERVER_H_

#include "erpc_config_internal.h"
#include "codec.h"
#if ERPC_NESTED_CALLS
#include "client_manager.h"
#endif
#if ERPC_MESSAGE_LOGGING
#include "message_loggers.h"
#endif

/*!
 * @addtogroup infra_server
 * @{
 * @file
 */

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpc {
#if ERPC_NESTED_CALLS
class RequestContext;
#endif
/*!
 * @brief Abstract interface for service, which can be executed on server side.
 *
 * @ingroup infra_server
 */
class Service
{
public:
    /*!
     * @brief Constructor.
     *
     * This function initializes object attributes.
     */
    Service(uint32_t serviceId)
    : m_serviceId(serviceId)
    , m_next(NULL)
    {
    }

    /*!
     * @brief Service destructor
     */
    virtual ~Service() {}

    /*!
     * @brief Return service id number.
     *
     * @return Service id number.
     */
    uint32_t getServiceId() const { return m_serviceId; }

    /*!
     * @brief Return next service.
     *
     * @return Pointer to next service.
     */
    Service *getNext() { return m_next; }

    /*!
     * @brief Set next service.
     *
     * @param[in] next Pointer to next service.
     */
    void setNext(Service *next) { m_next = next; }

    /*!
     * @brief This function call function implementation of current service.
     *
     * @param[in] methodId Id number of function, which is requested.
     * @param[in] sequence Sequence number. To be sure that reply from server belongs to client request.
     * @param[in] codec For reading and writing data.
     * @param[in] messageFactory Used for setting output buffer.
     *
     * @return Based on handleInvocation implementation.
     */
    virtual erpc_status_t handleInvocation(uint32_t methodId, uint32_t sequence, Codec *codec, MessageBufferFactory *messageFactory) = 0;

protected:
    uint32_t m_serviceId; /*!< Service unique id. */
    Service *m_next;      /*!< Pointer to next service. */
};

/*!
 * @brief Based server functionality.
 *
 * @ingroup infra_server
 */
#if ERPC_MESSAGE_LOGGING
class Server : public MessageLoggers
#else
class Server
#endif
{
public:
    /*!
     * @brief Constructor.
     *
     * This function initializes object atributes.
     */
    Server()
    : m_messageFactory()
    , m_codecFactory()
    , m_transport()
    , m_firstService()
#if ERPC_MESSAGE_LOGGING
    , MessageLoggers()
#endif
    {
    }

    /*!
     * @brief ClientManager destructor
     */
    virtual ~Server() {}

    /*!
     * @brief Set MessageBufferFactory to use.
     *
     * @param[in] factory MessageBufferFactory to use.
     */
    void setMessageBufferFactory(MessageBufferFactory *factory) { m_messageFactory = factory; }

    /*!
     * @brief Set CodecFactory to use.
     *
     * @param[in] factory CodecFactory to use.
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
     * @brief Add service.
     *
     * @param[in] service Service to use.
     */
    void addService(Service *service);

    /*!
     * @brief This function runs the server.
     */
    virtual erpc_status_t run() = 0;

    /*!
     * @brief This function stop the server.
     */
    virtual void stop() = 0;

protected:
    MessageBufferFactory *m_messageFactory; /*!< Contains MessageBufferFactory to use. */
    CodecFactory *m_codecFactory;           /*!< Contains CodecFactory to use. */
    Transport *m_transport;                 /*!< Transport layer used to send and receive data. */
    Service *m_firstService;                /*!< Contains pointer to first service. */

    /*!
     * @brief Process message.
     *
     * @param[in] codec Inout codec to use.
     * @param[in] msgType Type of received message. Based on message type will be (will be not) sent respond.
     * @param[in] serviceId To identify interface.
     * @param[in] methodId To identify function in interface.
     * @param[in] sequence To connect correct answer with correct request.
     *
     * @returns #kErpcStatus_Success or based on codec startReadMessage.
     */
    virtual erpc_status_t processMessage(Codec *codec, message_type_t msgType, uint32_t serviceId, uint32_t methodId, uint32_t sequence);

    /*!
     * @brief Read head of message to identify type of message.
     *
     * @param[in] codec Inout codec to use.
     * @param[out] msgType Type of received message. Based on message type will be (will be not) sent respond.
     * @param[out] serviceId To identify interface.
     * @param[out] methodId To identify function in interface.
     * @param[out] sequence To connect correct answer with correct request.
     *
     * @returns #kErpcStatus_Success or based on service handleInvocation.
     */
    virtual erpc_status_t readHeadOfMessage(Codec *codec, message_type_t &msgType, uint32_t &serviceId, uint32_t &methodId, uint32_t &sequence);

    /*!
     * @brief This function finds service base on service ID.
     *
     * @param[in] serviceId Service id number.
     *
     * @return Pointer to service or NULL.
     */
    virtual Service *findServiceWithId(uint32_t serviceId);

#if ERPC_NESTED_CALLS
    friend class ClientManager;
    friend class ArbitratedClientManager;

    /*!
     * @brief This function runs the server.
     *
     * @param[in] Request context to check that answer was for nested call.
     */
    virtual erpc_status_t run(RequestContext &request) = 0;
#endif

private:
    // Disable copy ctor.
    Server(const Server &);            /*!< Disable copy ctor. */
    Server &operator=(const Server &); /*!< Disable copy ctor. */
};

} // namespace erpc

/*! @} */

#endif // _EMBEDDED_RPC__SERVER_H_
