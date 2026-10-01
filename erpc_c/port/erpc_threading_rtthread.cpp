/*
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_threading.h"

#if ERPC_THREADS_IS(RTTHREAD)

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

#ifndef ERPC_RTTHREAD_THREAD_STACK_SIZE
#define ERPC_RTTHREAD_THREAD_STACK_SIZE (2048U)
#endif

#ifndef ERPC_RTTHREAD_THREAD_PRIORITY
#define ERPC_RTTHREAD_THREAD_PRIORITY (20U)
#endif

#ifndef ERPC_RTTHREAD_THREAD_TIMESLICE
#define ERPC_RTTHREAD_THREAD_TIMESLICE (10U)
#endif

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

static rt_tick_t usecsToTicks(uint32_t usecs)
{
    rt_int32_t ms;
    rt_tick_t ticks;

    if (usecs == 0U)
    {
        return 0;
    }

    ms = (rt_int32_t)(((uint64_t)usecs + 999ULL) / 1000ULL);
    ticks = (rt_tick_t)rt_tick_from_millisecond(ms);
    if (ticks == 0U)
    {
        ticks = 1U;
    }

    return ticks;
}

static rt_int32_t usecsToWaitTicks(uint32_t timeoutUsecs)
{
    if (timeoutUsecs == Semaphore::kWaitForever)
    {
        return RT_WAITING_FOREVER;
    }

    if (timeoutUsecs == 0U)
    {
        return RT_WAITING_NO;
    }

    return (rt_int32_t)usecsToTicks(timeoutUsecs);
}

static rt_uint8_t getThreadPriority(uint32_t priority)
{
    uint32_t rtPriority = (priority == 0U) ? ERPC_RTTHREAD_THREAD_PRIORITY : priority;

    if ((priority == 0U) && (rtPriority >= RT_THREAD_PRIORITY_MAX))
    {
        rtPriority = RT_THREAD_PRIORITY_MAX - 1U;
    }

    erpc_assert((rtPriority < RT_THREAD_PRIORITY_MAX) && ("RT-Thread priority out of range." != NULL));
    return (rt_uint8_t)rtPriority;
}

static rt_uint32_t getThreadStackSize(uint32_t stackSize)
{
    return (stackSize == 0U) ? ERPC_RTTHREAD_THREAD_STACK_SIZE : stackSize;
}

Thread::Thread(const char *name) :
m_name(name), m_entry(NULL), m_arg(NULL), m_stackSize(0), m_priority(0), m_stackPtr(NULL), m_thread(RT_NULL)
{
}

Thread::Thread(thread_entry_t entry, uint32_t priority, uint32_t stackSize, const char *name,
               thread_stack_pointer stackPtr) :
m_name(name), m_entry(entry), m_arg(NULL), m_stackSize(stackSize), m_priority(priority), m_stackPtr(stackPtr), m_thread(RT_NULL)
{
}

Thread::~Thread(void)
{
    if ((m_thread != RT_NULL) && (m_thread->user_data == static_cast<rt_ubase_t>(reinterpret_cast<uintptr_t>(this))))
    {
        m_thread->user_data = 0U;
    }
}

void Thread::init(thread_entry_t entry, uint32_t priority, uint32_t stackSize, thread_stack_pointer stackPtr)
{
    m_entry = entry;
    m_stackSize = stackSize;
    m_priority = priority;
    m_stackPtr = stackPtr;
}

void Thread::start(void *arg)
{
    const char *threadName = (m_name != NULL) ? m_name : "erpc";
    rt_uint8_t priority = getThreadPriority(m_priority);
    rt_uint32_t stackSize = getThreadStackSize(m_stackSize);

    if (m_stackPtr != NULL)
    {
        erpc_assert((m_stackPtr == NULL) && ("RT-Thread backend uses rt_thread_create()." != NULL));
        return;
    }

    m_arg = arg;

    m_thread = rt_thread_create(threadName, threadEntryPointStub, this, stackSize, priority, ERPC_RTTHREAD_THREAD_TIMESLICE);
    erpc_assert(m_thread != RT_NULL);
    if (m_thread != RT_NULL)
    {
        rt_err_t err;

        m_thread->user_data = static_cast<rt_ubase_t>(reinterpret_cast<uintptr_t>(this));
        err = rt_thread_startup(m_thread);
        erpc_assert(err == RT_EOK);
    }
}

bool Thread::operator==(Thread &o)
{
    return m_thread == o.m_thread;
}

Thread *Thread::getCurrentThread(void)
{
    rt_thread_t thread = rt_thread_self();

    if ((thread == RT_NULL) || (thread->user_data == 0U))
    {
        return NULL;
    }

    return reinterpret_cast<Thread *>(static_cast<uintptr_t>(thread->user_data));
}

void Thread::sleep(uint32_t usecs)
{
    rt_tick_t ticks = usecsToTicks(usecs);

    if (ticks == 0U)
    {
        (void)rt_thread_yield();
    }
    else
    {
        (void)rt_thread_delay(ticks);
    }
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
    rt_thread_t current = rt_thread_self();

    erpc_assert((_this != NULL) && ("Reinterpreting 'void *arg' to 'Thread *' failed." != NULL));
    if (current != RT_NULL)
    {
        current->user_data = static_cast<rt_ubase_t>(reinterpret_cast<uintptr_t>(_this));
    }

    _this->threadEntryPoint();

    if ((current != RT_NULL) && (current->user_data == static_cast<rt_ubase_t>(reinterpret_cast<uintptr_t>(_this))))
    {
        current->user_data = 0U;
    }
    _this->m_thread = RT_NULL;
}

Mutex::Mutex(void) : m_mutex()
{
    rt_err_t err = rt_mutex_init(&m_mutex, "erpc_m", RT_IPC_FLAG_PRIO);
    erpc_assert(err == RT_EOK);
}

Mutex::~Mutex(void)
{
    (void)rt_mutex_detach(&m_mutex);
}

bool Mutex::tryLock(void)
{
    return (rt_mutex_take(&m_mutex, RT_WAITING_NO) == RT_EOK);
}

bool Mutex::lock(void)
{
    return (rt_mutex_take(&m_mutex, RT_WAITING_FOREVER) == RT_EOK);
}

bool Mutex::unlock(void)
{
    return (rt_mutex_release(&m_mutex) == RT_EOK);
}

Semaphore::Semaphore(int count) : m_sem()
{
    rt_err_t err;

    erpc_assert((count >= 0) && (count <= 0xFFFF));
    err = rt_sem_init(&m_sem, "erpc_s", (rt_uint32_t)count, RT_IPC_FLAG_PRIO);
    erpc_assert(err == RT_EOK);
}

Semaphore::~Semaphore(void)
{
    (void)rt_sem_detach(&m_sem);
}

void Semaphore::put(void)
{
    (void)rt_sem_release(&m_sem);
}

bool Semaphore::get(uint32_t timeoutUsecs)
{
    return (rt_sem_take(&m_sem, usecsToWaitTicks(timeoutUsecs)) == RT_EOK);
}

int Semaphore::getCount(void) const
{
    rt_base_t level;
    rt_uint16_t value;
    rt_sem_t sem = const_cast<rt_sem_t>(&m_sem);

    level = rt_spin_lock_irqsave(&(sem->spinlock));
    value = sem->value;
    rt_spin_unlock_irqrestore(&(sem->spinlock), level);

    return (int)value;
}

#endif /* ERPC_THREADS_IS(RTTHREAD) */

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
