/*
 * Copyright (c) 2019, Embedded Planet, Inc
 * Copyright 2021 ACRIOS Systems s.r.o.
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * For supporting transports and examples see:
 * https://github.com/EmbeddedPlanet/mbed-rpc
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_threading.h"

#include "platform/CriticalSectionLock.h"

#if ERPC_THREADS_IS(MBED)

namespace erpc {

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

Thread *Thread::s_first = NULL;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

Thread::Thread(const char *name)
: m_name(name)
, m_entry(0)
, m_arg(0)
, m_stackSize(0)
, m_priority(0)
, m_thread(NULL)
, m_next(NULL)
{
}

Thread::Thread(thread_entry_t entry, uint32_t priority, uint32_t stackSize, const char *name,
               thread_stack_pointer stackPtr)
: m_name(name)
, m_entry(entry)
, m_arg(0)
, m_stackSize(stackSize)
, m_priority(priority)
, m_thread(NULL)
, m_next(NULL)
{
}

Thread::~Thread(void)
{
    if (m_thread != NULL)
    {
        delete m_thread;
        m_thread = NULL;
    }
}

void Thread::init(thread_entry_t entry, uint32_t priority, uint32_t stackSize, thread_stack_pointer stackPtr)
{
    m_entry = entry;
    m_stackSize = stackSize;
    m_priority = priority;
    m_stackPtr = stackPtr;
    m_thread =
        new rtos::Thread(osPriorityNormal, // Ignore priority because erpc does not map their priority to anything
                         ((m_stackSize + sizeof(uint32_t) - 1) / sizeof(uint32_t)), // Round up number of words
                         NULL, m_name);
}

void Thread::start(void *arg)
{
    m_arg = arg;

    // Enter a critical section to disable preemptive scheduling until we add the newly
    // created thread to the linked list. This prevents a race condition if the new thread is
    // higher priority than the current thread, and the new thread calls getCurrenThread(),
    // which will scan the linked list.
    mbed::CriticalSectionLock::enable();

    if (s_first != NULL)
    {
        m_next = s_first;
    }
    s_first = this;

    // Start the thread
    m_thread->start(mbed::callback(&erpc::Thread::threadEntryPointStub, this));

    mbed::CriticalSectionLock::disable();
}

bool Thread::operator==(Thread &o)
{
    return (this->getThreadId() == o.getThreadId());
}

Thread *Thread::getCurrentThread()
{
    osThreadId_t currentThreadId = rtos::ThisThread::get_id();

    // Walk the threads list to find the Thread object for the current task.
    mbed::CriticalSectionLock::enable();
    Thread *it = s_first;
    while (it != NULL)
    {
        if (it->getThreadId() == currentThreadId)
        {
            break;
        }
        it = it->m_next;
    }
    mbed::CriticalSectionLock::disable();
    return it;
}

void Thread::sleep(uint32_t usecs)
{
    rtos::ThisThread::sleep_for(usecs / 1000);
}

void Thread::threadEntryPoint(void)
{
    if (m_entry != NULL)
    {
        m_entry(m_arg);
    }
}

void Thread::threadEntryPointStub(void *arg)
{
    Thread *_this = reinterpret_cast<Thread *>(arg);
    erpc_assert(_this != NULL); // Reinterpreting 'void *arg' to 'Thread *' failed.
    _this->threadEntryPoint();

    // Remove this thread from the linked list.
    mbed::CriticalSectionLock::enable();
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
    mbed::CriticalSectionLock::disable();
}

Mutex::Mutex(void)
{
    // Having a member variable mutex
    // was causing memory alignment issues...
    m_mutex = new rtos::Mutex();
}

Mutex::~Mutex(void)
{
    if (m_mutex != NULL)
    {
        delete m_mutex;
        m_mutex = NULL;
    }
}

bool Mutex::tryLock(void)
{
    // Pass a zero timeout to poll the mutex.
    return m_mutex->trylock();
}

bool Mutex::lock(void)
{
    return m_mutex->lock();
}

bool Mutex::unlock(void)
{
    return m_mutex->unlock();
}

Semaphore::Semaphore(int count)
: m_count(count)
{
    m_sem = new rtos::Semaphore(m_count);
}

Semaphore::~Semaphore(void)
{
    if (m_sem != NULL)
    {
        delete m_sem;
        m_sem = NULL;
    }
}

void Semaphore::put(void)
{
    m_sem->release();
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

    m_count = m_sem->wait(timeoutUsecs);
    return (m_count < 0);
}

int Semaphore::getCount(void) const
{
    return m_count;
}

} // namespace erpc

#endif
