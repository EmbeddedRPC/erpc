/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
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

#ifndef _EMBEDDED_RPC__CLIENT_MANAGER_H_
#define _EMBEDDED_RPC__CLIENT_MANAGER_H_

#include "codec.h"

/*!
 * @addtogroup infra_client
 * @{
 * @file
 */

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpc
{
class RequestContext;

/*!
 * @brief Base client implementation.
 *
 * @ingroup infra_client
 */
class ClientManager
{
public:
    typedef void (*error_handler_t)(erpc_status_t err);
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
     * @param[in] transport Transport layer to use.
     */
    void setTransport(Transport *transport) { m_transport = transport; }
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
    void setErrorHandler(error_handler_t error_handler) { m_errorHandler = error_handler; }
    /*!
     * @brief This functions returns error handler function for infrastructure errors.
     *
     * @return Pointer to error handler function.
     */
    error_handler_t getErrorHandler() { return m_errorHandler; }
protected:
    MessageBufferFactory *m_messageFactory; //!< Message buffer factory to use.
    CodecFactory *m_codecFactory;           //!< Codec to use.
    Transport *m_transport;                 //!< Transport layer to use.
    uint32_t m_sequence;                    //!< Sequence number.
    error_handler_t m_errorHandler;         //!< Pointer to function error handler.

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
     * @param[in] inCodec Set in codec.
     * @param[out] outCodec Set out codec.
     * @param[in] isOneway Set information if codec is only oneway or bidirectional.
     */
    RequestContext(uint32_t sequence, Codec *inCodec, Codec *outCodec, bool isOneway)
    : m_sequence(sequence)
    , m_in(inCodec)
    , m_out(outCodec)
    , m_oneway(isOneway)
    {
    }

    /*!
     * @brief Get in codec (for reading).
     *
     * @return In codec.
     */
    Codec *getInCodec() { return m_in; }
    /*!
     * @brief Get out codec (for writing).
     *
     * @return Out codec.
     */
    Codec *getOutCodec() { return m_out; }
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
    Codec *m_in;         //!< In codec. Codec for receiving data.
    Codec *m_out;        //!< Out codec. Codec for sending data.
    bool m_oneway;       //!< When true, request context will be oneway type (only send data).
};

} // namespace erpc

/*! @} */

#endif // _EMBEDDED_RPC__CLIENT_MANAGER_H_
