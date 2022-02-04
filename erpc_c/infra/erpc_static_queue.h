/*
 * Copyright (c) 2015-2016, Freescale Semiconductor, Inc.
 * Copyright 2021 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __embedded_rpc__static_queue__
#define __embedded_rpc__static_queue__

#include <cstring>

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
    StaticQueue(void)
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
    ~StaticQueue(void) {}
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
        if ((m_head + 1U) % m_capacity != m_tail)
        {
            (void)std::memcpy(m_storage[m_head], &element, sizeof(T));
            m_head = (m_head + 1U) % m_capacity;
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
            (void)std::memcpy(element, m_storage[m_tail], sizeof(T));
            m_tail = (m_tail + 1U) % m_capacity;
            return true;
        }
        return false;
    }

    /*!
     * @brief This function returns number of elements in queue.
     *
     * @return Number of elements in queue.
     */
    uint32_t size(void)
    {
        if (m_head >= m_tail)
        {
            return m_head - m_tail;
        }
        return (m_capacity - m_tail + m_head);
    }

protected:
    uint64_t m_storage[elementCount]
                      [(sizeof(T) + sizeof(uint64_t) - 1U) / sizeof(uint64_t)]; /*!< Preallocated space based on data
                                                                                  type size and elements count. */
    uint32_t m_capacity;                                                       /*!< Capacity of queue */
    uint32_t volatile m_head;                                                  /*!< Index to free slot */
    uint32_t volatile m_tail;                                                  /*!< Index to slot with m_data */
};

} // namespace erpc

/*! @} */

#endif // defined(__embedded_rpc__static_queue__)
