/*
 * Copyright 2020-2021 NXP
 * Copyright 2020-2021 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_manually_constructed.h"
#include "erpc_setup_extensions.h"
#include "erpc_threading.h"

using namespace erpc;

static Semaphore *s_erpc_call_in_progress = NULL;
static TimerHandle_t s_erpc_call_timer_cb = NULL;
#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_STATIC
static StaticTimer_t s_static_erpc_call_timer_cb;
#endif

ERPC_MANUALLY_CONSTRUCTED_STATIC(Semaphore, s_semaphore);

void erpc::erpc_pre_cb_default(void)
{
    erpc_assert((s_erpc_call_in_progress != NULL) &&
           ("If you want use default pre cb action, do not forget call erpc_init_call_progress_detection_default." != NULL));
    (void)s_erpc_call_in_progress->get(s_erpc_call_in_progress->kWaitForever);
    erpc_assert((s_erpc_call_timer_cb != NULL) &&
           ("If you want use default pre cb action, do not forget call erpc_init_call_progress_detection_default." != NULL));
    (void)xTimerStart(s_erpc_call_timer_cb, 0);
}

void erpc::erpc_post_cb_default(void)
{
    (void)xTimerStop(s_erpc_call_timer_cb, 0);
    s_erpc_call_in_progress->put();
}

static void erpc_call_timer_cb_default(TimerHandle_t xTimer)
{
    (void)xTimer;
    erpc_assert(true && ("eRPC task freezed." != NULL));
}

void erpc_init_call_progress_detection_default(
    erpc_call_timer_cb_default_t erpc_call_timer_cb = erpc_call_timer_cb_default,
    uint32_t waitTimeMs = (5U * 60U * 1000U))
{
    const uint32_t semaphoreCount = 1;
#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_DYNAMIC
    s_erpc_call_in_progress = new Semaphore(semaphoreCount);
#elif ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_STATIC
    s_semaphore.construct(semaphoreCount);
    s_erpc_call_in_progress = s_semaphore.get();
#endif
    erpc_assert((s_erpc_call_in_progress != NULL) && ("Creating eRPC semaphore failed." != NULL));

#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_STATIC
    s_erpc_call_timer_cb = xTimerCreateStatic("Erpc client call timer", waitTimeMs / portTICK_PERIOD_MS, pdFALSE, NULL,
                                              erpc_call_timer_cb, &s_static_erpc_call_timer_cb);
#else
    s_erpc_call_timer_cb =
        xTimerCreate("Erpc client call timer", waitTimeMs / portTICK_PERIOD_MS, pdFALSE, NULL, erpc_call_timer_cb);
#endif
    erpc_assert((s_erpc_call_timer_cb != NULL) && ("Creating eRPC timer failed." != NULL));
}

void erpc_deinit_call_progress_detection_default(void)
{
    if (s_erpc_call_in_progress != NULL)
    {
#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_DYNAMIC
        delete s_erpc_call_in_progress;
#endif
        s_erpc_call_in_progress = NULL;
    }

    if (s_erpc_call_timer_cb != NULL)
    {
        (void)xTimerDelete(s_erpc_call_timer_cb, 0);
        s_erpc_call_timer_cb = NULL;
    }
}

bool erpc_is_call_in_progress_default(void)
{
    erpc_assert((s_erpc_call_in_progress != NULL) &&
           ("If you want use default pre cb action, do not forget call erpc_init_call_progress_detection_default." != NULL));
    if (s_erpc_call_in_progress->get(0))
    {
        s_erpc_call_in_progress->put();
        return false;
    }
    return true;
}

void erpc_reset_in_progress_state_default(void)
{

    erpc_assert((s_erpc_call_in_progress != NULL) &&
           ("If you want use default pre cb action, do not forget call erpc_init_call_progress_detection_default." != NULL));
    s_erpc_call_in_progress->get(0);
    s_erpc_call_in_progress->put();

    erpc_assert((s_erpc_call_timer_cb != NULL) &&
           ("If you want use default pre cb action, do not forget call erpc_init_call_progress_detection_default." != NULL));
    (void)xTimerStop(s_erpc_call_timer_cb, 0);
}
