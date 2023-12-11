/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2023 NXP
 * Copyright 2021 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_threading.h"

#include <errno.h>

#if ERPC_THREADS_IS(FREERTOS)

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

Thread *Thread::s_first = NULL;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

Thread::Thread(const char *name) :
m_name(name), m_entry(0), m_arg(0), m_stackSize(0), m_priority(0), m_task(0), m_next(0), m_stackPtr(0)
{
}

Thread::Thread(thread_entry_t entry, uint32_t priority, uint32_t stackSize, const char *name,
               thread_stack_pointer stackPtr) :
m_name(name),
m_entry(entry), m_arg(0), m_stackSize(stackSize), m_priority(priority), m_task(0), m_next(0), m_stackPtr(stackPtr)
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
    bool taskCreated = false;

    // Enter a critical section to disable preemptive scheduling until we add the newly
    // created thread to the linked list. This prevents a race condition if the new thread is
    // higher priority than the current thread, and the new thread calls getCurrenThread(),
    // which will scan the linked list.
    taskENTER_CRITICAL();

#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_STATIC
    if (m_stackPtr != NULL)
    {
        m_task = xTaskCreateStatic(threadEntryPointStub, (m_name != NULL ? m_name : "task"),
                                   (configSTACK_DEPTH_TYPE)((m_stackSize + sizeof(uint32_t) - 1U) /
                                                            sizeof(uint32_t)), // Round up number of words.
                                   this, m_priority, m_stackPtr, &m_staticTask);
        taskCreated = true;
    }
#endif

#if configSUPPORT_DYNAMIC_ALLOCATION
    if (m_stackPtr == NULL)
    {
        if (pdPASS == xTaskCreate(threadEntryPointStub, (m_name != NULL ? m_name : "task"),
                                  (configSTACK_DEPTH_TYPE)((m_stackSize + sizeof(uint32_t) - 1U) /
                                                           sizeof(uint32_t)), // Round up number of words.
                                  this, m_priority, &m_task))
        {
            taskCreated = true;
        }
    }
#endif

    if (taskCreated)
    {
        // Link in this thread to the list.
        if (NULL != s_first)
        {
            m_next = s_first;
        }
        s_first = this;
    }

    taskEXIT_CRITICAL();
}

bool Thread::operator==(Thread &o)
{
    return m_task == o.m_task;
}

Thread *Thread::getCurrentThread(void)
{
    TaskHandle_t thisTask = xTaskGetCurrentTaskHandle();

    // Walk the threads list to find the Thread object for the current task.
    taskENTER_CRITICAL();
    Thread *it = s_first;
    while (it != NULL)
    {
        if (it->m_task == thisTask)
        {
            break;
        }
        it = it->m_next;
    }
    taskEXIT_CRITICAL();
    return it;
}

void Thread::sleep(uint32_t usecs)
{
#if INCLUDE_vTaskDelay
    vTaskDelay(usecs / 1000U / portTICK_PERIOD_MS);
#endif
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
    erpc_assert((_this != NULL) && ("Reinterpreting 'void *arg' to 'Thread *' failed." != NULL));
    _this->threadEntryPoint();

    // Remove this thread from the linked list.
    taskENTER_CRITICAL();
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
    taskEXIT_CRITICAL();

// Handle a task returning from its function. Delete or suspend the task, if the API is
// available. If neither API is included, then just enter an infinite loop. If vTaskDelay()
// is available, the loop sleeps this task the maximum time each cycle. If not, it just
// yields.
#if INCLUDE_vTaskDelete
    _this->m_task = 0;
    vTaskDelete(NULL);
#elif INCLUDE_vTaskSuspend
    vTaskSuspend(NULL);
#else // INCLUDE_vTaskSuspend
    while (true)
    {
#if INCLUDE_vTaskDelay
        vTaskDelay(portMAX_DELAY);
#else  // INCLUDE_vTaskDelay
        taskYIELD();
#endif // INCLUDE_vTaskDelay
    }
#endif // INCLUDE_vTaskDelete
}

Mutex::Mutex(void) : m_mutex(0)
{
#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_STATIC
    m_mutex = xSemaphoreCreateRecursiveMutexStatic(&m_staticQueue);
#elif configSUPPORT_DYNAMIC_ALLOCATION
    m_mutex = xSemaphoreCreateRecursiveMutex();
#else
#error "Allocation method didn't match"
#endif
}

Mutex::~Mutex(void)
{
    vSemaphoreDelete(m_mutex);
}

bool Mutex::tryLock(void)
{
    // Pass a zero timeout to poll the mutex.
    return (pdTRUE == xSemaphoreTakeRecursive(m_mutex, 0) ? true : false);
}

bool Mutex::lock(void)
{
    return (pdTRUE == xSemaphoreTakeRecursive(m_mutex, portMAX_DELAY) ? true : false);
}

bool Mutex::unlock(void)
{
    return (pdTRUE == xSemaphoreGiveRecursive(m_mutex) ? true : false);
}

Semaphore::Semaphore(int count) : m_sem(0)
{
    // Set max count to highest signed int.
#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_STATIC
    m_sem = xSemaphoreCreateCountingStatic(0x7fffffffu, (UBaseType_t)count, &m_staticQueue);
#elif configSUPPORT_DYNAMIC_ALLOCATION
    m_sem = xSemaphoreCreateCounting(0x7fffffffu, (UBaseType_t)count);
#else
#error "Allocation method didn't match"
#endif
}

Semaphore::~Semaphore(void)
{
    vSemaphoreDelete(m_sem);
}

void Semaphore::put(void)
{
    (void)xSemaphoreGive(m_sem);
}

void Semaphore::putFromISR(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    (void)xSemaphoreGiveFromISR(m_sem, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

bool Semaphore::get(uint32_t timeoutUsecs)
{
    if (timeoutUsecs != kWaitForever)
    {
        if (timeoutUsecs > 0U)
        {
            timeoutUsecs /= 1000U * portTICK_PERIOD_MS;
            if (timeoutUsecs == 0U)
            {
                timeoutUsecs = 1U;
            }
#if configUSE_16_BIT_TICKS
            else if (timeoutUsecs > (portMAX_DELAY - 1))
            {
                timeoutUsecs = portMAX_DELAY - 1;
            }
#endif
        }
    }
#if configUSE_16_BIT_TICKS
    else
    {
        timeoutUsecs = portMAX_DELAY;
    }
#endif

    return (pdTRUE == xSemaphoreTake(m_sem, (TickType_t)timeoutUsecs));
}

int Semaphore::getCount(void) const
{
    return static_cast<int>(uxQueueMessagesWaiting(m_sem));
}
#endif /* ERPC_THREADS_IS(FREERTOS) */

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
