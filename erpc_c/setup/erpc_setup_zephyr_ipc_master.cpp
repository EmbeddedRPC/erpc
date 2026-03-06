/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_transport_setup.h"
#include "erpc_manually_constructed.hpp"
#include "erpc_ipc_service_zephyr_transport.hpp"

#if defined(__ZEPHYR__)
#include <zephyr/device.h>
#include <zephyr/kernel.h>
#endif

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

ERPC_MANUALLY_CONSTRUCTED_STATIC(ZephyrIPCTransport, s_zephyr_ipc_transport);

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

erpc_transport_t erpc_transport_zephyr_ipc_master_init(const char *ept_name, uint32_t init_delay_ms)
{
    erpc_transport_t transport;
    ZephyrIPCTransport *ipcTransport;

#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_STATIC
    if (s_zephyr_ipc_transport.isUsed())
    {
        ipcTransport = NULL;
    }
    else
    {
        s_zephyr_ipc_transport.construct();
        ipcTransport = s_zephyr_ipc_transport.get();
    }
#elif ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_DYNAMIC
    ipcTransport = new (std::nothrow) ZephyrIPCTransport();
#else
#error "Unknown eRPC allocation policy!"
#endif

    transport = reinterpret_cast<erpc_transport_t>(ipcTransport);

    if (ipcTransport != NULL)
    {
#if defined(__ZEPHYR__)
        // Get IPC device from device tree - must use compile-time constant!
        const struct device *ipc_instance = DEVICE_DT_GET(DT_ALIAS(erpc_ipc));

        if (!device_is_ready(ipc_instance))
        {
            erpc_transport_zephyr_ipc_master_deinit(transport);
            return NULL;
        }

        // Main/Master core: Give some time for Remote core to boot up
        if (init_delay_ms > 0U)
        {
            k_sleep(K_MSEC(init_delay_ms));
        }

        if (ipcTransport->init(ipc_instance, ept_name) != kErpcStatus_Success)
        {
            erpc_transport_zephyr_ipc_master_deinit(transport);
            transport = NULL;
        }
#else
        erpc_transport_zephyr_ipc_master_deinit(transport);
        transport = NULL;
#endif
    }

    return transport;
}

void erpc_transport_zephyr_ipc_master_deinit(erpc_transport_t transport)
{
#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_STATIC
    (void)transport;
    s_zephyr_ipc_transport.destroy();
#elif ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_DYNAMIC
    erpc_assert(transport != NULL);

    ZephyrIPCTransport *ipcTransport = reinterpret_cast<ZephyrIPCTransport *>(transport);

    delete ipcTransport;
#endif
}
