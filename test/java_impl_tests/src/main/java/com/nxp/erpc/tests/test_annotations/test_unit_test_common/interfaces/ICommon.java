/*!
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/** 
 * Generated by erpcgen 1.12.0 on Mon Oct 30 10:47:32 2023.
 * 
 * AUTOGENERATED - DO NOT EDIT
 */ 

package com.nxp.erpc.tests.test_annotations.test_unit_test_common.interfaces;



import com.nxp.erpc.tests.common.myEnum;

import com.nxp.erpc.auxiliary.Reference;

import java.util.List;

public interface ICommon {
    int SERVICE_ID = 0;
    int QUIT_ID = 1;
    int GETSERVERALLOCATED_ID = 2;

    void quit();
    int getServerAllocated();
}


