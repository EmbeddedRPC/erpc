/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
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
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
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

#include "unit_test_wrapped.h"
#include "unit_test_common/unit_test_common_server.h"
#include "erpc_server_setup.h"
#include "erpc_transport_setup.h"
#include "simple_server.h"

#if RPMSG || UART || LPUART
extern "C" {
#include "app_core1.h"
#if RPMSG
#include "rpmsg_lite.h"
#endif
}
#endif

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

int MyAlloc::allocated_ = 0;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

int main(int argc, const char *argv[])
{
#if RPMSG
// MU_Init(MU0_B);
/* Initialize GIC */
// env_init();
#endif

    erpc_transport_t transport;
#if RPMSG
    transport = erpc_transport_rpmsg_lite_init(101, 100);
#elif UART
    transport = erpc_transport_uart_init(ERPC_BOARD_UART_BASEADDR, ERPC_BOARD_UART_BAUDRATE,
                          CLOCK_GetFreq(ERPC_BOARD_UART_CLKSRC);
#elif LPUART
    transport = erpc_transport_lpuart_init(ERPC_BOARD_UART_BASEADDR, ERPC_BOARD_UART_BAUDRATE,
                          CLOCK_GetFreq(ERPC_BOARD_UART_CLKSRC);
#endif

    /* Init server */
    erpc_server_init(transport);

    /* Add test services. This function call erpc_add_service_to_server for all necessary services. */
    add_services_to_server();

    /* Add common service */
    add_common_service();

    /* Add run server */
    erpc_server_run();

    /* Deinit server */
    erpc_server_deinit();

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Server helper functions
////////////////////////////////////////////////////////////////////////////////

void add_common_service()
{
    erpc_add_service_to_server(create_Common_service());
}

////////////////////////////////////////////////////////////////////////////////
// Common service implementations here
////////////////////////////////////////////////////////////////////////////////

void quit()
{
    erpc_server_stop();
}

int32_t getServerAllocated()
{
    int result = MyAlloc::allocated();
    MyAlloc::allocated(0);
    return result;
}
