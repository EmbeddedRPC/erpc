/*
 * The Clear BSD License
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
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

#include "erpc_threading.h"
#include <errno.h>
#include <sys/time.h>
#include <time.h>

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

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

Thread::~Thread()
{
}

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

Thread *Thread::getCurrentThread()
{
    void *value = pthread_getspecific(s_threadObjectKey);
    return reinterpret_cast<Thread *>(value);
}

void Thread::sleep(uint32_t usecs)
{
    // Sleep for the requested number of microseconds.
    struct timespec rq = {.tv_sec = usecs / 1000000, .tv_nsec = (usecs % 1000000) * 1000 };
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

void Thread::threadEntryPoint()
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

Mutex::Mutex()
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

    pthread_mutex_init(&m_mutex, &attr);

    pthread_mutexattr_destroy(&attr);
}

Mutex::~Mutex()
{
    pthread_mutex_destroy(&m_mutex);
}

bool Mutex::tryLock()
{
    return pthread_mutex_trylock(&m_mutex) == 0;
}

bool Mutex::lock()
{
    return pthread_mutex_lock(&m_mutex) == 0;
}

bool Mutex::unlock()
{
    return pthread_mutex_unlock(&m_mutex) == 0;
}

Semaphore::Semaphore(int count)
: m_count(count)
, m_mutex()
{
    pthread_cond_init(&m_cond, NULL);
}

Semaphore::~Semaphore()
{
    pthread_cond_destroy(&m_cond);
}

void Semaphore::put()
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
            struct timespec wait = {.tv_sec = tv.tv_sec + (timeout / 1000000), .tv_nsec = (timeout % 1000000) * 1000 };
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

int Semaphore::getCount() const
{
    return m_count;
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
