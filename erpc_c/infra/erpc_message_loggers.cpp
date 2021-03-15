/*
 * Copyright 2017 NXP
 * Copyright 2021 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_message_loggers.h"

#include "erpc_manually_constructed.h"

#if ERPC_ALLOCATION_POLICY == ERPC_DYNAMIC_POLICY
#include <new>
#endif

using namespace erpc;
using namespace std;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

ERPC_MANUALLY_CONSTRUCTED_ARRAY_STATIC(MessageLogger, s_messageLoggersManual, ERPC_MESSAGE_LOGGERS_COUNT);

MessageLoggers::~MessageLoggers(void)
{
    MessageLogger *logger;

    while (m_logger != NULL)
    {
        logger = m_logger;
        m_logger = m_logger->getNext();
        ERPC_DESTROY_OBJECT(logger, s_messageLoggersManual, ERPC_MESSAGE_LOGGERS_COUNT)
    }
}

bool MessageLoggers::addMessageLogger(Transport *transport)
{
    bool retVal = false;
    MessageLogger *logger;
    MessageLogger *_logger;

    if (transport != NULL)
    {
        logger = create(transport);
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

MessageLogger *MessageLoggers::create(Transport *transport)
{
    ERPC_CREATE_NEW_OBJECT(MessageLogger, s_messageLoggersManual, ERPC_MESSAGE_LOGGERS_COUNT, transport)
}
