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
    int fd = 0;
    char sysgpio[64];
    int ret;

    /* Check if the gpio has already been exported */
    ret = sprintf(sysgpio, "/sys/class/gpio/gpio%d/value", gpio);
    if (ret >= 0)
    {
        fd = open(sysgpio, O_WRONLY);
        if (-1 != fd)
        {
            close(fd);
            ret = ERPC_SYSGPIO_STATUS_SUCCESS;
        }
    }

    if ((fd == -1) && (ret >= 0))
    {
        fd = open("/sys/class/gpio/export", O_WRONLY | O_SYNC);
        if (-1 == fd)
        {
            (void)fprintf(stderr, "Could not open gpio export file (%d).\r\n", errno);
            ret = -1;
        }
        else
        {
            ret = sprintf(sysgpio, "%d", gpio);
            if (ret >= 0)
            {
                if (strlen(sysgpio) != write(fd, sysgpio, strlen(sysgpio)))
                {
                    (void)fprintf(stderr, "Could not export gpio (%d) (%d).\r\n", gpio, errno);
                    ret = -2;
                }
                else
                {
                    ret = ERPC_SYSGPIO_STATUS_SUCCESS;
                }
            }
            close(fd);
        }
    }

    return ret;
}

int gpio_direction(int gpio, int direction)
{
    int fd;
    char sysgpio[64];
    int ret;

    ret = sprintf(sysgpio, "/sys/class/gpio/gpio%d/direction", gpio);
    if (ret >= 0)
    {
        ret = ERPC_SYSGPIO_STATUS_SUCCESS;
        fd = open(sysgpio, O_WRONLY | O_SYNC);
        if (-1 == fd)
        {
            (void)fprintf(stderr, "Could not open gpio (%d) direction file (%d).\r\n", gpio, errno);
            ret = -1;
        }
        else
        {
            if (0 == direction)
            {
                if (3 != write(fd, "out", 3))
                {
                    (void)fprintf(stderr, "Could not set gpio (%d) direction to out (low) (%d).\r\n", gpio, errno);
                    ret = -2;
                }
            }
            else if (1 == direction)
            {
                if (2 != write(fd, "in", 2))
                {
                    (void)fprintf(stderr, "Could not set gpio (%d) direction to in (%d).\r\n", gpio, errno);
                    ret = -3;
                }
            }
            else if (2 == direction)
            {
                if (4 != write(fd, "high", 4))
                {
                    (void)fprintf(stderr, "Could not set gpio (%d) direction to out (high) (%d).\r\n", gpio, errno);
                    ret = -4;
                }
            }

            close(fd);
        }
    }

    return ret;
}

int gpio_set_edge(int gpio, char *edge)
{
    int fd;
    char sysgpio[64];
    unsigned char len;
    int ret;

    ret = sprintf(sysgpio, "/sys/class/gpio/gpio%d/edge", gpio);
    if (ret >= 0)
    {
        fd = open(sysgpio, O_WRONLY | O_SYNC);
        if (-1 == fd)
        {
            (void)fprintf(stderr, "Could not open gpio (%d) edge file (%d).\r\n", gpio, errno);
            ret = -1;
        }
        else
        {
            len = strlen(edge) + 1U;
            if (len != write(fd, edge, len))
            {
                (void)fprintf(stderr, "Could not set gpio (%d) edge (%d).\r\n", gpio, errno);
                ret = -2;
            }
            else
            {
                ret = ERPC_SYSGPIO_STATUS_SUCCESS;
            }

            close(fd);
        }
    }

    return ret;
}

int gpio_read(int gpio)
{
    int fd;
    char sysgpio[64];
    char in[2];
    int ret;

    ret = sprintf(sysgpio, "/sys/class/gpio/gpio%d/value", gpio);
    if (ret >= 0)
    {
        fd = open(sysgpio, O_RDWR | O_SYNC);
        if (-1 == fd)
        {
            (void)fprintf(stderr, "Could not open gpio (%d) value file (%d).\r\n", gpio, errno);
            ret = -1;
        }
        else
        {
            if (1 != read(fd, &in, 1))
            {
                (void)fprintf(stderr, "Could not read gpio (%d) value (%d).\r\n", gpio, errno);
                ret = -2;
            }
            else
            {
                in[1] = '\0';
                ret = atoi(&in);
            }

            close(fd);
        }
    }

    return ret;
}

int gpio_open(int gpio)
{
    int fd;
    char sysgpio[64];
    char in;
    int ret;

    ret = sprintf(sysgpio, "/sys/class/gpio/gpio%d/value", gpio);
    if (ret >= 0)
    {
        fd = open(sysgpio, O_RDWR | O_SYNC);
        if (-1 == fd)
        {
            (void)fprintf(stderr, "Could not open gpio (%d) value file (%d).\r\n", gpio, errno);
            ret = -1;
        }
        else
        {
            /* perform a dummy read to clean the events */
            if (1 != read(fd, &in, 1))
            {
                (void)fprintf(stderr, "Could not read gpio (%d) value (%d).\r\n", gpio, errno);
                close(fd);
                ret = -2;
            }
            else
            {
                ret = fd;
            }
        }
    }

    return ret;
}

int gpio_close(int fd)
{
    return close(fd);
}

int gpio_poll(int fd, int timeout)
{
    int pollr;
    struct pollfd fds;
    int ret;
    char val;

    fds.fd = fd;
    fds.events = POLLPRI | POLLERR;
    ;
    pollr = poll(&fds, 1, timeout);

    if (pollr < 0)
    {
        (void)fprintf(stderr, "Could not poll gpio (%d).\r\n", errno);
        ret = -1;
    }
    else
    {
        if ((fds.revents & POLLPRI) > 0)
        {
            /* perform a dummy read to clean the events */
            lseek(fds.fd, 0, SEEK_SET);
            read(fds.fd, &val, 1);
            ret = 1;
        }
        else if ((fds.revents & POLLERR) > 0)
        {
            (void)fprintf(stderr, "Error while polling gpio (%d).\r\n", errno);
            ret = -2;
        }
        else
        {
            ret = ERPC_SYSGPIO_STATUS_SUCCESS;
        }
    }

    return ret;
}
