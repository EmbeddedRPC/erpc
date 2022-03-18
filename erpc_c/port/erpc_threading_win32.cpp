/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2021 NXP
 * Copyright 2021 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_threading.h"

#include <errno.h>
#include <process.h>
#include <time.h>
#include <windows.h>

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

Thread *Thread::s_first = NULL;
BOOL Thread::m_critical_section_inited = FALSE;
CRITICAL_SECTION Thread::m_critical_section;

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
, m_thrdaddr(0)
, m_next(0)
{
}

Thread::Thread(thread_entry_t entry, uint32_t priority, uint32_t stackSize, const char *name,
               thread_stack_pointer stackPtr)
: m_name(name)
, m_entry(entry)
, m_arg(0)
, m_stackSize(stackSize)
, m_priority(priority)
, m_thread(0)
, m_thrdaddr(0)
, m_next(0)
{
}

Thread::~Thread(void) {}

void Thread::init(thread_entry_t entry, uint32_t priority, uint32_t stackSize, thread_stack_pointer stackPtr)
{
    m_entry = entry;
    m_stackSize = stackSize;
    m_priority = priority;
    m_stackPtr = stackPtr;
    if (m_critical_section_inited == FALSE)
    {
        InitializeCriticalSection(&m_critical_section);
        m_critical_section_inited = TRUE;
    }
}

void Thread::start(void *arg)
{
    m_arg = arg;

    EnterCriticalSection(&m_critical_section);
    m_thread = (HANDLE)_beginthreadex(NULL, m_stackSize, threadEntryPointStub, this, 0, &m_thrdaddr);

    // Link in this thread to the list.
    if (NULL != s_first)
    {
        m_next = s_first;
    }
    s_first = this;
    LeaveCriticalSection(&m_critical_section);
}

bool Thread::operator==(const Thread &o)
{
    return (m_thrdaddr == o.m_thrdaddr);
}

Thread *Thread::getCurrentThread(void)
{
    unsigned int thisThrdaddr = GetCurrentThreadId();

    // Walk the threads list to find the Thread object for the current task.
    EnterCriticalSection(&m_critical_section);
    Thread *it = s_first;
    while (it != NULL)
    {
        if (it->m_thrdaddr == thisThrdaddr)
        {
            break;
        }
        it = it->m_next;
    }
    LeaveCriticalSection(&m_critical_section);
    return it;
}

void Thread::sleep(uint32_t usecs)
{
    LARGE_INTEGER startTime;
    LARGE_INTEGER currTick;
    LARGE_INTEGER freq;
    uint32_t elapsedTimeMicroSeconds;

    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&startTime);

    do
    {
        QueryPerformanceCounter(&currTick);

        elapsedTimeMicroSeconds =
            static_cast<uint32_t>(((currTick.QuadPart - startTime.QuadPart) * 1000000) / freq.QuadPart);
    } while (elapsedTimeMicroSeconds < usecs);
}

void Thread::threadEntryPoint(void)
{
    if (m_entry != NULL)
    {
        m_entry(m_arg);
    }
}

unsigned WINAPI Thread::threadEntryPointStub(void *arg)
{
    Thread *_this = reinterpret_cast<Thread *>(arg);
    if (_this != NULL)
    {
        _this->threadEntryPoint();
    }

    return 0;
}

Mutex::Mutex(void)
{
    m_mutex = CreateMutex(NULL, FALSE, "");
}

Mutex::~Mutex(void)
{
    CloseHandle(m_mutex);
}

bool Mutex::tryLock(void)
{
    return (WAIT_OBJECT_0 == WaitForSingleObject(m_mutex, 0));
}

bool Mutex::lock(void)
{
    return (WAIT_OBJECT_0 == WaitForSingleObject(m_mutex, INFINITE));
}

bool Mutex::unlock(void)
{
    return ReleaseMutex(m_mutex);
}

Semaphore::Semaphore(int count)
: m_count(count)
, m_sem()
, m_mutex()
{
    m_sem = CreateSemaphore(NULL, m_count, 1, "");
}

Semaphore::~Semaphore(void)
{
    CloseHandle(m_sem);
}

void Semaphore::put(void)
{
    LONG precount;
    ReleaseSemaphore(m_sem, 1, &precount);
    m_mutex.lock();
    ++m_count;
    m_mutex.unlock();
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

    DWORD ret = WaitForSingleObject(m_sem, timeoutUsecs);
    m_mutex.lock();
    --m_count;
    m_mutex.unlock();
    return (WAIT_OBJECT_0 == ret);
}

int Semaphore::getCount(void) const
{
    return m_count;
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
