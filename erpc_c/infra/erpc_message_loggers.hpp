/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__LOGEVENTLISTENERS_H_
#define _EMBEDDED_RPC__LOGEVENTLISTENERS_H_

#include "erpc_transport.hpp"

/*!
 * @addtogroup infra_transport
 * @{
 * @file
 */

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpc {

/*!
 * @brief Logging messages objects used for logging sent/received messages.
 *
 * @ingroup infra_utility
 */
class MessageLogger
{
public:
    /*!
     * @brief MessageLogger constructor.
     */
    explicit MessageLogger(Transport *transport) : m_next(NULL), m_transport(transport){};

    /*!
     * @brief MessageLogger destructor
     */
    ~MessageLogger(void){};

    /*!
     * @brief Return next logger.
     *
     * @return Pointer to next logger.
     */
    MessageLogger *getNext(void) { return m_next; }

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
    Transport *getLogger(void) { return m_transport; }

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
    MessageLoggers(void) : m_logger(NULL){};

    /*!
     * @brief Transport destructor
     */
    virtual ~MessageLoggers(void);

    /*!
     * @brief This function add given transport to newly created MessageLogger object.
     *
     * @param[in] transport Transport used for logging messages
     *
     * @retval True When transport was successfully added.
     * @retval False When transport wasn't added.
     */
    bool addMessageLogger(Transport *transport);

protected:
    MessageLogger *m_logger; /*!< Pointer to first logger. */

    /*!
     * @brief This function add given transport to newly created MessageLogger object.
     *
     * @param[in] msg Buffer which will be logged.
     *
     * @return The eRPC status based on transport type.
     */
    erpc_status_t logMessage(MessageBuffer *msg);

private:
    /**
     * @brief This function create new MessageLogger object
     *
     * @param[in] transport Transport used by MessageLogger.
     *
     * @return MessageLogger* Returns new MessageLogger object.
     */
    MessageLogger *create(Transport *transport);
};

} // namespace erpc

/*! @} */

#endif // _EMBEDDED_RPC__LOGEVENTLISTENERS_H_
