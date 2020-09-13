/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
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
    T *get(void) { return reinterpret_cast<T *>(&m_storage); }
    const T *get(void) const { return reinterpret_cast<const T *>(&m_storage); }
    T *operator->(void) { return get(); }
    const T *operator->(void)const { return get(); }
    T &operator*(void) { return *get(); }
    const T &operator*(void)const { return *get(); }
    operator T *(void) { return get(); }
    operator const T *(void)const { return get(); }
    //@}

    //! @name Explicit construction methods
    //@{
    void construct(void) { new (m_storage) T; }
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
	
    template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
    void construct(const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6)
    {
        new (m_storage) T(a1, a2, a3, a4, a5, a6);
    }
    //@}

    /*!
     * @brief Invoke the object's destructor.
     *
     * Behavior is undefined if the objected was not previously initialized.
     */
    void destroy(void) { get()->~T(); }

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
