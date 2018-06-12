/*
 * The Clear BSD License
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
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

#ifndef __embedded_rpc__thread__
#define __embedded_rpc__thread__

#include "erpc_config_internal.h"
#include <stdint.h>

// Exclude the rest of the file if threading is disabled.
#if ERPC_THREADS

#if ERPC_THREADS_IS(PTHREADS)
#include <pthread.h>
#elif ERPC_THREADS_IS(FREERTOS)
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#endif // ERPC_THREADS_IS

/*!
 * @addtogroup port_threads
 * @{
 * @file
 */

////////////////////////////////////////////////////////////////////////////////
// Types
////////////////////////////////////////////////////////////////////////////////

//! @brief Thread function type.
//!
//! @param arg User provided argument that was passed into the start() method.
typedef void (*thread_entry_t)(void *arg);

////////////////////////////////////////////////////////////////////////////////
// Declarations
////////////////////////////////////////////////////////////////////////////////

#if defined(__cplusplus)

namespace erpc {
/*!
 * @brief Simple thread class.
 *
 * @ingroup port_threads
 */
class Thread
{
public:
    //! @brief Unique identifier for a thread.
    typedef void *thread_id_t;

    /*!
     * @brief Default constructor for use with the init() method.
     *
     * If this constructor is used, the init() method must be called before the thread can be
     * started.
     *
     * @param name Optional name for the thread.
     */
    Thread(const char *name = 0);

    /*!
     * @brief Constructor.
     *
     * This constructor fully initializes the thread object.
     *
     * @param entry
     * @param priority
     * @param stackSize
     * @param name Optional name for the thread.
     */
    Thread(thread_entry_t entry, uint32_t priority = 0, uint32_t stackSize = 0, const char *name = 0);
    virtual ~Thread();

    void setName(const char *name) { m_name = name; }
    const char *getName() const { return m_name; }
    void init(thread_entry_t entry, uint32_t priority = 0, uint32_t stackSize = 0);

    void start(void *arg = 0);

    static void sleep(uint32_t usecs);

    thread_id_t getThreadId() const
    {
#if ERPC_THREADS_IS(PTHREADS)
        return reinterpret_cast<thread_id_t>(m_thread);
#elif ERPC_THREADS_IS(FREERTOS)
        return reinterpret_cast<thread_id_t>(m_task);
#endif
    }

    static thread_id_t getCurrentThreadId()
    {
#if ERPC_THREADS_IS(PTHREADS)
        return reinterpret_cast<thread_id_t>(pthread_self());
#elif ERPC_THREADS_IS(FREERTOS)
        return reinterpret_cast<thread_id_t>(xTaskGetCurrentTaskHandle());
#endif
    }

    static Thread *getCurrentThread();

    bool operator==(Thread &o);

protected:
    virtual void threadEntryPoint();

private:
    const char *m_name;
    thread_entry_t m_entry;
    void *m_arg;
    uint32_t m_stackSize;
    uint32_t m_priority;
#if ERPC_THREADS_IS(PTHREADS)
    static pthread_key_t s_threadObjectKey;
    pthread_t m_thread;
#elif ERPC_THREADS_IS(FREERTOS)
    TaskHandle_t m_task;
    Thread *m_next;
    static Thread *s_first;
#endif

#if ERPC_THREADS_IS(PTHREADS)
    static void *threadEntryPointStub(void *arg);
#elif ERPC_THREADS_IS(FREERTOS)
    static void threadEntryPointStub(void *arg);
#endif

private:
    Thread(const Thread &o);
    Thread &operator=(const Thread &o);
};

/*!
 * @brief Mutex.
 *
 * If the OS supports it, the mutex will be recursive.
 *
 * @ingroup port_threads
 */
class Mutex
{
public:
    /*!
     * @brief
     */
    class Guard
    {
    public:
        Guard(Mutex &mutex)
        : m_mutex(mutex)
        {
            m_mutex.lock();
        }
        ~Guard() { m_mutex.unlock(); }

    private:
        Mutex &m_mutex;
    };

    Mutex();
    ~Mutex();

    bool tryLock();
    bool lock();
    bool unlock();

#if ERPC_THREADS_IS(PTHREADS)
    pthread_mutex_t *getPtr()
    {
        return &m_mutex;
    }
#endif

private:
#if ERPC_THREADS_IS(PTHREADS)
    pthread_mutex_t m_mutex;
#elif ERPC_THREADS_IS(FREERTOS)
    SemaphoreHandle_t m_mutex;
#endif

private:
    Mutex(const Mutex &o);
    Mutex &operator=(const Mutex &o);
};

/*!
 * @brief Simple semaphore class.
 *
 * @ingroup port_threads
 */
class Semaphore
{
public:
    static const uint32_t kWaitForever = 0xffffffff;

    Semaphore(int count = 0);
    ~Semaphore();

    void put();
    void putFromISR();
    bool get(uint32_t timeout = kWaitForever);
    int getCount() const;

private:
#if ERPC_THREADS_IS(PTHREADS)
    int m_count;
    pthread_cond_t m_cond;
    Mutex m_mutex;
#elif ERPC_THREADS_IS(FREERTOS)
    SemaphoreHandle_t m_sem;
#endif // ERPC_HAS_PTHREADS

private:
    Semaphore(const Semaphore &o);
    Semaphore &operator=(const Semaphore &o);
};

} // namespace erpc

#endif // defined(__cplusplus)

/*! @} */

#endif // ERPC_THREADS

#endif // defined(__embedded_rpc__thread__)
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
