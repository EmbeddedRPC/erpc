/*
 * Copyright 2017-2021 NXP
 * Copyright 2021 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "erpc_config_internal.h"
#if ERPC_MESSAGE_LOGGING

#include "erpc_message_loggers.h"

#include <new>

using namespace erpc;
using namespace std;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

MessageLoggers::~MessageLoggers(void)
{
    MessageLogger *logger;

    while (m_logger != NULL)
    {
        logger = m_logger;
        m_logger = m_logger->getNext();
        delete logger;
    }
}

bool MessageLoggers::addMessageLogger(Transport *transport)
{
    bool retVal = false;
    MessageLogger *logger;
    MessageLogger *_logger;

    if (transport != NULL)
    {
        logger = new (nothrow) MessageLogger(transport);
        if (logger != NULL)
        {
            if (m_logger == NULL)
            {
                m_logger = logger;
            }
            else
            {
                _logger = m_logger;
                while (_logger->getNext() != NULL)
                {
                    _logger = _logger->getNext();
                }

                _logger->setNext(logger);
            }
            retVal = true;
        }
    }

    return retVal;
}

erpc_status_t MessageLoggers::logMessage(MessageBuffer *msg)
{
    erpc_status_t err = kErpcStatus_Success;
    MessageLogger *_logger = m_logger;

    while (_logger != NULL)
    {
        err = _logger->getLogger()->send(msg);
        if (err == kErpcStatus_Success)
        {
            _logger = _logger->getNext();
        }
        else
        {
            break;
        }
    }

    return err;
}
#endif /* ERPC_MESSAGE_LOGGING */
