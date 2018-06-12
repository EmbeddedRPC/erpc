/*
 * The Clear BSD License
 * Copyright (c) 2015-2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted (subject to the limitations in the disclaimer below) provided
 *  that the following conditions are met:
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

#ifndef __embedded_rpc__static_queue__
#define __embedded_rpc__static_queue__

#include <cstring>

#if !(__embedded_cplusplus)
using namespace std;
#endif

/*!
 * @addtogroup infra_utility
 * @{
 * @file
 */

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpc {
/*!
 * @brief Template class which implements static queue as ring buffer
 *
 * @ingroup infra_utility
 */
template <class T, uint32_t elementCount>
class StaticQueue
{
public:
    /*!
     * @brief Constructor of StaticQueue class.
     *
     * This function sets capacity of queue, m_head and m_tail indexes to zero.
     */
    StaticQueue()
    {
        m_capacity = elementCount;
        m_head = 0;
        m_tail = 0;
    }

    /*!
     * @brief Destructor of StaticQueue class.
     *
     * This function free allocated buffer for m_data.
     */
    ~StaticQueue() {}
    /*!
     * @brief This function adds element to queue.
     *
     * @param[in] element Element for adding.
     *
     * @retval true Element was added.
     * @retval false Element was not added, queue is full.
     */
    bool add(T element)
    {
        if ((m_head + 1) % m_capacity != m_tail)
        {
            memcpy(m_storage[m_head], &element, sizeof(T));
            m_head = (m_head + 1) % m_capacity;
            return true;
        }
        return false;
    }

    /*!
     * @brief This function returns element from queue.
     *
     * @param[out] element Pointer to element to which will be copied element from queue.
     *
     * @retval true Element was copied from queue.
     * @retval false Element was not copied, queue is empty.
     */
    bool get(T *element)
    {
        if (m_tail != m_head)
        {
            memcpy(element, m_storage[m_tail], sizeof(T));
            m_tail = (m_tail + 1) % m_capacity;
            return true;
        }
        return false;
    }

    /*!
    * @brief This function returns number of elements in queue.
    *
    * @return Number of elements in queue.
    */
    int size()
    {
        if (m_head >= m_tail)
        {
            return m_head - m_tail;
        }
        return (m_capacity - m_tail + m_head);
    }

protected:
    uint64_t m_storage[elementCount][(sizeof(T) + sizeof(uint64_t) - 1) /
                                     sizeof(uint64_t)]; /*!< Preallocated space based on data type size and elements
                                                           count. */
    uint32_t m_capacity;                                /*!< Capacity of queue */
    uint32_t volatile m_head;                           /*!< Index to free slot */
    uint32_t volatile m_tail;                           /*!< Index to slot with m_data */
};

} // namespace erpc

/*! @} */

#endif // defined(__embedded_rpc__static_queue__)
