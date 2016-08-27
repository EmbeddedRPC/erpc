/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
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

#ifndef _ERPC_TRANSPORT_SETUP_H_
#define _ERPC_TRANSPORT_SETUP_H_

#include "erpc_version.h"
#include <stdint.h>

/*!
 * @addtogroup transport_setup
 * @{
 * @file
 */

////////////////////////////////////////////////////////////////////////////////
// Types
////////////////////////////////////////////////////////////////////////////////

//! @brief Opaque transport object type.
typedef struct ErpcTransport *erpc_transport_t;

////////////////////////////////////////////////////////////////////////////////
// API
////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

//! @name Transport setup
//@{

/*!
 * @brief Create a UART transport.
 */
erpc_transport_t erpc_transport_uart_init(void *baseAddr, uint32_t baudRate, uint32_t srcClock_Hz);

/*!
 * @brief Create an LPUART transport.
 */
erpc_transport_t erpc_transport_lpuart_init(void *baseAddr, uint32_t baudRate, uint32_t srcClock_Hz);

/*!
 * @brief Create a host PC serial port transport.
 */
erpc_transport_t erpc_transport_serial_init(const char *portName, long baudRate);

/*!
 * @brief Create a SPI master transport.
 */
erpc_transport_t erpc_transport_spi_master_init(void *baseAddr, uint32_t baudRate, uint32_t srcClock_Hz);

/*!
 * @brief Create a SPI slave transport.
 */
erpc_transport_t erpc_transport_spi_slave_init(void *baseAddr, uint32_t baudRate, uint32_t srcClock_Hz);

/*!
 * @brief Create a DSPI master transport.
 */
erpc_transport_t erpc_transport_dspi_master_init(void *baseAddr, uint32_t baudRate, uint32_t srcClock_Hz);

/*!
 * @brief Create a DSPI slave transport.
 */
erpc_transport_t erpc_transport_dspi_slave_init(void *baseAddr, uint32_t baudRate, uint32_t srcClock_Hz);

//@}

//! @name RPMsg-Lite transport setup
//@{

/*!
 * @brief Create an RPMsg-Lite transport.
 */
erpc_transport_t erpc_transport_rpmsg_lite_master_init(unsigned long src_addr,
                                                       unsigned long dst_addr,
                                                       int rpmsg_link_id);

/*!
 * @brief Create an RPMsg-Lite transport.
 */
erpc_transport_t erpc_transport_rpmsg_lite_remote_init(
    unsigned long src_addr, unsigned long dst_addr, void *start_address, int rpmsg_link_id, void (*ready_cb)(void));

/*!
 * @brief Create an RPMsg-Lite RTOS transport.
 */
erpc_transport_t erpc_transport_rpmsg_lite_rtos_master_init(unsigned long src_addr,
                                                            unsigned long dst_addr,
                                                            int rpmsg_link_id);

/*!
 * @brief Create an RPMsg-Lite RTOS transport.
 */
erpc_transport_t erpc_transport_rpmsg_lite_rtos_remote_init(
    unsigned long src_addr, unsigned long dst_addr, void *start_address, int rpmsg_link_id, void (*ready_cb)(void));

//@}

//! @name RPMsg transport setup
//@{

/*!
 * @brief Create an RPMsg transport.
 */
erpc_transport_t erpc_transport_rpmsg_init(int dev_id, int role);

/*!
 * @brief Create an RPMsg RTOS transport.
 */
erpc_transport_t erpc_transport_rpmsg_rtos_init(int dev_id, int role);

//@}

#ifdef __cplusplus
}
#endif

/*! @} */

#endif // _ERPC_TRANSPORT_SETUP_H_
