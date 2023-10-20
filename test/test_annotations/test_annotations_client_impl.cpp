/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "c_test_client.h"
#include "gtest.h"
#include "unit_test_wrapped.h"

////////////////////////////////////////////////////////////////////////////////
// Unit test Implementation code
////////////////////////////////////////////////////////////////////////////////

void initInterfaces(erpc_client_t client)
{
    initAnnotateTest_client(client);
}

TEST(test_annotations, AnnotationServiceID)
{
    EXPECT_EQ(kAnnotateTest_service_id, 5);
}

TEST(test_annotations, IncludeAnnotationCheck)
{
    EXPECT_EQ(addOne(4), 5);

    includedInt_t testInt = 5;
    EXPECT_EQ(testInt, 5);
}

TEST(test_annotations, testIfMyIntAndConstExist)
{
    EXPECT_EQ(i, testIfMyIntAndConstExist(i));
}
