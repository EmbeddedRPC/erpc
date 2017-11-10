/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
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
#include <cassert>
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

Thread::Thread(const char *name)
: m_name(name)
, m_entry(0)
, m_arg(0)
, m_stackSize(0)
, m_priority(0)
, m_task(0)
, m_next(0)
{
}

Thread::Thread(thread_entry_t entry, uint32_t priority, uint32_t stackSize, const char *name)
: m_name(name)
, m_entry(entry)
, m_arg(0)
, m_stackSize(stackSize)
, m_priority(priority)
, m_task(0)
, m_next(0)
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
    m_arg = arg;

    // Enter a critical section to disable preemptive scheduling until we add the newly
    // created thread to the linked list. This prevents a race condition if the new thread is
    // higher priority than the current thread, and the new thread calls getCurrenThread(),
    // which will scan the linked list.
    taskENTER_CRITICAL();

    xTaskCreate(threadEntryPointStub, (m_name ? m_name : "task"),
                ((m_stackSize + sizeof(uint32_t) - 1) / sizeof(uint32_t)), // Round up number of words.
                this, m_priority, &m_task);

    // Link in this thread to the list.
    if (s_first)
    {
        m_next = s_first;
    }
    s_first = this;

    taskEXIT_CRITICAL();
}

bool Thread::operator==(Thread &o)
{
    return m_task == o.m_task;
}

Thread *Thread::getCurrentThread()
{
    TaskHandle_t thisTask = xTaskGetCurrentTaskHandle();

    // Walk the threads list to find the Thread object for the current task.
    taskENTER_CRITICAL();
    Thread *it = s_first;
    while (it)
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
    vTaskDelay(usecs / 1000 / portTICK_PERIOD_MS);
#endif
}

void Thread::threadEntryPoint()
{
    if (m_entry)
    {
        m_entry(m_arg);
    }
}

void Thread::threadEntryPointStub(void *arg)
{
    Thread *_this = reinterpret_cast<Thread *>(arg);
    assert(_this && "Reinterpreting 'void *arg' to 'Thread *' failed.");
    _this->threadEntryPoint();

    // Remove this thread from the linked list.
    taskENTER_CRITICAL();
    Thread *it = s_first;
    Thread *prev = NULL;
    while (it)
    {
        if (it == _this)
        {
            if (it == s_first)
            {
                s_first = _this->m_next;
            }
            else if (prev)
            {
                prev->m_next = _this->m_next;
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

Mutex::Mutex()
: m_mutex(0)
{
    m_mutex = xSemaphoreCreateMutex();
}

Mutex::~Mutex()
{
    vSemaphoreDelete(m_mutex);
}

bool Mutex::tryLock()
{
    // Pass a zero timeout to poll the mutex.
    return xSemaphoreTakeRecursive(m_mutex, 0);
}

bool Mutex::lock()
{
    return xSemaphoreTakeRecursive(m_mutex, portMAX_DELAY);
}

bool Mutex::unlock()
{
    return xSemaphoreGiveRecursive(m_mutex);
}

Semaphore::Semaphore(int count)
: m_sem(0)
{
    // Set max count to highest signed int.
    m_sem = xSemaphoreCreateCounting(0x7fffffff, count);
}

Semaphore::~Semaphore()
{
    vSemaphoreDelete(m_sem);
}

void Semaphore::put()
{
    xSemaphoreGive(m_sem);
}

void Semaphore::putFromISR()
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(m_sem, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

bool Semaphore::get(uint32_t timeout)
{
#if configUSE_16_BIT_TICKS
    if (timeout == kWaitForever)
    {
        timeout = portMAX_DELAY;
    }
    else if (timeout > portMAX_DELAY - 1)
    {
        timeout = portMAX_DELAY - 1;
    }
#endif
    xSemaphoreTake(m_sem, timeout / 1000 / portTICK_PERIOD_MS);
    return true;
}

int Semaphore::getCount() const
{
    return static_cast<int>(uxQueueMessagesWaiting(m_sem));
}
#endif /* ERPC_THREADS_IS(FREERTOS) */

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
