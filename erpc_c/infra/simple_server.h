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

#ifndef _EMBEDDED_RPC__SIMPLE_SERVER_H_
#define _EMBEDDED_RPC__SIMPLE_SERVER_H_

#include "server.h"

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
 * @brief Based server implementation.
 *
 * @ingroup infra_server
 */
class SimpleServer : public Server
{
public:
    /*!
     * @brief Constructor.
     *
     * This function initializes object attributes.
     */
    SimpleServer()
    : m_transport()
    , m_isServerOn(true)
    {
    }
    /*!
     * @brief SimpleServer destructor
     */
    virtual ~SimpleServer();

    /*!
     * @brief This function sets transport layer to use.
     *
     * @param[in] transport Transport layer to use.
     */
    void setTransport(Transport *transport) { m_transport = transport; }
    /*!
     * @brief Run server in infinite loop.
     *
     * Will never jump out from this function.
     */
    virtual erpc_status_t run();

    /*!
     * @brief Run server implementation only if exist message to process.
     *
     * If is message to process, server process it and jumps out from this function,
     * useful for bare-metal because doesn't block main loop, when are not messages
     * to process.
     *
     * @return Return true when server is ON, else false.
     */
    virtual erpc_status_t poll();

    /*!
     * @brief This function sets server from ON to OFF
     */
    virtual void stop();

protected:
    /*!
     * @brief Run server implementation.
     *
     * This function call functions for receiving data, process this data and
     * if reply exist, send it back.
     */
    erpc_status_t runInternal();

    /*!
     * @brief Disposing message buffers and codecs.
     *
     * @param[in] codec Pointer to codec to dispose. It contains also message buffer to dispose.
     */
    void disposeBufferAndCodec(Codec *codec);

    /*!
     * @brief Create message buffers and codecs.
     *
     * @return Pointer to created codec with message buffer.
     */
    Codec *createBufferAndCodec();

    Transport *m_transport; /*!< Transport layer used to send and receive data. */
    bool m_isServerOn;      /*!< Information if server is ON or OFF. */
};

} // namespace erpc

/*! @} */

#endif // _EMBEDDED_RPC__SIMPLE_SERVER_H_
