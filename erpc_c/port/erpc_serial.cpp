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

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
//#include <unistd.h>
#include <errno.h>
#include <string.h>
/*included this here for cygwin. need to figure out proper include handling for cygwin */
//#include <termios.h>

//#define LINUX 1
//#ifdef LINUX
//#include <termios.h>
//#endif
// included from serial.h

#include "erpc_serial.h"

int serial_setup(int fd, speed_t speed)
{
#ifdef WIN32
    COMMTIMEOUTS timeouts;
    DCB dcb = { 0 };
    HANDLE hCom = (HANDLE)fd;

    dcb.DCBlength = sizeof(dcb);

    dcb.BaudRate = speed;
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;

    if (!SetCommState(hCom, &dcb))
    {
        return -1;
    }

    // These timeouts mean:
    // read: return immediately with whatever data is available, if any
    // write: timeouts not used
    // reference: http://www.robbayer.com/files/serial-win.pdf
    timeouts.ReadIntervalTimeout = MAXDWORD;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.ReadTotalTimeoutConstant = 0;
    timeouts.WriteTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = 0;

    if (!SetCommTimeouts(hCom, &timeouts))
    {
        return -1;
    }
#else
    (void)speed;
    struct termios tty;

    memset(&tty, 0x00, sizeof(tty));
    cfmakeraw(&tty);

    tty.c_cflag &= ~(PARENB | CSTOPB | CSIZE);
    tty.c_cflag |= (CS8 | CLOCAL | CREAD);
    tty.c_oflag = 0;
    tty.c_lflag = 0;

#ifdef LINUX
    switch (speed)
    {
        case 9600:
            speed = B9600;
            break;
        case 38400:
            speed = B38400;
            break;
        case 115200:
            speed = B115200;
            break;
        case 57600:
        default:
            speed = B57600;
            break;
    }

    cfsetospeed(&tty, speed);
    cfsetispeed(&tty, speed);
#endif

    // Completely non-blocking read
    // VMIN = 0 and VTIME = 0
    // Completely non-blocking read
    // reference: http://www.unixwiz.net/techtips/termios-vmin-vtime.html

    // currently set as total blocking - must see at least 1 bit
    tty.c_cc[VTIME] = 0;
    tty.c_cc[VMIN] = 0;

    if (tcsetattr(fd, TCSAFLUSH, &tty) < 0)
    {
        return -1;
    }

#ifdef MACOSX
    return ioctl(fd, IOSSIOSPEED, &speed);
#endif //#ifdef MACOSX

#endif // WIN32
    return 0;
}

int serial_set_read_timeout(int fd, uint8_t vtime, uint8_t vmin)
{
#ifdef WIN32
    COMMTIMEOUTS timeouts;
    HANDLE hCom = (HANDLE)fd;

    // These timeouts mean:
    // read: return if:
    //  1. Inter-character timeout exceeds ReadIntervalTimeout
    //  2. Total timeout exceeds (ReadIntervalTimeout*ReadTotalTimeoutMultiplier*number of characters) +
    //  ReadTotalTimeoutConstant
    // In practice it seems that no matter how many characters you ask for, if no characters appear on the interface
    // then
    // only ReadTotalTimeoutConstant applies.
    // write: timeouts not used
    // reference: http://www.robbayer.com/files/serial-win.pdf
    if (timeoutMs != 0)
    {
        timeouts.ReadIntervalTimeout = 1000;
        timeouts.ReadTotalTimeoutMultiplier = 10;
        timeouts.ReadTotalTimeoutConstant = timeoutMs;
        timeouts.WriteTotalTimeoutMultiplier = 0;
        timeouts.WriteTotalTimeoutConstant = 0;
    }
    else
    {
        // Need a separate case for timeoutMs == 0
        // setting all these values to 0 results in no timeout
        // so set them to a minimum value, this will return immediately
        // if there is no data available
        timeouts.ReadIntervalTimeout = 1;
        timeouts.ReadTotalTimeoutMultiplier = 1;
        timeouts.ReadTotalTimeoutConstant = 1;
        timeouts.WriteTotalTimeoutMultiplier = 0;
        timeouts.WriteTotalTimeoutConstant = 0;
    }

    if (!SetCommTimeouts(hCom, &timeouts))
    {
        return -1;
    }

#else
    struct termios tty;
    /*memset(&tty, 0x00, sizeof(tty));
    cfmakeraw(&tty); */
    tcgetattr(fd, &tty);

    /************************************************************
     * Timeout Options:
     * VMIN = 0; VTIME = 0 : completely non-blocking
     * VMIN > 0; VTIME = 0 : pure blocking until VMIN bytes read
     * VMIN = 0; VTIME > 0 : pure timed read
     * VMIN > 0; VTIME > 0 : interbyte timeout;
     ************************************************************/
    // NOTE: VTIME is in 0.1 second intervals
    tty.c_cc[VTIME] = vtime;
    tty.c_cc[VMIN] = vmin;

    if (tcsetattr(fd, TCSAFLUSH, &tty) < 0)
    {
        return -1;
    }

#endif // WIN32
    return 0;
}

int serial_write(int fd, char *buf, int size)
{
#ifdef WIN32
    HANDLE hCom = (HANDLE)fd;
    unsigned long bwritten = 0;

    if (!WriteFile(hCom, buf, size, &bwritten, NULL))
    {
        return 0;
    }
    else
    {
        return bwritten;
    }
#else
    return write(fd, buf, size);
#endif
}

int serial_read(int fd, char *buf, int size)
{
#ifdef WIN32
    HANDLE hCom = (HANDLE)fd;
    unsigned long bread = 0;

    if (!ReadFile(hCom, buf, size, &bread, NULL))
    {
        return 0;
    }
    else
    {
        return bread;
    }
#else
    int len = 0;
    int ret = 0;
    int timeout = 0;

    while (len < size)
    {
        ret = read(fd, buf + len, size - len);
        if (ret == -1)
        {
            return -1;
        }

        if (ret == 0)
        {
            timeout++;

            if (timeout >= 10)
            {
                break;
            }

            continue;
        }

        len += ret;
    }

    return len;
#endif
}

int serial_open(const char *port)
{
    int fd;
#ifdef WIN32
    static char full_path[32] = { 0 };

    HANDLE hCom = NULL;

    if (port[0] != '\\')
    {
        _snprintf(full_path, sizeof(full_path) - 1, "\\\\.\\%s", port);
        port = full_path;
    }

#pragma warning(suppress : 6053)
    hCom = CreateFileA(port, GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (!hCom || hCom == INVALID_HANDLE_VALUE)
    {
        fd = -1;
    }
    else
    {
        fd = (int)hCom;
    }
#else
    fd = open(port, O_RDWR | O_NOCTTY);
    if (fd == -1)
    {
        fprintf(stderr, "Could not open serial port.");
        return -1;
    }
#endif
    return fd;
}

int serial_close(int fd)
{
#ifdef WIN32
    HANDLE hCom = (HANDLE)fd;

    CloseHandle(hCom);
#else
    close(fd);
#endif
    return 0;
}
