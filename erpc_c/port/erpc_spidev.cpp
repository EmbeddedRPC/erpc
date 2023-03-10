/*
 * Copyright 2020 NXP
 * Copyright 2021 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_spidev.h"

#include <errno.h>
#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

int spidev_open(const char *port)
{
    int fd;

    fd = open(port, O_RDWR);
    if (-1 == fd)
    {
        (void)fprintf(stderr, "Could not open spidev port (%d).\r\n", errno);
    }

    return fd;
}

int spidev_set_mode(int fd, unsigned char mode)
{
    int retVal = ERPC_SPIDEV_STATUS_SUCCESS;

    if (-1 == ioctl(fd, SPI_IOC_WR_MODE, &mode))
    {
        (void)fprintf(stderr, "Could not set mode for spidev port (%d).\r\n", errno);
        retVal = ERPC_SPIDEV_STATUS_FAIL;
    }

    return retVal;
}

int spidev_set_speed(int fd, unsigned int speed_hz)
{
    int retVal = ERPC_SPIDEV_STATUS_SUCCESS;

    if (-1 == ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed_hz))
    {
        (void)fprintf(stderr, "Could not set max speed for spidev port (%d).\r\n", errno);
        retVal = ERPC_SPIDEV_STATUS_FAIL;
    }

    return retVal;
}

int spidev_set_wordbits(int fd, unsigned char bits)
{
    int retVal = ERPC_SPIDEV_STATUS_SUCCESS;

    if (-1 == ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits))
    {
        (void)fprintf(stderr, "Could not set bits per word for spidev port (%d).\r\n", errno);
        retVal = ERPC_SPIDEV_STATUS_FAIL;
    }

    return retVal;
}

int spidev_transfer(int fd, const unsigned char *tx_buf, unsigned char *rx_buf, unsigned int len)
{
    int retVal = ERPC_SPIDEV_STATUS_SUCCESS;
    struct spi_ioc_transfer spi_message;

    (void)memset(&spi_message, 0, sizeof(spi_message));
    spi_message.tx_buf = (unsigned long)tx_buf;
    spi_message.rx_buf = (unsigned long)rx_buf;
    spi_message.len = len;

    if (-1 == ioctl(fd, SPI_IOC_MESSAGE(1), &spi_message))
    {
        (void)fprintf(stderr, "Could not transfer data on spidev port (%d).\r\n", errno);
        retVal = ERPC_SPIDEV_STATUS_FAIL;
    }

    return retVal;
}

int spidev_close(int fd)
{
    close(fd);
    return ERPC_SPIDEV_STATUS_SUCCESS;
}
