/*
 * The Clear BSD License
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
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

#ifndef _EMBEDDED_RPC__MANUALLY_CONSTRUCTED_H_
#define _EMBEDDED_RPC__MANUALLY_CONSTRUCTED_H_
#include <new>
#include <stdint.h>

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
 * @brief Allocates static storage for an object.
 *
 * This template class defines storage of the template class' size. Then it
 * gives the user explicit control over construction and destruction of the
 * object residing in that storage. This is useful for placing objects in
 * unions, or other situations where you need static storage but want to delay
 * the actual construction of the object.
 *
 * Objects of this class will act as a pointer to the template argument class
 * type. Of course, if the object has not yet been constructed then the
 * behavior is undefined if the pointer to it is used.
 *
 * Note that there is not a constructor or deconstructor. This is because
 * members of unions cannot have constructors or deconstructors.
 *
 * Instances of this template class are aligned to 8 bytes.
 *
 * @ingroup infra_utility
 */
template <class T>
class ManuallyConstructed
{
public:
    //! @name Object access
    //@{
    T *get() { return reinterpret_cast<T *>(&m_storage); }
    const T *get() const { return reinterpret_cast<const T *>(&m_storage); }
    T *operator->() { return get(); }
    const T *operator->() const { return get(); }
    T &operator*() { return *get(); }
    const T &operator*() const { return *get(); }
    operator T *() { return get(); }
    operator const T *() const { return get(); }
    //@}

    //! @name Explicit construction methods
    //@{
    void construct() { new (m_storage) T; }
    template <typename A1>
    void construct(const A1 &a1)
    {
        new (m_storage) T(a1);
    }

    template <typename A1, typename A2>
    void construct(const A1 &a1, const A2 &a2)
    {
        new (m_storage) T(a1, a2);
    }

    template <typename A1, typename A2, typename A3>
    void construct(const A1 &a1, const A2 &a2, const A3 &a3)
    {
        new (m_storage) T(a1, a2, a3);
    }

    template <typename A1, typename A2, typename A3, typename A4>
    void construct(const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4)
    {
        new (m_storage) T(a1, a2, a3, a4);
    }

    template <typename A1, typename A2, typename A3, typename A4, typename A5>
    void construct(const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5)
    {
        new (m_storage) T(a1, a2, a3, a4, a5);
    }
    //@}

    /*!
     * @brief Invoke the object's destructor.
     *
     * Behavior is undefined if the objected was not previously initialized.
     */
    void destroy() { get()->~T(); }

protected:
    /*!
     * @brief Storage for the object.
     *
     * An array of uint64 is used to get 8-byte alignment.
     */
    uint64_t m_storage[(sizeof(T) + sizeof(uint64_t) - 1) / sizeof(uint64_t)];
};

} // namespace erpc

/*! @} */

#endif // _EMBEDDED_RPC__MANUALLY_CONSTRUCTED_H_
