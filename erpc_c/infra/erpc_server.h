/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * Copyright 2020 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__SERVER_H_
#define _EMBEDDED_RPC__SERVER_H_

#include "erpc_client_server_common.h"
#include "erpc_codec.h"
#include "erpc_config_internal.h"
#if ERPC_NESTED_CALLS
#include "erpc_client_manager.h"
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
    virtual ~Service(void) {}

    /*!
     * @brief Return service id number.
     *
     * @return Service id number.
     */
    uint32_t getServiceId(void) const { return m_serviceId; }

    /*!
     * @brief Return next service.
     *
     * @return Pointer to next service.
     */
    Service *getNext(void) { return m_next; }

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
    virtual erpc_status_t handleInvocation(uint32_t methodId, uint32_t sequence, Codec *codec,
                                           MessageBufferFactory *messageFactory) = 0;

protected:
    uint32_t m_serviceId; /*!< Service unique id. */
    Service *m_next;      /*!< Pointer to next service. */
};

/*!
 * @brief Based server functionality.
 *
 * @ingroup infra_server
 */
class Server : public ClientServerCommon
{
public:
    /*!
     * @brief Constructor.
     *
     * This function initializes object attributes.
     */
    Server(void)
    : ClientServerCommon()
    , m_messageFactory()
    , m_codecFactory()
    , m_transport()
    , m_firstService()
    {
    }

    /*!
     * @brief ClientManager destructor
     */
    virtual ~Server(void) {}

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
     * @brief Remove service.
     *
     * @param[in] service Service to remove.
     */
    void removeService(Service *service);

    /*!
     * @brief This function runs the server.
     */
    virtual erpc_status_t run(void) = 0;

    /*!
     * @brief This function stop the server.
     */
    virtual void stop(void) = 0;

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
    virtual erpc_status_t processMessage(Codec *codec, message_type_t msgType, uint32_t serviceId, uint32_t methodId,
                                         uint32_t sequence);

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
    virtual erpc_status_t readHeadOfMessage(Codec *codec, message_type_t &msgType, uint32_t &serviceId,
                                            uint32_t &methodId, uint32_t &sequence);

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
