/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "gtest.h"
#include "test.h"

////////////////////////////////////////////////////////////////////////////////
// Unit test Implementation code
////////////////////////////////////////////////////////////////////////////////

TEST(test_annotations, AnnotationServiceID)
{
    EXPECT_TRUE(5 == kAnnotateTest_service_id);
}

TEST(test_annotations, IncludeAnnotationCheck)
{
    EXPECT_TRUE(5 == addOne(4));

    includedInt_t testInt = 5;
    EXPECT_TRUE(5 == testInt);
}

TEST(test_annotations, testIfMyIntAndConstExist)
{
    EXPECT_TRUE(i == testIfMyIntAndConstExist(i));
}
