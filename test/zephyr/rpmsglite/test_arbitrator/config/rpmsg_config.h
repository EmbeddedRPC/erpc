/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * All rights reserved.
 * Copyright (c) 2015 Xilinx, Inc. All rights reserved.
 * Copyright 2016 Freescale Semiconductor, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RPMSG_CONFIG_H_
#define RPMSG_CONFIG_H_

#include "erpc_config_internal.h"

/* RPMsg config values */
/* START { */
#define RL_MS_PER_INTERVAL (1)

#define RL_BUFFER_PAYLOAD_SIZE (ERPC_DEFAULT_BUFFER_SIZE)

#define RL_BUFFER_COUNT (ERPC_DEFAULT_BUFFERS_COUNT)

#define RL_API_HAS_ZEROCOPY (1)

#define RL_USE_STATIC_API (0)

#define RL_USE_MCMGR_IPC_ISR_HANDLER (1)

// #define SH_MEM_TOTAL_SIZE (22527)

#define RL_ASSERT(x)  \
    do                \
    {                 \
        if (!(x))     \
            while (1) \
                ;     \
    } while (0);
/* } END */

#endif /* RPMSG_CONFIG_H_ */
