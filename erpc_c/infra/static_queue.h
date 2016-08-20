/*
 * Copyright (c) 2015-2016, Freescale Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
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

namespace erpc
{
/*!
 * @brief Base class which implements static queue as ring buffer that operates on m_data type void*.
 *
 * @ingroup infra_utility
 */
class BaseStaticQueue
{
protected:
    uint32_t m_capacity;      //!< m_capacity of queue
    void *volatile m_data;    //!< Pointer to m_data of queue
    uint32_t volatile m_head; //!< Index to free slot
    uint32_t volatile m_tail; //!< Index to slot with m_data
    uint32_t m_elementSize;   //!< Size of one element

    /*!
     * @brief Constructor of BaseStaticQueue class.
     *
     * This function sets m_capacity of queue, size of one element in bytes, m_head and m_tail indexes to zero.
     *
     * @param[in] max_capacity m_capacity of queue.
     * @param[in] elementSize Size of one element in bytes.
     * @param[in] buffer Queue preallocated buffer.
     */
    BaseStaticQueue(uint32_t max_capacity, uint32_t elementSize, void *buffer)
    {
        m_data = buffer;
        m_elementSize = elementSize;
        m_capacity = max_capacity;
        m_head = 0;
        m_tail = 0;
    }

    /*!
     * @brief This function adds element to queue.
     *
     * @param[in] element Pointer to element for adding.
     *
     * @retval true Element was added.
     * @retval false Element was not added, queue is full.
     */
    bool add(void *element)
    {
        if ((m_head + 1) % m_capacity != m_tail)
        {
            memcpy((void *)((uint32_t)m_data + m_head * m_elementSize), element, m_elementSize);
            m_head = (m_head + 1) % m_capacity;
            return true;
        }
        return false;
    }

    /*!
     * @brief This function returns pointer to element from queue.
     *
     * @return Pointer to element.
     */
    void *get()
    {
        void *element = NULL;
        if (m_tail != m_head)
        {
            element = (void *)((uint32_t)m_data + m_tail * m_elementSize);
            m_tail = (m_tail + 1) % m_capacity;
        }
        return element;
    }

public:
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
};

/*!
 * @brief Template subclass that wraps the member functions to use the template type parameter.
 *
 * @ingroup infra_utility
 */
template <class T, uint32_t elementCount>
class StaticQueue : public BaseStaticQueue
{
public:
    /*!
     * @brief Constructor of StaticQueue class.
     */
    StaticQueue()
    : BaseStaticQueue(elementCount, sizeof(T), m_storage)
    {
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
    bool add(T element) { return BaseStaticQueue::add(&element); }
    /*!
     * @brief This function returns element from queue.
     *
     * @return Element.
     */
    T *get()
    {
        void *element = BaseStaticQueue::get();
        if (element == NULL)
        {
            return NULL;
        }
        return (T *)element;
    }

protected:
    uint64_t m_storage[elementCount][(sizeof(T) + sizeof(uint64_t) - 1) /
                                     sizeof(uint64_t)]; /*!< Preallocated space based on data type size and elements
                                                           count. */
};

} // namespace erpc

/*! @} */

#endif // defined(__embedded_rpc__static_queue__)
