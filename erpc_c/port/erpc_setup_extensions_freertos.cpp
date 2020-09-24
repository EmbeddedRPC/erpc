/*
 * Copyright 2020 NXP
 * Copyright 2020 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_setup_extensions.h"

#include <cassert>

using namespace erpc;

static bool s_erpc_call = false;
TimerHandle_t g_erpc_call_timer_cb = NULL;

void erpc::erpc_pre_cb_default()
{
    s_erpc_call = true;
    assert(g_erpc_call_timer_cb && "If you want use default pre cb action, do not forget call erpc_create_timer.");
    xTimerStart(g_erpc_call_timer_cb, 0);
}

void erpc::erpc_post_cb_default()
{
    xTimerStop(g_erpc_call_timer_cb, 0);
    s_erpc_call = false;
}

void erpc_call_timer_cb_default(TimerHandle_t xTimer)
{
    assert(1 != 1 && "eRPC task freezed.");
}

void erpc_create_timer_default(uint32_t waitTime = 5 * 60 * 1000)
{
    g_erpc_call_timer_cb = xTimerCreate("Erpc client call timer", waitTime, pdFALSE, NULL, erpc_call_timer_cb_default);
    assert(g_erpc_call_timer_cb && "Creating eRPC timer failed.");
}

bool is_erpc_call_executed_default()
{
    return s_erpc_call;
}

void set_erpc_call_execution_state_default(bool erpc_call)
{
    s_erpc_call = erpc_call;
}