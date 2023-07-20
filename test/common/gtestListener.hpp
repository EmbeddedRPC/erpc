/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__GTESTLISTENER_H_
#define _EMBEDDED_RPC__GTESTLISTENER_H_

#include "gtest.h"
#include "myAlloc.h"
#include "c_test_unit_test_common_client.h"

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

class LeakChecker : public ::testing::EmptyTestEventListener
{
private:
    // Called after a test ends.
    virtual void OnTestEnd(const ::testing::TestInfo & /* test_info */)
    {
        int serverAlloc = getServerAllocated();

        EXPECT_EQ(MyAlloc::allocated(), 0)
            << "Leaked (on client side) : " << MyAlloc::allocated() << " unit(s) need be freed!";
        EXPECT_EQ(serverAlloc, 0) << "Leaked (on server side) : " << serverAlloc << " unit(s) need be freed!";
        MyAlloc::allocated(0);
    }
};

#endif // _EMBEDDED_RPC__GTESTLISTENER_H_
