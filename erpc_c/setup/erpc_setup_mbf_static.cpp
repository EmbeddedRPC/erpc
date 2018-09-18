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

#if !ERPC_THREADS_IS(ERPC_THREADS_NONE)
#include "erpc_threading.h"
#endif

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

/*!
 * @brief Static Message buffer factory
 */
class StaticMessageBufferFactory : public MessageBufferFactory
{
public:
    /*!
     * @brief Constructor.
     */
    StaticMessageBufferFactory(void)
#if !ERPC_THREADS_IS(ERPC_THREADS_NONE)
    : m_semaphore(1)
#endif
    {
        uint32_t i;
        for (i = 0; i <= (ERPC_DEFAULT_BUFFERS_COUNT >> 3); i++)
        {
            m_freeBufferBitmap[i] = 0xff;
        }
    }

    /*!
     * @brief CodecFactory destructor
     */
    virtual ~StaticMessageBufferFactory(void) {}

    /*!
     * @brief This function creates new message buffer.
     *
     * @return MessageBuffer New created MessageBuffer.
     */
    virtual MessageBuffer create(void)
    {
        uint8_t idx = 0;
#if !ERPC_THREADS_IS(ERPC_THREADS_NONE)
        m_semaphore.get();
#endif
        while (((m_freeBufferBitmap[idx >> 3] & (1 << (idx & 0x7))) == 0) && (idx < ERPC_DEFAULT_BUFFERS_COUNT))
        {
            idx++;
        }

        assert(idx < ERPC_DEFAULT_BUFFERS_COUNT);

        m_freeBufferBitmap[idx >> 3] &= ~(1 << (idx & 0x7));
#if !ERPC_THREADS_IS(ERPC_THREADS_NONE)
        m_semaphore.put();
#endif

        uint8_t *buf;
        buf = (uint8_t *)m_buffers[idx];

        assert(NULL != buf);
        return MessageBuffer(buf, ERPC_DEFAULT_BUFFER_SIZE);
    }

    /*!
     * @brief This function disposes message buffer.
     *
     * @param[in] buf MessageBuffer to dispose.
     */
    virtual void dispose(MessageBuffer *buf)
    {
        assert(buf);
        uint8_t *tmp = buf->get();
        if (tmp)
        {
            uint8_t idx = 0;
#if !ERPC_THREADS_IS(ERPC_THREADS_NONE)
            m_semaphore.get();
#endif
            while ((tmp != (uint8_t *)m_buffers[idx]) && (idx < ERPC_DEFAULT_BUFFERS_COUNT))
            {
                idx++;
            }
            if (idx < ERPC_DEFAULT_BUFFERS_COUNT)
            {
                m_freeBufferBitmap[idx >> 3] |= 1 << (idx & 0x7);
            }
#if !ERPC_THREADS_IS(ERPC_THREADS_NONE)
            m_semaphore.put();
#endif
        }
    }

protected:
    uint8_t m_freeBufferBitmap[(ERPC_DEFAULT_BUFFERS_COUNT >> 3) + 1]; /*!< Bitmat of used/not used buffers. */
    uint64_t m_buffers[ERPC_DEFAULT_BUFFERS_COUNT]
                      [(ERPC_DEFAULT_BUFFER_SIZE + sizeof(uint64_t) - 1) / sizeof(uint64_t)]; /*!< Static buffers. */
#if !ERPC_THREADS_IS(ERPC_THREADS_NONE)
    Semaphore m_semaphore; /*!< Semaphore.*/
#endif
};

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

static ManuallyConstructed<StaticMessageBufferFactory> s_msgFactory;

erpc_mbf_t erpc_mbf_static_init(void)
{
    s_msgFactory.construct();
    return reinterpret_cast<erpc_mbf_t>(s_msgFactory.get());
}
