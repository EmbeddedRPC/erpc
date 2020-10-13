/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_message_loggers.h"

#include <new>

using namespace erpc;
using namespace std;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

MessageLoggers::~MessageLoggers(void)
{
    while (m_logger != NULL)
    {
        MessageLogger *logger = m_logger;
        m_logger = m_logger->getNext();
        delete logger;
    }
}

bool MessageLoggers::addMessageLogger(Transport *transport)
{
    if (transport != NULL)
    {
        MessageLogger *logger = new (nothrow) MessageLogger(transport);
        if (logger)
        {
            if (m_logger == NULL)
            {
                m_logger = logger;
                return true;
            }

            MessageLogger *_logger = m_logger;
            while (_logger->getNext() != NULL)
            {
                _logger = _logger->getNext();
            }

            _logger->setNext(logger);
            return true;
        }
    }
    return false;
}

erpc_status_t MessageLoggers::logMessage(MessageBuffer *msg)
{
    MessageLogger *_logger = m_logger;
    while (_logger != NULL)
    {
        if (erpc_status_t err = _logger->getLogger()->send(msg))
        {
            return err;
        }
        _logger = _logger->getNext();
    }
    return kErpcStatus_Success;
}
