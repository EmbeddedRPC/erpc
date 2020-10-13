/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_config_internal.h"
#include "erpc_manually_constructed.h"
#include "erpc_mbf_setup.h"
#include "erpc_message_buffer.h"

#include <assert.h>
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
    virtual MessageBuffer create()
    {
        uint8_t *buf = new (nothrow) uint8_t[ERPC_DEFAULT_BUFFER_SIZE];
        return MessageBuffer(buf, ERPC_DEFAULT_BUFFER_SIZE);
    }

    virtual void dispose(MessageBuffer *buf)
    {
        assert(buf);
        if (buf->get())
        {
            delete[] buf->get();
        }
    }
};

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

static ManuallyConstructed<DynamicMessageBufferFactory> s_msgFactory;

erpc_mbf_t erpc_mbf_dynamic_init(void)
{
    s_msgFactory.construct();
    return reinterpret_cast<erpc_mbf_t>(s_msgFactory.get());
}
