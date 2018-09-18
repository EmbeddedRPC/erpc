/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _ERPC_TRANSPORT_SETUP_H_
#define _ERPC_TRANSPORT_SETUP_H_

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
//! @brief Ready callback object type for RPMsg-Lite transport.
typedef void (*rpmsg_ready_cb)(void);

////////////////////////////////////////////////////////////////////////////////
// API
////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

//! @name Transport setup
//@{

/*!
 * @brief Create a CMSIS UART transport.
 */
erpc_transport_t erpc_transport_cmsis_uart_init(void *uartDrv);

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

/*!
 * @brief Create an MU transport.
 */
erpc_transport_t erpc_transport_mu_init(void *baseAddr);

//@}

//! @name RPMsg-Lite transport setup
//@{

/*!
 * @brief Create an RPMsg-Lite transport.
 *
 * Create RPMsg-Lite baremetal transport instance, to be used at master core.
 *
 * @param[in] src_addr Address of local RPMsg endpoint used for communication.
 * @param[in] dst_addr Address of remote RPMsg endpoint used for communication.
 * @param[in] rpmsg_link_id Link ID used to define the rpmsg-lite instance, see
 *                          rpmsg_platform.h
 *
 * @return Return NULL or erpc_transport_t instance pointer.
 */
erpc_transport_t erpc_transport_rpmsg_lite_master_init(unsigned long src_addr, unsigned long dst_addr,
                                                       int rpmsg_link_id);

/*!
 * @brief Create an RPMsg-Lite transport.
 *
 * Create RPMsg-Lite baremetal transport instance, to be used at slave/remote
 * core.
 *
 * @param[in] src_addr Address of local RPMsg endpoint used for communication.
 * @param[in] dst_addr Address of remote RPMsg endpoint used for communication.
 * @param[in] start_address Shared memory base address used for this instance of
 *                          RPMsg-Lite.
 * @param[in] rpmsg_link_id Link ID used to define the rpmsg-lite instance, see
 *                          rpmsg_platform.h.
 * @param[in] ready Callback function, which gets called, when RPMsg is
 *                  initialized and master core can be notified.
 * @param[in] nameservice_name Name of the nameservice channel to be announced
 *                             to the other core.
 *
 * @return Return NULL or erpc_transport_t instance pointer.
 */
erpc_transport_t erpc_transport_rpmsg_lite_remote_init(unsigned long src_addr, unsigned long dst_addr,
                                                       void *start_address, int rpmsg_link_id, rpmsg_ready_cb ready,
                                                       char *nameservice_name);

/*!
 * @brief Create an RPMsg-Lite RTOS transport.
 *
 * Create RPMsg-Lite RTOS transport instance, to be used at master core.
 *
 * @param[in] src_addr Address of local RPMsg endpoint used for communication.
 * @param[in] dst_addr Address of remote RPMsg endpoint used for communication.
 * @param[in] rpmsg_link_id Link ID used to define the rpmsg-lite instance, see
 *                          rpmsg_platform.h
 *
 * @return Return NULL or erpc_transport_t instance pointer.
 */
erpc_transport_t erpc_transport_rpmsg_lite_rtos_master_init(unsigned long src_addr, unsigned long dst_addr,
                                                            int rpmsg_link_id);

/*!
 * @brief Create an RPMsg-Lite RTOS transport.
 *
 * Create RPMsg-Lite RTOS transport instance, to be used at slave/remote core.
 *
 * @param[in] src_addr Address of local RPMsg endpoint used for communication.
 * @param[in] dst_addr Address of remote RPMsg endpoint used for communication.
 * @param[in] start_address Shared memory base address used for this instance of
 *                          RPMsg-Lite.
 * @param[in] rpmsg_link_id Link ID used to define the rpmsg-lite instance, see
 *                          rpmsg_platform.h.
 * @param[in] ready Callback function, which gets called, when RPMsg is
 *                  initialized and master core can be notified.
 * @param[in] nameservice_name Name of the nameservice channel to be announced
 *                             to the other core.
 *
 * @return Return NULL or erpc_transport_t instance pointer.
 */
erpc_transport_t erpc_transport_rpmsg_lite_rtos_remote_init(unsigned long src_addr, unsigned long dst_addr,
                                                            void *start_address, int rpmsg_link_id,
                                                            rpmsg_ready_cb ready, char *nameservice_name);

/*!
 * @brief Create an RPMsg-Lite TTY transport.
 *
 * Create RPMsg-Lite TTY transport instance, to be used at slave/remote core.
 * This function is mainly used with Linux running on the master core.
 *
 * @param[in] src_addr Address of local RPMsg endpoint used for communication.
 * @param[in] dst_addr Address of remote RPMsg endpoint used for communication.
 * @param[in] start_address Shared memory base address used for this instance of
 *                          RPMsg-Lite.
 * @param[in] rpmsg_link_id Link ID used to define the rpmsg-lite instance, see
 *                          rpmsg_platform.h.
 * @param[in] ready Callback function, which gets called, when RPMsg is
 *                  initialized and master core can be notified.
 * @param[in] nameservice_name Name of the nameservice channel to be announced
 *                             to the other core.
 *
 * @return Return NULL or erpc_transport_t instance pointer.
 */
erpc_transport_t erpc_transport_rpmsg_lite_tty_rtos_remote_init(unsigned long src_addr, unsigned long dst_addr,
                                                                void *start_address, int rpmsg_link_id,
                                                                rpmsg_ready_cb ready, char *nameservice_name);

/*!
 * @brief Create an Linux RPMSG endpoint transport.
 *
 * This function is using RPMSG endpoints based on this implementation:
 * https://github.com/NXPmicro/rpmsg-sysfs/tree/0aa1817545a765c200b1b2f9b6680a420dcf9171 .
 *
 * When local/remote address is set to '-1', then default addresses will be used.
 * When type is set to '0', then Datagram model will be used, else Stream.
 *
 * @param[in] local_addr Local endpoint address.
 * @param[in] type Datagram or Stream.
 * @param[in] remote_addr Remote endpoint address.
 */
erpc_transport_t erpc_transport_rpmsg_linux_init(int16_t local_addr, int8_t type, int16_t remote_addr);
//@}

/*!
 * @brief Deinitialize an Linux RPMSG endpoint transport.
 */
void erpc_transport_rpmsg_linux_deinit(void);

#ifdef __cplusplus
}
#endif

/*! @} */

#endif // _ERPC_TRANSPORT_SETUP_H_
