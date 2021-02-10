/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _ERPC_SYSGPIO_H_
#define _ERPC_SYSGPIO_H_

#if __cplusplus
extern "C" {
#endif

#define ERPC_SYSGPIO_STATUS_SUCCESS 0

int gpio_export(int gpio);
int gpio_direction(int gpio, int direction);
int gpio_set_edge(int gpio, char *edge);
int gpio_read(int gpio);

int gpio_open(int gpio);
int gpio_close(int fd);
int gpio_poll(int fd, int timeout);

#if __cplusplus
}
#endif

#endif /* _ERPC_SYSGPIO_H_ */
