/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_threading.h"
#include <errno.h>
#include <sys/time.h>
#include <time.h>

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

/*!
 * Thread object key.
 */
pthread_key_t Thread::s_threadObjectKey = 0;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

Thread::Thread(const char *name)
: m_name(name)
, m_entry(0)
, m_arg(0)
, m_stackSize(0)
, m_priority(0)
, m_thread(0)
{
}

Thread::Thread(thread_entry_t entry, uint32_t priority, uint32_t stackSize, const char *name)
: m_name(name)
, m_entry(entry)
, m_arg(0)
, m_stackSize(stackSize)
, m_priority(priority)
, m_thread(0)
{
}

Thread::~Thread(void) {}

void Thread::init(thread_entry_t entry, uint32_t priority, uint32_t stackSize)
{
    m_entry = entry;
    m_stackSize = stackSize;
    m_priority = priority;
}

void Thread::start(void *arg)
{
    if (s_threadObjectKey == 0)
    {
        pthread_key_create(&s_threadObjectKey, NULL);
    }

    m_arg = arg;
    pthread_create(&m_thread, NULL, threadEntryPointStub, this);
    pthread_setspecific(s_threadObjectKey, reinterpret_cast<void *>(this));
    pthread_detach(m_thread);
}

bool Thread::operator==(Thread &o)
{
    return pthread_equal(m_thread, o.m_thread);
}

Thread *Thread::getCurrentThread(void)
{
    void *value = pthread_getspecific(s_threadObjectKey);
    return reinterpret_cast<Thread *>(value);
}

void Thread::sleep(uint32_t usecs)
{
    // Sleep for the requested number of microseconds.
    struct timespec rq = { .tv_sec = usecs / 1000000, .tv_nsec = (usecs % 1000000) * 1000 };
    struct timespec actual = { 0 };

    // Keep sleeping until the requested time elapses even if we get interrupted by a signal.
    while (nanosleep(&rq, &actual) == EINTR)
    {
        rq.tv_sec -= actual.tv_sec;
        rq.tv_nsec -= actual.tv_nsec;
        if (rq.tv_nsec < 0)
        {
            --rq.tv_sec;
            rq.tv_nsec += 1000000000;
        }
    }
}

void Thread::threadEntryPoint(void)
{
    if (m_entry)
    {
        m_entry(m_arg);
    }
}

void *Thread::threadEntryPointStub(void *arg)
{
    Thread *_this = reinterpret_cast<Thread *>(arg);
    if (_this)
    {
        _this->threadEntryPoint();
    }

    return 0;
}

Mutex::Mutex(void)
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

    pthread_mutex_init(&m_mutex, &attr);

    pthread_mutexattr_destroy(&attr);
}

Mutex::~Mutex(void)
{
    pthread_mutex_destroy(&m_mutex);
}

bool Mutex::tryLock(void)
{
    return pthread_mutex_trylock(&m_mutex) == 0;
}

bool Mutex::lock(void)
{
    return pthread_mutex_lock(&m_mutex) == 0;
}

bool Mutex::unlock(void)
{
    return pthread_mutex_unlock(&m_mutex) == 0;
}

Semaphore::Semaphore(int count)
: m_count(count)
, m_mutex()
{
    pthread_cond_init(&m_cond, NULL);
}

Semaphore::~Semaphore(void)
{
    pthread_cond_destroy(&m_cond);
}

void Semaphore::put(void)
{
    Mutex::Guard guard(m_mutex);
    if (m_count == 0)
    {
        pthread_cond_signal(&m_cond);
    }
    ++m_count;
}

bool Semaphore::get(uint32_t timeout)
{
    Mutex::Guard guard(m_mutex);
    int err;
    while (m_count == 0)
    {
        if (timeout == kWaitForever)
        {
            err = pthread_cond_wait(&m_cond, m_mutex.getPtr());
            if (err)
            {
                return false;
            }
        }
        else if (timeout > 0)
        {
            // Create an absolute timeout time.
            struct timeval tv;
            gettimeofday(&tv, NULL);
            struct timespec wait = { .tv_sec = tv.tv_sec + (timeout / 1000000), .tv_nsec = (timeout % 1000000) * 1000 };
            err = pthread_cond_timedwait(&m_cond, m_mutex.getPtr(), &wait);
            if (err)
            {
                return false;
            }
        }
    }
    --m_count;

    return true;
}

int Semaphore::getCount(void) const
{
    return m_count;
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
