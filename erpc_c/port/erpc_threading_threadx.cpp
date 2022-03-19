/*
 * Copyright (c) 2021, StarGate, Inc.
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * Ibrahim ERTURK <ierturk@ieee.org>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_threading.h"

#include <errno.h>

#if ERPC_THREADS_IS(THREADX)

#define TX_ERPC_APP_MEM_POOL_SIZE (4 * 1024)

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

Thread *Thread::s_first = NULL;

static UCHAR tx_byte_pool_buffer[TX_ERPC_APP_MEM_POOL_SIZE];
static TX_BYTE_POOL tx_app_byte_pool;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

Thread::Thread(const char *name)
: m_name(name)
, m_entry(0)
, m_arg(0)
, m_stackSize(0)
, m_priority(0)
, m_thread()
, m_next()
{
}

Thread::Thread(thread_entry_t entry, uint32_t priority, uint32_t stackSize, const char *name,
               thread_stack_pointer stackPtr)
: m_name(name)
, m_entry(entry)
, m_arg(0)
, m_stackSize(stackSize)
, m_priority(priority)
, m_thread()
, m_next()
{
}

Thread::~Thread(void)
{
    if (&m_thread != NULL)
    {
        tx_thread_delete(&m_thread);
        delete &m_thread;
    }
}

void Thread::init(thread_entry_t entry, uint32_t priority, uint32_t stackSize, thread_stack_pointer stackPtr)
{
    m_entry = entry;
    m_stackSize = stackSize;
    m_priority = priority;
    m_stackPtr = stackPtr;
    CHAR name[] = "Tx eRPC App memory pool";
    if (tx_byte_pool_create(&tx_app_byte_pool, name, tx_byte_pool_buffer, m_stackSize) != TX_SUCCESS)
    {
        // Fail
    }
    else
    {
        // Success
        UINT ret = TX_SUCCESS;
        CHAR *pointer;

        /* Allocate the stack for MainThread.  */
        if (tx_byte_allocate(&tx_app_byte_pool, (VOID **)&pointer, m_stackSize, TX_NO_WAIT) != TX_SUCCESS)
        {
            ret = TX_POOL_ERROR;
        }

        /* Create eRPC Thread.  */
        CHAR thread_name[] = "eRPC Thread";
        if (tx_thread_create(&m_thread, thread_name, threadEntryPointStub, (ULONG)m_entry, pointer, m_stackSize,
                             m_priority, m_priority, TX_NO_TIME_SLICE, TX_DONT_START) != TX_SUCCESS)
        {
            ret = TX_THREAD_ERROR;
        }

        (void)ret;
    }
}

void Thread::start(void *arg)
{
    m_arg = arg;

    // ENTER CRITICAL SECTION
    UINT my_old_posture;
    /* Lockout interrupts */
    my_old_posture = tx_interrupt_control(TX_INT_DISABLE);

    tx_thread_resume(&m_thread);

    // Link in this thread to the list.
    if (NULL != s_first)
    {
        m_next = s_first;
    }
    s_first = this;

    // EXIT CRITICAL SECTION
    /* Restore previous interrupt lockout posture. */
    tx_interrupt_control(my_old_posture);
}

bool Thread::operator==(Thread &o)
{
    return &m_thread == &(o.m_thread);
}

Thread *Thread::getCurrentThread(void)
{
    TX_THREAD *thisThread = tx_thread_identify();

    // Walk the threads list to find the Thread object for the current task.

    // ENTER CRITICAL SECTION
    UINT my_old_posture;
    /* Lockout interrupts */
    my_old_posture = tx_interrupt_control(TX_INT_DISABLE);

    Thread *it = s_first;
    while (it != NULL)
    {
        if (&(it->m_thread) == thisThread)
        {
            break;
        }
        it = it->m_next;
    }

    // EXIT CRITICAL SECTION
    /* Restore previous interrupt lockout posture. */
    tx_interrupt_control(my_old_posture);

    return it;
}

void Thread::sleep(uint32_t usecs)
{
    tx_thread_sleep(usecs / 1000);
}

void Thread::threadEntryPoint(void)
{
    if (m_entry != NULL)
    {
        m_entry(m_arg);
    }
}

void Thread::threadEntryPointStub(ULONG arg)
{
    Thread *_this = reinterpret_cast<Thread *>(arg);
    erpc_assert(_this && "Reinterpreting 'void *arg' to 'Thread *' failed.");
    _this->threadEntryPoint();

    // Remove this thread from the linked list.
    // ENTER CRITICAL SECTION
    UINT my_old_posture;
    /* Lockout interrupts */
    my_old_posture = tx_interrupt_control(TX_INT_DISABLE);

    Thread *it = s_first;
    Thread *prev = NULL;
    while (it != NULL)
    {
        if (it == _this)
        {
            if (it == s_first)
            {
                s_first = _this->m_next;
            }
            else
            {
                if (prev != NULL)
                {
                    prev->m_next = _this->m_next;
                }
            }
            _this->m_next = NULL;

            break;
        }
        prev = it;
        it = it->m_next;
    }

    // EXIT CRITICAL SECTION
    /* Restore previous interrupt lockout posture. */
    tx_interrupt_control(my_old_posture);
}

Mutex::Mutex(void)
: m_mutex()
{
    CHAR name[] = "erpc_mutex";
    tx_mutex_create(&m_mutex, name, TX_INHERIT);
}

Mutex::~Mutex(void)
{
    tx_mutex_delete(&m_mutex);
}

bool Mutex::tryLock(void)
{
    UINT status = tx_mutex_get(&m_mutex, TX_NO_WAIT);
    return (status == TX_SUCCESS);
}

bool Mutex::lock(void)
{
    UINT status = tx_mutex_get(&m_mutex, TX_WAIT_FOREVER);
    return (status == TX_SUCCESS);
}

bool Mutex::unlock(void)
{
    UINT status = tx_mutex_put(&m_mutex);
    return (status == TX_SUCCESS);
}

Semaphore::Semaphore(int count)
: m_sem()
{
    CHAR name[] = "erpc_sem";
    tx_semaphore_create(&m_sem, name, count);
}

Semaphore::~Semaphore(void)
{
    tx_semaphore_delete(&m_sem);
}

void Semaphore::put(void)
{
    tx_semaphore_put(&m_sem);
}

bool Semaphore::get(uint32_t timeoutUsecs)
{
    if (timeoutUsecs != kWaitForever)
    {
        if (timeoutUsecs > 0U)
        {
            timeoutUsecs /= 1000U;
            if (timeoutUsecs == 0U)
            {
                timeoutUsecs = 1U;
            }
        }
    }

    UINT status = tx_semaphore_get(&m_sem, timeoutUsecs);
    return (status == TX_SUCCESS);
}

int Semaphore::getCount(void) const
{
    CHAR *name;
    ULONG current_value;
    TX_THREAD *first_suspended;
    ULONG suspended_count;
    TX_SEMAPHORE *next_semaphore;
    UINT status;

    status = tx_semaphore_info_get((TX_SEMAPHORE *)&m_sem, &name, &current_value, &first_suspended, &suspended_count,
                                   &next_semaphore);

    if (status != TX_SUCCESS)
    {
        // return -1;
    }
    return current_value;
}

#endif /* ERPC_THREADS_IS(THREADX) */

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
