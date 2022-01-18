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

#elif defined(STM32F446xx)
#include "stm32f4xx_hal.h"
#else

#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
//#include <termios.h>
#include <unistd.h>

#endif

typedef int speed_t;
/*!
 * @addtogroup port_serial
 * @{
 * @file
 */

#if __cplusplus
extern "C" {
#endif



#if defined(STM32F446xx)
int serial_open(USART_TypeDef  *Instance, UART_HandleTypeDef *huart);
HAL_StatusTypeDef serial_close(UART_HandleTypeDef *huart);
HAL_StatusTypeDef serial_write(UART_HandleTypeDef *huart, char *buf, int size);
HAL_StatusTypeDef serial_read(UART_HandleTypeDef *huart, char *buf, int size);
HAL_StatusTypeDef serial_setup(UART_HandleTypeDef *huart, speed_t speed);
#else
int serial_open(const char *port);
int serial_close(int fd);
int serial_setup(int fd, speed_t speed);
int serial_set_read_timeout(int fd, uint8_t vtime, uint8_t vmin);
int serial_write(int fd, char *buf, int size);
int serial_read(int fd, char *buf, int size);
#endif



#if __cplusplus
}
#endif

/*! @} */

#endif
