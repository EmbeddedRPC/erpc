/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
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
#include <stdbool.h>

//! @name Transport setup
//@{

//! @name CMSIS UART transport setup
//@{

/*!
 * @brief Create a CMSIS UART transport.
 *
 * Create a CMSIS UART transport instance, to be used on both the server
 * and the client side.
 *
 * @param[in] uartDrv CMSIS USART driver structure address (Driver Control Block).
 *
 * @return Return NULL or erpc_transport_t instance pointer.
 */
erpc_transport_t erpc_transport_cmsis_uart_init(void *uartDrv);
//@}

//! @name Host PC serial port transport setup
//@{

/*!
 * @brief Create a host PC serial port transport.
 *
 * Create a host PC serial port transport instance.
 *
 * @param[in] portName Port name.
 * @param[in] baudRate Baud rate.
 *
 * @return Return NULL or erpc_transport_t instance pointer.
 */
erpc_transport_t erpc_transport_serial_init(const char *portName, long baudRate);
//@}

//! @name SPI transport setup
//@{

/*!
 * @brief Create a SPI master transport.
 *
 * Create SPI master transport instance, to be used at master core.
 *
 * @param[in] baseAddr Base address of SPI peripheral used in this transport layer.
 * @param[in] baudRate SPI baud rate.
 * @param[in] srcClock_Hz SPI source clock in Hz.
 *
 * @return Return NULL or erpc_transport_t instance pointer.
 */
erpc_transport_t erpc_transport_spi_master_init(void *baseAddr, uint32_t baudRate, uint32_t srcClock_Hz);

/*!
 * @brief Create a SPI slave transport.
 *
 * Create SPI slave transport instance, to be used at slave core.
 *
 * @param[in] baseAddr Base address of SPI peripheral used in this transport layer.
 * @param[in] baudRate SPI baud rate.
 * @param[in] srcClock_Hz SPI source clock in Hz.
 *
 * @return Return NULL or erpc_transport_t instance pointer.
 */
erpc_transport_t erpc_transport_spi_slave_init(void *baseAddr, uint32_t baudRate, uint32_t srcClock_Hz);
//@}

//! @name DSPI transport setup
//@{

/*!
 * @brief Create a DSPI master transport.
 *
 * Create DSPI master transport instance, to be used at master core.
 *
 * @param[in] baseAddr Base address of DSPI peripheral used in this transport layer.
 * @param[in] baudRate DSPI baud rate.
 * @param[in] srcClock_Hz DSPI source clock in Hz.
 *
 * @return Return NULL or erpc_transport_t instance pointer.
 */
erpc_transport_t erpc_transport_dspi_master_init(void *baseAddr, uint32_t baudRate, uint32_t srcClock_Hz);

/*!
 * @brief Create a DSPI slave transport.
 *
 * Create DSPI slave transport instance, to be used at slave core.
 *
 * @param[in] baseAddr Base address of DSPI peripheral used in this transport layer.
 * @param[in] baudRate DSPI baud rate.
 * @param[in] srcClock_Hz DSPI source clock in Hz.
 *
 * @return Return NULL or erpc_transport_t instance pointer.
 */
erpc_transport_t erpc_transport_dspi_slave_init(void *baseAddr, uint32_t baudRate, uint32_t srcClock_Hz);
//@}

//! @name MU transport setup
//@{

/*!
 * @brief Create an MU transport.
 *
 * Create Messaging Unit (MU) transport instance, to be used on both the server
 * and the client side. Base address of the MU peripheral needs to be passed.
 *
 * @param[in] baseAddr Base address of MU peripheral.
 *
 * @return Return NULL or erpc_transport_t instance pointer.
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
erpc_transport_t erpc_transport_rpmsg_lite_master_init(uint32_t src_addr, uint32_t dst_addr, uint32_t rpmsg_link_id);

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
erpc_transport_t erpc_transport_rpmsg_lite_remote_init(uint32_t src_addr, uint32_t dst_addr, void *start_address,
                                                       uint32_t rpmsg_link_id, rpmsg_ready_cb ready,
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
erpc_transport_t erpc_transport_rpmsg_lite_rtos_master_init(uint32_t src_addr, uint32_t dst_addr,
                                                            uint32_t rpmsg_link_id);

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
erpc_transport_t erpc_transport_rpmsg_lite_rtos_remote_init(uint32_t src_addr, uint32_t dst_addr, void *start_address,
                                                            uint32_t rpmsg_link_id, rpmsg_ready_cb ready,
                                                            char *nameservice_name);

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
erpc_transport_t erpc_transport_rpmsg_lite_tty_rtos_remote_init(uint32_t src_addr, uint32_t dst_addr,
                                                                void *start_address, uint32_t rpmsg_link_id,
                                                                rpmsg_ready_cb ready, char *nameservice_name);

/*!
 * @brief Deinitialize an RPMSG lite tty rtos transport.
 *
 * This function deinitializes the RPMSG lite tty rtos transport.
 */
void erpc_transport_rpmsg_lite_tty_rtos_deinit(void);
//@}

//! @name Linux RPMSG endpoint setup
//@{

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
 * @param[in] type Datagram (0) or Stream (1).
 * @param[in] remote_addr Remote endpoint address.
 *
 * @return Return NULL or erpc_transport_t instance pointer.
 */
erpc_transport_t erpc_transport_rpmsg_linux_init(int16_t local_addr, int8_t type, int16_t remote_addr);

/*!
 * @brief Deinitialize an Linux RPMSG endpoint transport.
 *
 * This function deinitializes the Linux RPMSG endpoint transport.
 */
void erpc_transport_rpmsg_linux_deinit(void);
//@}

//! @name TCP transport setup
//@{
	
/*!
 * @brief Create and open TCP transport
 *
 * For server, create a TCP listen socket and wait for connections
 * For client, connect to server
 *
 * @param[in] host hostname/IP address to listen on or server to connect to
 * @param[in] port port to listen on or server to connect to
 * @param[in] isServer true if we are a server
 *
 * @return Return NULL or erpc_transport_t instance pointer.
 */
erpc_transport_t erpc_transport_tcp_init(const char *host, uint16_t port, bool isServer);

/*!
 * @brief Close TCP connection
 *
 * For server, stop listening and close all sockets. Note that the server mode 
 * uses and accept() which is a not-recommended blocking method so we can't exit
 * until a connection attempts is made. This is a deadlock but assuming that TCP
 * code is supposed to be for test, I assume it's acceptable. Otherwise a non-blocking
 * socket or select() shoudl be used 
 * For client, close server connection
 *
 * @return Return TRUE if listen/connection successful
 */
void erpc_transport_tcp_close(void);
//@}

#ifdef __cplusplus
}
#endif

/*! @} */

#endif // _ERPC_TRANSPORT_SETUP_H_
