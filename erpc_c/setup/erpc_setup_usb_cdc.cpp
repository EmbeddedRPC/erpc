/*
 * Copyright 2020 NXP
 * Copyright 2021 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_manually_constructed.hpp"
#include "erpc_transport_setup.h"
#include "erpc_usb_cdc_transport.hpp"

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

ERPC_MANUALLY_CONSTRUCTED_STATIC(UsbCdcTransport, s_usbTransport);

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

erpc_transport_t erpc_transport_usb_cdc_init(void *serialHandle, void *serialConfig, void *usbCdcConfig,
                                             uint8_t *usbRingBuffer, uint32_t usbRingBufferLength)
{
    erpc_transport_t transport;
    UsbCdcTransport *usbTransport;

#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_STATIC
    if (s_usbTransport.isUsed())
    {
        usbTransport = NULL;
    }
    else
    {
        s_usbTransport.construct(
            reinterpret_cast<serial_handle_t>(serialHandle), reinterpret_cast<serial_manager_config_t *>(serialConfig),
            reinterpret_cast<serial_port_usb_cdc_config_t *>(usbCdcConfig), usbRingBuffer, usbRingBufferLength);
        usbTransport = s_usbTransport.get();
    }
#elif ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_DYNAMIC
    usbTransport = new UsbCdcTransport(
        reinterpret_cast<serial_handle_t>(serialHandle), reinterpret_cast<serial_manager_config_t *>(serialConfig),
        reinterpret_cast<serial_port_usb_cdc_config_t *>(usbCdcConfig), usbRingBuffer, usbRingBufferLength);
#else
#error "Unknown eRPC allocation policy!"
#endif

    transport = reinterpret_cast<erpc_transport_t>(usbTransport);

    if (usbTransport != NULL)
    {
        if (usbTransport->init() != kErpcStatus_Success)
        {
            erpc_transport_usb_cdc_deinit(transport);
            transport = NULL;
        }
    }

    return transport;
}

void erpc_transport_usb_cdc_deinit(erpc_transport_t transport)
{
#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_STATIC
    (void)transport;
    s_usbTransport.destroy();
#elif ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_DYNAMIC
    erpc_assert(transport != NULL);

    UsbCdcTransport *usbTransport = reinterpret_cast<UsbCdcTransport *>(transport);

    delete usbTransport;
#endif
}
