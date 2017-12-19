/*
 * Copyright 2017 NXP
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

#ifndef _EMBEDDED_RPC__LOGEVENTLISTENERS_H_
#define _EMBEDDED_RPC__LOGEVENTLISTENERS_H_

#include "transport.h"

/*!
 * @addtogroup infra_transport
 * @{
 * @file
 */

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

/*!
 * @brief Logging messages objects used for logging sent/received messages.
 *
 * @ingroup infra_utility
 */
namespace erpc {

class MessageLogger
{
public:
    /*!
     * @brief MessageLogger constructor.
     */
    MessageLogger(Transport *transport)
    : m_next(NULL)
    , m_transport(transport){};

    /*!
     * @brief MessageLogger destructor
     */
    ~MessageLogger(){};

    /*!
     * @brief Return next logger.
     *
     * @return Pointer to next logger.
     */
    MessageLogger *getNext() { return m_next; }

    /*!
     * @brief Set next logger.
     *
     * @param[in] next Pointer to next logger.
     */
    void setNext(MessageLogger *next) { m_next = next; }

    /*!
     * @brief Returns next transport object used for logging messages.
     *
     * @return Transport object used for logging messages.
     */
    Transport *getLogger() { return m_transport; }

protected:
    MessageLogger *m_next;  /*!< Pointer to next logger. */
    Transport *m_transport; /*!< Pointer to transport object used for logging messages. */
};

/*!
 * @brief Logging messages functionality.
 *
 * @ingroup infra_utility
 */
class MessageLoggers
{
public:
    /*!
     * @brief Constructor.
     */
    MessageLoggers()
    : m_logger(NULL){};

    /*!
     * @brief Transport destructor
     */
    virtual ~MessageLoggers();

    /*!
     * @brief This function add given transport to newly created MessageLogger object.
     *
     * @param[in] transport Transport used for logging messages
     */
    void addMessageLogger(Transport *transport);

protected:
    MessageLogger *m_logger; /*!< Pointer to first logger. */

    /*!
     * @brief This function add given transport to newly created MessageLogger object.
     *
     * @param[in] MessageBuffer Buffer which will be logged.
     *
     * @return The eRPC status based on transport type.
     */
    erpc_status_t logMessage(MessageBuffer *msg);
};

} // namespace erpc

/*! @} */

#endif // _EMBEDDED_RPC__LOGEVENTLISTENERS_H_
