/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_sysgpio.h"

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int gpio_export(int gpio)
{
    int fd;
    char sysgpio[64];

    /* Check if the gpio has already been exported */
    sprintf(sysgpio, "/sys/class/gpio/gpio%d/value", gpio);
    fd = open(sysgpio, O_WRONLY);
    if (-1 != fd)
    {
        close(fd);
        return ERPC_SYSGPIO_STATUS_SUCCESS;
    }

    fd = open("/sys/class/gpio/export", O_WRONLY | O_SYNC);
    if (-1 == fd)
    {
        fprintf(stderr, "Could not open gpio export file (%d).\r\n", errno);
        return -1;
    }
    else
    {
        sprintf(sysgpio, "%d", gpio);
        if (strlen(sysgpio) != write(fd, sysgpio, strlen(sysgpio)))
        {
            fprintf(stderr, "Could not export gpio (%d) (%d).\r\n", gpio, errno);
            close(fd);
            return -2;
        }
    }

    close(fd);
    return ERPC_SYSGPIO_STATUS_SUCCESS;
}

int gpio_direction(int gpio, int direction)
{
    int fd;
    char sysgpio[64];
    int ret = ERPC_SYSGPIO_STATUS_SUCCESS;

    sprintf(sysgpio, "/sys/class/gpio/gpio%d/direction", gpio);
    fd = open(sysgpio, O_WRONLY | O_SYNC);
    if (-1 == fd)
    {
        fprintf(stderr, "Could not open gpio (%d) direction file (%d).\r\n", gpio, errno);
        return -1;
    }

    else if (0 == direction)
    {
        if (3 != write(fd, "out", 3))
        {
            fprintf(stderr, "Could not set gpio (%d) direction to out (low) (%d).\r\n", gpio, errno);
            ret = -2;
        }
    }
    else if (1 == direction)
    {
        if (2 != write(fd, "in", 2))
        {
            fprintf(stderr, "Could not set gpio (%d) direction to in (%d).\r\n", gpio, errno);
            ret = -3;
        }
    }
    else if (2 == direction)
    {
        if (4 != write(fd, "high", 4))
        {
            fprintf(stderr, "Could not set gpio (%d) direction to out (high) (%d).\r\n", gpio, errno);
            ret = -4;
        }
    }

    close(fd);
    return ret;
}

int gpio_set_edge(int gpio, char *edge)
{
    int fd;
    char sysgpio[64];
    unsigned char len = 0;

    sprintf(sysgpio, "/sys/class/gpio/gpio%d/edge", gpio);
    fd = open(sysgpio, O_WRONLY | O_SYNC);
    if (-1 == fd)
    {
        fprintf(stderr, "Could not open gpio (%d) edge file (%d).\r\n", gpio, errno);
        return -1;
    }

    len = strlen(edge) + 1;
    if (len != write(fd, edge, len))
    {
        fprintf(stderr, "Could not set gpio (%d) edge (%d).\r\n", gpio, errno);
        close(fd);
        return -2;
    }

    close(fd);
    return ERPC_SYSGPIO_STATUS_SUCCESS;
}

int gpio_read(int gpio)
{
    int fd;
    char sysgpio[64];
    char in;

    sprintf(sysgpio, "/sys/class/gpio/gpio%d/value", gpio);
    fd = open(sysgpio, O_RDWR | O_SYNC);
    if (-1 == fd)
    {
        fprintf(stderr, "Could not open gpio (%d) value file (%d).\r\n", gpio, errno);
        return -1;
    }

    if (1 != read(fd, &in, 1))
    {
        fprintf(stderr, "Could not read gpio (%d) value (%d).\r\n", gpio, errno);
        close(fd);
        return -2;
    }

    close(fd);
    return atoi(&in);
}

int gpio_open(int gpio)
{
    int fd;
    char sysgpio[64];
    char in;

    sprintf(sysgpio, "/sys/class/gpio/gpio%d/value", gpio);
    fd = open(sysgpio, O_RDWR | O_SYNC);
    if (-1 == fd)
    {
        fprintf(stderr, "Could not open gpio (%d) value file (%d).\r\n", gpio, errno);
        return -1;
    }

    /* perform a dummy read to clean the events */
    if (1 != read(fd, &in, 1))
    {
        fprintf(stderr, "Could not read gpio (%d) value (%d).\r\n", gpio, errno);
        close(fd);
        return -2;
    }

    return fd;
}

int gpio_close(int fd)
{
    return close(fd);
}

int gpio_poll(int fd, int timeout)
{
    int pollr;
    struct pollfd fds;

    fds.fd = fd;
    fds.events = POLLPRI | POLLERR;
    ;
    pollr = poll(&fds, 1, timeout);

    if (pollr < 0)
    {
        fprintf(stderr, "Could not poll gpio (%d).\r\n", errno);
        return -1;
    }

    if (fds.revents & POLLPRI)
    {
        /* perform a dummy read to clean the events */
        char val;
        lseek(fds.fd, 0, SEEK_SET);
        read(fds.fd, &val, 1);
        return 1;
    }
    else if (fds.revents & POLLERR)
    {
        fprintf(stderr, "Error while polling gpio (%d).\r\n", errno);
        return -2;
    }

    return ERPC_SYSGPIO_STATUS_SUCCESS;
}
