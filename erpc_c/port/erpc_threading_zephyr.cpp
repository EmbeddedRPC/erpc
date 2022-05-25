/*
 * Copyright 2017-2021 NXP
 * Copyright 2021 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_threading.h"
#if ERPC_THREADS_IS(ZEPHYR)
using namespace erpc;
K_THREAD_STACK_DEFINE(saas_thread_stack, 4096);

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
{
}

Thread::~Thread(void) {}

void Thread::init(thread_entry_t entry, uint32_t priority, uint32_t stackSize, thread_stack_pointer stackPtr)
{
    m_entry = entry;
    m_stackSize = stackSize;
    m_priority = priority;
    m_stackPtr = stackPtr;
}

void Thread::start(void *arg)
{
    m_arg = arg;
    m_thread_id = k_thread_create(&m_thread, saas_thread_stack, 4096, (k_thread_entry_t) threadEntryPointStub, this, NULL, NULL, m_priority, 0, K_NO_WAIT);
}

bool Thread::operator==(Thread &o)
{
    return m_thread_id == o.m_thread_id;
}

Thread *Thread::getCurrentThread(void)
{
    return reinterpret_cast<Thread *>(k_thread_custom_data_get());
}

void Thread::sleep(uint32_t usecs)
{
    k_sleep(k_timeout_t {usecs / 1000});
}

void Thread::threadEntryPoint(void)
{
    if (m_entry != NULL)
    {
        m_entry(m_arg);
    }
}

void *Thread::threadEntryPointStub(void *arg1, void *arg2, void *arg3)
{
    Thread *_this = reinterpret_cast<Thread *>(arg1);
    erpc_assert(_this && "Reinterpreting 'void *arg1' to 'Thread *' failed.");
    k_thread_custom_data_set(arg1);
    _this->threadEntryPoint();

    // Handle a task returning from its function.
    k_thread_abort(k_current_get());
    return NULL;
}

Mutex::Mutex(void)
: m_mutex()
{
    k_mutex_init(&m_mutex);
}

Mutex::~Mutex(void) {}

bool Mutex::tryLock(void)
{
    return (k_mutex_lock(&m_mutex, K_NO_WAIT) == 0);
}

bool Mutex::lock(void)
{
    return (k_mutex_lock(&m_mutex, K_FOREVER) == 0);
}

bool Mutex::unlock(void)
{
    k_mutex_unlock(&m_mutex);
    return true;
}

Semaphore::Semaphore(int count)
: m_sem()
{
    // Set max count to highest signed int.
    k_sem_init(&m_sem, count, 0x7fffffff);
}

Semaphore::~Semaphore(void) {}

void Semaphore::put(void)
{
    k_sem_give(&m_sem);
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

    return (k_sem_take(&m_sem, k_timeout_t {timeoutUsecs}) == 0);
}

int Semaphore::getCount(void)
{
    return k_sem_count_get(&m_sem);
}
#endif /* ERPC_THREADS_IS(ZEPHYR) */

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
