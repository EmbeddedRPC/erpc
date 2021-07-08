/*
 * Copyright 2020 NXP
 * Copyright 2020 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__CLIENTSERVERCOMMON_H_
#define _EMBEDDED_RPC__CLIENTSERVERCOMMON_H_

#include "erpc_config_internal.h"
#if ERPC_MESSAGE_LOGGING
#include "erpc_message_loggers.h"
#endif
#if ERPC_PRE_POST_ACTION
#include "erpc_pre_post_action.h"
#endif

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
 * @brief Common class inheritand by client and server class.
 *
 * @ingroup infra_utility
 */
class ClientServerCommon
#if ERPC_MESSAGE_LOGGING
#ifdef ERPC_OTHER_INHERITANCE
    ,
#else
#define ERPC_OTHER_INHERITANCE 1
:
#endif
    public MessageLoggers
#endif
#if ERPC_PRE_POST_ACTION
#ifdef ERPC_OTHER_INHERITANCE
    ,
#else
#define ERPC_OTHER_INHERITANCE 1
:
#endif
    public PrePostAction
#endif
{
public:
    /*!
     * @brief ClientServerCommon constructor.
     */
    ClientServerCommon(void)
#ifdef ERPC_OTHER_INHERITANCE
#undef ERPC_OTHER_INHERITANCE
#endif
#if ERPC_MESSAGE_LOGGING
#ifdef ERPC_OTHER_INHERITANCE
        ,
#else
#define ERPC_OTHER_INHERITANCE 1
    :
#endif
        MessageLoggers()
#endif
#if ERPC_PRE_POST_ACTION
#ifdef ERPC_OTHER_INHERITANCE
            ,
#else
#define ERPC_OTHER_INHERITANCE 1
    :
#endif
        PrePostAction()
#endif
            {};

    /*!
     * @brief ClientServerCommon destructor
     */
    ~ClientServerCommon(void){};
};

} // namespace erpc

/*! @} */

#endif // _EMBEDDED_RPC__CLIENTSERVERCOMMON_H_
