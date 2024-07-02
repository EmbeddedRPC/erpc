/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <zephyr/kernel.h>

#include "rpmsg_lite.h"
#include "erpc_client_setup.h"

#include "c_test_unit_test_common_client.h"
#include "gtest.h"
#include "gtestListener.hpp"
#include "myAlloc.hpp"
#include "unit_test_wrapped.h"

// #ifdef UNITY_DUMP_RESULTS
// #include "corn_g_test.h"
// #endif

using namespace std;

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

class ZephyrPrinter : public ::testing::EmptyTestEventListener
{
    virtual void OnTestCaseStart(const ::testing::TestCase &test_case)
    {
        printk("[----------] %d %s from  %s\n", test_case.test_to_run_count(),
               (test_case.test_to_run_count() == 1 ? "test" : "tests"), test_case.name());
    }

    virtual void OnTestStart(const ::testing::TestInfo &test_info)
    {
        printk("[ RUN      ] %s.%s\n", test_info.test_case_name(), test_info.name());
    }

    virtual void OnTestDisabled(const ::testing::TestInfo &test_info)
    {
        printk("[ DISABLED ] %s.%s\n", test_info.test_case_name(), test_info.name());
    }

    virtual void OnTestPartResult(const ::testing::TestPartResult &test_part_result)
    {
        if (test_part_result.type() == ::testing::TestPartResult::kSuccess)
        {
            return;
        }
        else
        {
            printk("%s (%d) %s\n%s\n", test_part_result.file_name(), test_part_result.line_number(),
                   test_part_result.failed() ? "error" : "Success", test_part_result.summary());
        }
    }

    virtual void OnTestEnd(const ::testing::TestInfo &test_info)
    {
        if (test_info.result()->Passed())
        {
            printk("[       OK ] ");
        }
        else
        {
            printk("[  FAILED  ] ");
        }
        printk("%s.%s\n", test_info.test_case_name(), test_info.name());
    }

    virtual void OnTestCaseEnd(const ::testing::TestCase &test_case)
    {
        printk("%d %s from %s\n", test_case.test_to_run_count(),
               (test_case.test_to_run_count() == 1 ? "test" : "tests"), test_case.name());
    }

    virtual void OnTestProgramEnd(const ::testing::UnitTest &) { printk("[==========] Done running all tests.\r\n"); }
};

using namespace std;

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

#define RPMSG_LITE_LINK_ID (0)
int ::MyAlloc::allocated_ = 0;
extern char rpmsg_lite_base[];

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

int main(void)
{
    env_sleep_msec(4000);

    int fake_argc = 1;
    const auto fake_arg0 = "dummy";
    char *fake_argv0 = const_cast<char *>(fake_arg0);
    char **fake_argv = &fake_argv0;
    ::testing::InitGoogleTest(&fake_argc, fake_argv);

    ::testing::TestEventListeners &listeners = ::testing::UnitTest::GetInstance()->listeners();
    listeners.Append(new LeakChecker);

    delete listeners.Release(listeners.default_result_printer());
    listeners.Append(new ZephyrPrinter);

    erpc_transport_t transport;
    erpc_mbf_t message_buffer_factory;
    erpc_client_t client;

    transport = erpc_transport_rpmsg_lite_rtos_master_init(100, 101, RPMSG_LITE_LINK_ID);
    message_buffer_factory = erpc_mbf_rpmsg_init(transport);

    client = erpc_client_init(transport, message_buffer_factory);
    initInterfaces_common(client);
    initInterfaces(client);

    int i = RUN_ALL_TESTS();
    quit();

    erpc_client_deinit(client);

    return i;
}

void initInterfaces_common(erpc_client_t client)
{
    initCommon_client(client);
}
