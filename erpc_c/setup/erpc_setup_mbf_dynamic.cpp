/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_config_internal.h"
#include "erpc_manually_constructed.hpp"
#include "erpc_mbf_setup.h"
#include "erpc_message_buffer.hpp"

#include <new>

using namespace std;
using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

/*!
 * @brief Dynamic message buffer factory.
 */
class DynamicMessageBufferFactory : public MessageBufferFactory
{
public:
    virtual MessageBuffer create(void)
    {
        uint8_t *buf = new (nothrow) uint8_t[ERPC_DEFAULT_BUFFER_SIZE];
        return MessageBuffer(buf, ERPC_DEFAULT_BUFFER_SIZE);
    }

    virtual void dispose(MessageBuffer *buf)
    {
        erpc_assert(buf != NULL);
        if (buf->get() != NULL)
        {
            delete[] buf->get();
        }
    }
};

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

ERPC_MANUALLY_CONSTRUCTED_STATIC(DynamicMessageBufferFactory, s_msgFactory);

erpc_mbf_t erpc_mbf_dynamic_init(void)
{
    DynamicMessageBufferFactory *msgFactory;

#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_STATIC
    if (s_msgFactory.isUsed())
    {
        msgFactory = NULL;
    }
    else
    {
        s_msgFactory.construct();
        msgFactory = s_msgFactory.get();
    }
#elif ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_DYNAMIC
    msgFactory = new DynamicMessageBufferFactory();
#else
#error "Unknown eRPC allocation policy!"
#endif

    return reinterpret_cast<erpc_mbf_t>(msgFactory);
}

void erpc_mbf_dynamic_deinit(erpc_mbf_t mbf)
{
#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_STATIC
    (void)mbf;
    s_msgFactory.destroy();
#elif ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_DYNAMIC
    erpc_assert(mbf != NULL);

    DynamicMessageBufferFactory *msgFactory = reinterpret_cast<DynamicMessageBufferFactory *>(mbf);

    delete msgFactory;
#endif
}
