/*
 * Copyright 2020 NXP
 * Copyright 2020 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_setup_extensions.h"
#include "erpc_threading.h"
#include "erpc_config.h"

using namespace erpc;

static Semaphore *s_erpc_call_in_progress = NULL;
static TimerHandle_t s_erpc_call_timer_cb = NULL;

void erpc::erpc_pre_cb_default()
{
    erpc_assert(s_erpc_call_in_progress &&
           "If you want use default pre cb action, do not forget call erpc_init_call_progress_detection_default.");
    s_erpc_call_in_progress->get(s_erpc_call_in_progress->kWaitForever);
    erpc_assert(s_erpc_call_timer_cb &&
           "If you want use default pre cb action, do not forget call erpc_init_call_progress_detection_default.");
    xTimerStart(s_erpc_call_timer_cb, 0);
}

void erpc::erpc_post_cb_default()
{
    xTimerStop(s_erpc_call_timer_cb, 0);
    s_erpc_call_in_progress->put();
}

void erpc_call_timer_cb_default(TimerHandle_t xTimer)
{
    erpc_assert(1 != 1 && "eRPC task freezed.");
}

void erpc_init_call_progress_detection_default(
    erpc_call_timer_cb_default_t erpc_call_timer_cb = erpc_call_timer_cb_default, uint32_t waitTimeMs = 5 * 60 * 1000)
{
    s_erpc_call_in_progress = new Semaphore(1);
    erpc_assert(s_erpc_call_in_progress && "Creating eRPC semaphore failed.");

    s_erpc_call_timer_cb =
        xTimerCreate("Erpc client call timer", waitTimeMs / portTICK_PERIOD_MS, pdFALSE, NULL, erpc_call_timer_cb);
    erpc_assert(s_erpc_call_timer_cb && "Creating eRPC timer failed.");
}

void erpc_deinit_call_progress_detection_default()
{
    if (s_erpc_call_in_progress)
    {
        delete s_erpc_call_in_progress;
        s_erpc_call_in_progress = NULL;
    }

    if (s_erpc_call_timer_cb)
    {
        xTimerDelete(s_erpc_call_timer_cb, 0);
        s_erpc_call_timer_cb = NULL;
    }
}

bool erpc_is_call_in_progress_default()
{
    erpc_assert(s_erpc_call_in_progress &&
           "If you want use default pre cb action, do not forget call erpc_init_call_progress_detection_default.");
    if (s_erpc_call_in_progress->get(0))
    {
        s_erpc_call_in_progress->put();
        return false;
    }
    return true;
}

void erpc_reset_in_progress_state_default()
{

    erpc_assert(s_erpc_call_in_progress &&
           "If you want use default pre cb action, do not forget call erpc_init_call_progress_detection_default.");
    s_erpc_call_in_progress->get(0);
    s_erpc_call_in_progress->put();

    erpc_assert(s_erpc_call_timer_cb &&
           "If you want use default pre cb action, do not forget call erpc_init_call_progress_detection_default.");
    xTimerStop(s_erpc_call_timer_cb, 0);
}
