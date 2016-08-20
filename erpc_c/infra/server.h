/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
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

#ifndef _EMBEDDED_RPC__SERVER_H_
#define _EMBEDDED_RPC__SERVER_H_

#include "codec.h"

/*!
 * @addtogroup infra_server
 * @{
 * @file
 */

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpc
{
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
     * @param[in] inCodec For reading data.
     * @param[in] outCodec For sending data.
     *
     * @return Based on handleInvocation implementation.
     */
    virtual status_t handleInvocation(uint32_t methodId, uint32_t sequence, Codec *inCodec, Codec *outCodec) = 0;

protected:
    uint32_t m_serviceId; /*!< Service unique id. */
    Service *m_next;      /*!< Pointer to next service. */
};

/*!
 * @brief Based server functionality.
 *
 * @ingroup infra_server
 */
class Server
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
    , m_transportFactory()
    , m_firstService()
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
     * @brief Set TransportFactory to use.
     *
     * @param[in] factory TransportFactory to use.
     */
    void setTransportFactory(TransportFactory *factory) { m_transportFactory = factory; }
    /*!
     * @brief Add service.
     *
     * @param[in] service Service to use.
     */
    void addService(Service *service);

    /*!
     * @brief This function runs the server.
     */
    virtual status_t run() = 0;

    /*!
     * @brief This function stop the server.
     */
    virtual void stop() = 0;

protected:
    MessageBufferFactory *m_messageFactory; /*!< Contains MessageBufferFactory to use. */
    CodecFactory *m_codecFactory;           /*!< Contains CodecFactory to use. */
    TransportFactory *m_transportFactory;   /*!< Contains TransportFatcory to use. */
    Service *m_firstService;                /*!< Contains pointer to first service. */

    /*!
     * @brief Process message.
     *
     * @param[in] inCodec In codec to use.
     * @param[in] outCodec Out codec to use.
     * @param[out] msgType Type of received message. Based on message type will be (will be not) sent respond.
     *
     * @returns #kErpcStatus_Success, #kErpcStatus_InvalidArgument or based on codec startReadMessage,
     * or based on service handleInvocation.
     */
    virtual status_t processMessage(Codec *inCodec, Codec *outCodec, message_type_t &msgType);

    /*!
     * @brief This function finds service base on service ID.
     *
     * @param[in] serviceId Service id number.
     *
     * @return Pointer to service or NULL.
     */
    virtual Service *findServiceWithId(uint32_t serviceId);

private:
    // Disable copy ctor.
    Server(const Server &);            /*!< Disable copy ctor. */
    Server &operator=(const Server &); /*!< Disable copy ctor. */
};

} // namespace erpc

/*! @} */

#endif // _EMBEDDED_RPC__SERVER_H_
