/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "Logging.h"
#include "out.h"
#include "unit_test.h"

#include <boost/test/unit_test.hpp>

using namespace boost::unit_test;

////////////////////////////////////////////////////////////////////////////////
//// Unit test Implementation
////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE(Unit_Tests)

/* Test cases */
BOOST_AUTO_TEST_CASE(GetMember1)
{
    BOOST_CHECK(1 == 1);
}

BOOST_AUTO_TEST_SUITE_END()
