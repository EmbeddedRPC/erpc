/*
 * The Clear BSD License
 * Copyright (c) 2013-2014, Freescale Semiconductor, Inc.
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
#if !defined(_smart_ptr_h_)
#define _smart_ptr_h_

#include <cstdlib>

//! @brief Delete policy for regular objects.
template <typename T>
struct smart_ptr_delete
{
    static inline void del(T *v) { delete v; }
};

//! @brief Delete policy using free() to delete objects.
template <typename T>
struct smart_ptr_free
{
    static inline void del(T *v) { free(v); }
};

//! @brief Delete policy for arrays.
template <typename T>
struct smart_ptr_delete_array
{
    static inline void del(T *v) { delete[] v; }
};

/*!
 * \brief Simple, standard smart pointer class.
 *
 * This class only supports the single-owner paradigm.
 */
template <typename T, class delete_policy = smart_ptr_delete<T> >
class smart_ptr
{
public:
    typedef T data_type;
    typedef T *ptr_type;
    typedef const T *const_ptr_type;
    typedef T &ref_type;
    typedef const T &const_ref_type;

    //! Default constructor. Initializes with no pointer set.
    smart_ptr()
    : _p(nullptr)
    {
    }

    //! This constructor takes a pointer to the object to be deleted.
    smart_ptr(ptr_type p)
    : _p(p)
    {
    }

    //! @brief Move copy constructor.
    smart_ptr(smart_ptr<T> &&other)
    : _p(other._p)
    {
        other._p = nullptr;
    }

    //! @brief Move assignment operator.
    smart_ptr<T> &operator=(smart_ptr<T> &&other)
    {
        set(other._p);
        other._p = nullptr;
        return *this;
    }

    //! Destructor. If an object (pointer) has been set, it will be deleted.
    //! Deletes the object using safe_delete().
    virtual ~smart_ptr() { safe_delete(); }

    //! Return the current pointer value.
    ptr_type get() { return _p; }

    //! Return the const form of the current pointer value.
    const_ptr_type get() const { return _p; }

    //! Change the pointer value, or set if if the default constructor was used.
    //! If a pointer had previously been associated with the object, and \a p is
    //! different than that previous pointer, it will be deleted before taking
    //! ownership of \a p. If this is not desired, call reset() beforehand.
    void set(ptr_type p)
    {
        if (_p && p != _p)
        {
            safe_delete();
        }
        _p = p;
    }

    //! Dissociates any previously set pointer value without deleting it.
    void reset() { _p = nullptr; }

    //! Dissociates a previously set pointer value, deleting it at the same time.
    void clear() { safe_delete(); }

    //! Forces immediate deletion of the object. If you are planning on using
    //! this method, think about just using a normal pointer. It probably makes
    //! more sense.
    virtual void safe_delete()
    {
        if (_p)
        {
            delete_policy::del(_p);
            _p = nullptr;
        }
    }

    //! \name Operators
    //@{

    //! Makes the object transparent as the template type.
    operator ptr_type() { return _p; }

    //! Const version of the pointer operator.
    operator const_ptr_type() const { return _p; }

    //! Makes the object transparent as a reference of the template type.
    operator ref_type() { return *_p; }

    //! Const version of the reference operator.
    operator const_ref_type() const { return *_p; }

    //! Returns a boolean indicating whether the object has a pointer set or not.
    operator bool() const { return _p != nullptr; }

    //! To allow setting the pointer directly. Equivalent to a call to set().
    smart_ptr<T> &operator=(ptr_type p)
    {
        set(p);
        return *this;
    }

    //! Another operator to allow you to treat the object just like a pointer.
    ptr_type operator->() { return _p; }

    //! Another operator to allow you to treat the object just like a pointer.
    const_ptr_type operator->() const { return _p; }
    //@}

protected:
    ptr_type _p; //!< The wrapped pointer.

    //! @brief Disable the copy constructor.
    smart_ptr(const smart_ptr<T> &) = delete;

    //! @brief Disable assignment operator.
    smart_ptr<T> &operator=(const smart_ptr<T> &) = delete;
};

//! @brief Version of smart_ptr that uses delete [].
template <typename T>
using smart_array_ptr = smart_ptr<T, smart_ptr_delete_array<T> >;

//! @brief Version of smart_ptr that uses free().
template <typename T>
using smart_free_ptr = smart_ptr<T, smart_ptr_free<T> >;

#endif // _smart_ptr_h_
