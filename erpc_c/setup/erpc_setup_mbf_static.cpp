/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 * Copyright 2021 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_config_internal.h"
#include "erpc_manually_constructed.hpp"
#include "erpc_mbf_setup.h"
#include "erpc_message_buffer.hpp"

#include <string.h>

#if !ERPC_THREADS_IS(NONE)
#include "erpc_threading.h"
#endif

using namespace erpc;

#define ERPC_BUFFER_SIZE_UINT64 ((ERPC_DEFAULT_BUFFER_SIZE + sizeof(uint64_t) - 1) / sizeof(uint64_t))

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
#if !ERPC_THREADS_IS(NONE)
    : m_semaphore(1)
#endif
    {
        (void)memset(m_freeBufferBitmap, 0xff, sizeof(m_freeBufferBitmap));
        (void)memset(m_buffers, 0, sizeof(m_buffers));
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
#if !ERPC_THREADS_IS(NONE)
        m_semaphore.get();
#endif
        while (((m_freeBufferBitmap[idx >> 3U] & (1U << (idx & 0x7U))) == 0U) && (idx < ERPC_DEFAULT_BUFFERS_COUNT))
        {
            idx++;
        }

        erpc_assert(idx < ERPC_DEFAULT_BUFFERS_COUNT);

        m_freeBufferBitmap[idx >> 3U] &= ~(1U << (idx & 0x7U));
#if !ERPC_THREADS_IS(NONE)
        m_semaphore.put();
#endif

        uint8_t *buf;
        buf = reinterpret_cast<uint8_t *>(m_buffers[idx]);

        erpc_assert(NULL != buf);
        return MessageBuffer(buf, ERPC_DEFAULT_BUFFER_SIZE);
    }

    /*!
     * @brief This function disposes message buffer.
     *
     * @param[in] buf MessageBuffer to dispose.
     */
    virtual void dispose(MessageBuffer *buf)
    {
        erpc_assert(buf != NULL);
        uint8_t *tmp = buf->get();
        if (tmp != NULL)
        {
            uint8_t idx = 0;
#if !ERPC_THREADS_IS(NONE)
            m_semaphore.get();
#endif
            while ((idx < ERPC_DEFAULT_BUFFERS_COUNT) && (tmp != reinterpret_cast<uint8_t *>(m_buffers[idx])))
            {
                idx++;
            }
            if (idx < ERPC_DEFAULT_BUFFERS_COUNT)
            {
                m_freeBufferBitmap[idx >> 3U] |= 1U << (idx & 0x7U);
            }
#if !ERPC_THREADS_IS(NONE)
            m_semaphore.put();
#endif
        }
    }

protected:
    //! Bitmap representing which buffers are in use. A bit value of 1 means free and 0 means in
    //! use.
    uint8_t m_freeBufferBitmap[(ERPC_DEFAULT_BUFFERS_COUNT >> 3U) + (ERPC_DEFAULT_BUFFERS_COUNT % 8 ? 1U : 0U)];
    //! Static buffers
    uint64_t m_buffers[ERPC_DEFAULT_BUFFERS_COUNT][ERPC_BUFFER_SIZE_UINT64];
#if !ERPC_THREADS_IS(NONE)
    Semaphore m_semaphore; /*!< Semaphore.*/
#endif
};

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

ERPC_MANUALLY_CONSTRUCTED_STATIC(StaticMessageBufferFactory, s_msgFactory);

erpc_mbf_t erpc_mbf_static_init(void)
{
    StaticMessageBufferFactory *msgFactory;

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
    msgFactory = new StaticMessageBufferFactory();
#else
#error "Unknown eRPC allocation policy!"
#endif

    return reinterpret_cast<erpc_mbf_t>(msgFactory);
}

void erpc_mbf_static_deinit(erpc_mbf_t mbf)
{
#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_STATIC
    (void)mbf;
    s_msgFactory.destroy();
#elif ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_DYNAMIC
    erpc_assert(mbf != NULL);

    StaticMessageBufferFactory *msgFactory = reinterpret_cast<StaticMessageBufferFactory *>(mbf);

    delete msgFactory;
#endif
}
