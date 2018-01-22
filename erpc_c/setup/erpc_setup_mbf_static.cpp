/*
 * The Clear BSD License
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted (subject to the limitations in the disclaimer below) provided
 * that the following conditions are met:
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
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE.
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
    : m_semaphore(0)
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
