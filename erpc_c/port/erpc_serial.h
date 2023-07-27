/*
 * This file is part of the Bus Pirate project (http://code.google.com/p/the-bus-pirate/).
 *
 * Written and maintained by the Bus Pirate project and http://dangerousprototypes.com
 *
 * To the extent possible under law, the project has
 * waived all copyright and related or neighboring rights to Bus Pirate. This
 * work is published from United States.
 *
 * For details see: http://creativecommons.org/publicdomain/zero/1.0/.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
/*
 * OS independent serial interface
 *
 * Heavily based on Pirate-Loader:
 * http://the-bus-pirate.googlecode.com/svn/trunk/bootloader-v4/pirate-loader/source/pirate-loader.c
 *
 */
#ifndef MYSERIAL_H_
#define MYSERIAL_H_

#ifdef __APPLE__
#include <IOKit/serial/ioss.h>
#include <sys/ioctl.h>

#define B1500000 1500000
#define B1000000 1000000
#define B921600 921600
#endif

#include <stdint.h>

#ifdef _WIN32
#include <time.h>
#include <windows.h>

#define B115200 115200
#define B921600 921600

typedef long speed_t;
#else

#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#endif

/*!
 * @addtogroup port_serial
 * @{
 * @file
 */

#if __cplusplus
extern "C" {
#endif

int serial_setup(int fd, speed_t speed);
int serial_set_read_timeout(int fd, uint8_t vtime, uint8_t vmin);
int serial_write(int fd, const char *buf, int size);
int serial_read(int fd, char *buf, int size);
int serial_open(const char *port);
int serial_close(int fd);

#if __cplusplus
}
#endif

/*! @} */

#endif
