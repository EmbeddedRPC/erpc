/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * Copyright 2021 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__MANUALLY_CONSTRUCTED_H_
#define _EMBEDDED_RPC__MANUALLY_CONSTRUCTED_H_

#include "erpc_config_internal.h"

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
    T *get(void) { return (m_isConstructed) ? reinterpret_cast<T *>(&m_storage) : nullptr; }
    const T *get(void) const { return (m_isConstructed) ? reinterpret_cast<const T *>(&m_storage) : nullptr; }
    T *operator->(void) { return get(); }
    const T *operator->(void) const { return get(); }
    T &operator*(void)
    {
        if (m_isConstructed)
        {
            return *get();
        }
        else
        {
            memset(&m_storage, 0, sizeof(m_storage) * 8);
            return reinterpret_cast<T &>(m_storage);
        };
    }
    const T &operator*(void) const
    {
        if (m_isConstructed)
        {
            return *get();
        }
        else
        {
            memset(&m_storage, 0, sizeof(m_storage) * 8);
            return reinterpret_cast<const T &>(m_storage);
        };
    }
    operator T *(void) { return get(); }
    operator const T *(void) const { return get(); }
    //@}

    //! @name Explicit construction methods
    //@{
    void construct(void)
    {
        destroy();
        new (m_storage) T;
        m_isConstructed = true;
    }

    template <typename A1>
    void construct(const A1 &a1)
    {
        destroy();
        new (m_storage) T(a1);
        m_isConstructed = true;
    }

    template <typename A1, typename A2>
    void construct(const A1 &a1, const A2 &a2)
    {
        destroy();
        new (m_storage) T(a1, a2);
        m_isConstructed = true;
    }

    template <typename A1, typename A2, typename A3>
    void construct(const A1 &a1, const A2 &a2, const A3 &a3)
    {
        destroy();
        new (m_storage) T(a1, a2, a3);
        m_isConstructed = true;
    }

    template <typename A1, typename A2, typename A3, typename A4>
    void construct(const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4)
    {
        destroy();
        new (m_storage) T(a1, a2, a3, a4);
        m_isConstructed = true;
    }

    template <typename A1, typename A2, typename A3, typename A4, typename A5>
    void construct(const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5)
    {
        destroy();
        new (m_storage) T(a1, a2, a3, a4, a5);
        m_isConstructed = true;
    }

    template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
    void construct(const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6)
    {
        destroy();
        new (m_storage) T(a1, a2, a3, a4, a5, a6);
        m_isConstructed = true;
    }
    //@}

    /*!
     * @brief Invoke the object's destructor.
     *
     * Behavior is undefined if the objected was not previously initialized.
     */
    void destroy(void)
    {
        if (m_isConstructed)
        {
            get()->~T();
            m_isConstructed = false;
        }
    }

    /*!
     * @brief Returns information if object is free or is used.
     *
     * @return true Object is constructed and used.
     * @return false Object wasn't constructer or it is destructed and free.
     */
    bool isUsed(void) { return m_isConstructed; }

protected:
    /*!
     * @brief Storage for the object.
     *
     * An array of uint64 is used to get 8-byte alignment.
     */
    uint64_t m_storage[(sizeof(T) + sizeof(uint64_t) - 1) / sizeof(uint64_t)];

    /*!
     * @brief Track construct/destruct calls.
     *
     * Based on this variable we can allow or forbid construct/destruct calls.
     */
    bool m_isConstructed = false;
};

#define ERPC_MANUALLY_CONSTRUCTED(class, variableName) static ManuallyConstructed<class> variableName
#define ERPC_MANUALLY_CONSTRUCTED_ARRAY(class, variableName, dimension) \
    ERPC_MANUALLY_CONSTRUCTED(class, variableName)[dimension]

#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_STATIC
#define ERPC_MANUALLY_CONSTRUCTED_STATIC(class, variableName) ERPC_MANUALLY_CONSTRUCTED(class, variableName)
#else
#define ERPC_MANUALLY_CONSTRUCTED_STATIC(class, variableName)
#endif

#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_STATIC
#define ERPC_MANUALLY_CONSTRUCTED_ARRAY_STATIC(class, variableName, dimension) \
    ERPC_MANUALLY_CONSTRUCTED_ARRAY(class, variableName, dimension)
#else
#define ERPC_MANUALLY_CONSTRUCTED_ARRAY_STATIC(class, variableName, dimension)
#endif

#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_DYNAMIC
#define ERPC_CREATE_NEW_OBJECT(class, arrayOfObjects, numberOfObjects, ...) \
    return new (std::nothrow) class(__VA_ARGS__);

#define ERPC_DESTROY_OBJECT(object, ...) delete object;

#elif ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_STATIC
#define ERPC_CREATE_NEW_OBJECT(class, arrayOfObjects, numberOfObjects, ...)         \
    uint8_t objectsIterator;                                                        \
    class *ret = NULL;                                                              \
    for (objectsIterator = 0; objectsIterator < numberOfObjects; objectsIterator++) \
    {                                                                               \
        if (!arrayOfObjects[objectsIterator].isUsed())                              \
        {                                                                           \
            arrayOfObjects[objectsIterator].construct(__VA_ARGS__);                 \
            ret = arrayOfObjects[objectsIterator].get();                            \
            break;                                                                  \
        }                                                                           \
    }                                                                               \
    return ret;

#define ERPC_DESTROY_OBJECT(object, arrayOfObjects, numberOfObjects)                \
    uint8_t objectsIterator;                                                        \
    for (objectsIterator = 0; objectsIterator < numberOfObjects; objectsIterator++) \
    {                                                                               \
        if (object == arrayOfObjects[objectsIterator].get())                        \
        {                                                                           \
            arrayOfObjects[objectsIterator].destroy();                              \
            break;                                                                  \
        }                                                                           \
    }
#endif

} // namespace erpc

/*! @} */

#endif // _EMBEDDED_RPC__MANUALLY_CONSTRUCTED_H_
