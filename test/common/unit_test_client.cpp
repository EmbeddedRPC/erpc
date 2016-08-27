/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
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

#include "erpc_client_setup.h"
#include "erpc_transport_setup.h"
#include "unit_test_common/unit_test_common.h"
#include "gtest.h"
#include "gtestListener.h"

#if RPMSG || UART || LPUART
extern "C" {
#include "fsl_debug_console.h"
#include "board.h"
#include "app_core0.h"
#include "mcmgr.h"
#if RPMSG
#include "rpmsg_lite.h"
#endif
}

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

class MinimalistPrinter : public ::testing::EmptyTestEventListener
{
    // Called before a test starts.
    virtual void OnTestStart(const ::testing::TestInfo &test_info)
    {
        PRINTF("*** Test %s.%s starting.\n", test_info.test_case_name(), test_info.name());
    }

    // Called after a failed assertion or a SUCCEED() invocation.
    virtual void OnTestPartResult(const ::testing::TestPartResult &test_part_result)
    {
        PRINTF("%s in %s:%d\n%s\n", test_part_result.failed() ? "*** Failure" : "Success", test_part_result.file_name(),
               test_part_result.line_number(), test_part_result.summary());
    }

    // Called after a test ends.
    virtual void OnTestEnd(const ::testing::TestInfo &test_info)
    {
        PRINTF("*** Test %s.%s ending.\n", test_info.test_case_name(), test_info.name());
    }

    virtual void OnTestCaseEnd(const ::testing::TestCase &test_case)
    {
        PRINTF("*** Total tests passed: %d, failed: %d.\n", test_case.successful_test_count(),
               test_case.failed_test_count());
    }
};
#endif

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

int MyAlloc::allocated_ = 0;

#if RPMSG
extern char rpmsg_lite_base[];
#endif

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    ::testing::TestEventListeners &listeners = ::testing::UnitTest::GetInstance()->listeners();
    listeners.Append(new LeakChecker);

#if RPMSG || UART || LPUART
    delete listeners.Release(listeners.default_result_printer());
    listeners.Append(new MinimalistPrinter);

    /* Initialize GIC */
    BOARD_InitHardware();

#ifdef CORE1_IMAGE_COPY_TO_RAM
    /* Calculate size of the image */
    uint32_t core1_image_size;
    core1_image_size = get_core1_image_size();
    PRINTF("Copy CORE1 image to address: 0x%x, size: %d\n", CORE1_BOOT_ADDRESS, core1_image_size);

    /* Copy application from FLASH to RAM */
    memcpy(CORE1_BOOT_ADDRESS, (void *)CORE1_IMAGE_START, core1_image_size);
#endif

#if RPMSG
    env_init();
#endif
#endif

#if RPMSG
    // MU_Init(MU0_A);

    /* start the second core */
    // MU_BootCoreB(MU0_A, kMU_CoreBootFromImem);
    /* Boot source for Core 1 */
    // MCMGR_StartCore(kMCMGR_Core1, CORE1_BOOT_ADDRESS);
    MCMGR_StartCore(kMCMGR_Core1, CORE1_BOOT_ADDRESS, (uint32_t)rpmsg_lite_base, kMCMGR_Start_Synchronous);

    /* Wait for remote side to come up. This delay is arbitrary and may
       need adjustment for different configuration of remote systems */
    env_sleep_msec(1000);
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

    erpc_client_init(transport);

    int i = RUN_ALL_TESTS();
    quit();
#if RPMSG
    /* wait a while to allow the erpc server side to finalize shutdown,
       otherwise an IPC interrupt can be triggered on the client side at the
       time the rpmsg is deinitilaized yet => hardfault */
    env_sleep_msec(10000);
#endif
    erpc_client_deinit();

    return i;
}
