/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _ERPC_SPIDEV_H_
#define _ERPC_SPIDEV_H_

#if __cplusplus
extern "C" {
#endif

#define ERPC_SPIDEV_STATUS_SUCCESS 0
#define ERPC_SPIDEV_STATUS_FAIL -1

int spidev_open(const char *port);
int spidev_set_mode(int fd, unsigned char mode);
int spidev_set_speed(int fd, unsigned int speed_hz);
int spidev_set_wordbits(int fd, unsigned char bits);
int spidev_transfer(int fd, const unsigned char *tx_buf, unsigned char *rx_buf, unsigned int len);
int spidev_close(int fd);

#if __cplusplus
}
#endif

#endif /* _ERPC_SPIDEV_H_ */
