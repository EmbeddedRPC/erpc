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

#ifdef _WIN32
static OVERLAPPED s_writeOverlap;
static OVERLAPPED s_readOverlap;
#define TX_BUF_BYTES 1024U
#define RX_BUF_BYTES 1024U
#endif

int serial_setup(int fd, speed_t speed)
{
#ifdef _WIN32
    COMMTIMEOUTS timeouts;
    DCB dcb = { 0 };
    HANDLE hCom = (HANDLE)fd;

    DWORD errors;
    COMSTAT status;

    ClearCommError(hCom, &errors, &status);
    PurgeComm(hCom, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);

    memset(&timeouts, 0, sizeof(timeouts));
    timeouts.ReadIntervalTimeout = MAXDWORD;
    timeouts.WriteTotalTimeoutConstant = 500;

    if (!SetCommTimeouts(hCom, &timeouts))
    {
        return -1;
    }

    dcb.DCBlength = sizeof(dcb);
    dcb.BaudRate = 115200;
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    dcb.fBinary = TRUE;
    dcb.fDtrControl = DTR_CONTROL_ENABLE;
    dcb.fRtsControl = RTS_CONTROL_ENABLE;

    if (!SetCommState(hCom, &dcb))
    {
        return -1;
    }

    s_writeOverlap.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    s_readOverlap.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    SetCommMask(hCom, EV_RXCHAR);
#else
    (void)speed;
    struct termios tty;

    memset(&tty, 0x00, sizeof(tty));
    cfmakeraw(&tty);

    tty.c_cflag &= ~(PARENB | CSTOPB | CSIZE);
    tty.c_cflag |= (CS8 | CLOCAL | CREAD);
    tty.c_oflag = 0;
    tty.c_lflag = 0;

#ifdef __linux__
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

#ifdef __APPLE__
    return ioctl(fd, IOSSIOSPEED, &speed);
#endif // #ifdef __APPLE__

#endif // _WIN32
    return 0;
}

int serial_set_read_timeout(int fd, uint8_t vtime, uint8_t vmin)
{
#ifdef _WIN32
    // TODO
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

#endif // _WIN32
    return 0;
}

int serial_write(int fd, const char *buf, int size)
{
#ifdef _WIN32
    HANDLE hCom = (HANDLE)fd;
    DWORD errors;
    COMSTAT status;
    unsigned long bwritten = 0;

    ClearCommError(hCom, &errors, &status);
    if (!WriteFile(hCom, buf, size, &bwritten, &s_writeOverlap))
    {
        if (GetLastError() == ERROR_IO_PENDING)
        {
            if (!GetOverlappedResult(hCom, &s_writeOverlap, &bwritten, TRUE))
            {
                return 0;
            }
        }
        else
        {
            return 0;
            ;
        }
    }
    ClearCommError(hCom, &errors, &status);

    return bwritten;
#else
    return write(fd, buf, size);
#endif
}

int serial_read(int fd, char *buf, int size)
{
#ifdef _WIN32
    HANDLE hCom = (HANDLE)fd;
    char temp[RX_BUF_BYTES] = { 0 };
    DWORD errors;
    DWORD bytesToRead = 0;
    DWORD bytesRead = 0;
    DWORD ret = 0;

    while (bytesToRead != size)
    {
        do
        {

            ClearCommError(hCom, &errors, NULL);

            if (!ReadFile(hCom, temp, size - bytesToRead, &bytesRead, &s_readOverlap))
            {
                if (GetLastError() == ERROR_IO_PENDING)
                {
                    ret = WaitForSingleObject(s_readOverlap.hEvent, INFINITE);

                    if (WAIT_OBJECT_0 == ret)
                    {
                        if (!GetOverlappedResult(hCom, &s_readOverlap, &bytesRead, FALSE))
                        {
                            bytesRead = 0;
                            bytesToRead = 0;
                        }
                    }
                    else
                    {
                        bytesRead = 0;
                        bytesToRead = 0;
                    }
                }
                else
                {
                    bytesRead = 0;
                    bytesToRead = 0;
                }
            }

            bytesToRead += bytesRead;

            if (bytesRead)
            {
                (void)memcpy(buf, temp, bytesRead);
                buf += bytesRead;
            }

        } while ((bytesRead > 0) && (RX_BUF_BYTES >= bytesToRead));
    }

    return bytesToRead;
#else
    int len = 0;
    int ret;
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
#ifdef _WIN32
    static char full_path[32] = { 0 };

    HANDLE hCom = NULL;

    if (memcmp(port, "\\\\.\\", 4))
    {
        _snprintf(full_path, sizeof(full_path) - 1, "\\\\.\\%s", port);
    }
    else
    {
        (void)memcpy(full_path, port, strnlen_s(port, sizeof(full_path) - 1));
    }

    hCom = CreateFileA(full_path, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

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
#ifdef _WIN32
    HANDLE hCom = (HANDLE)fd;

    CloseHandle(hCom);
#else
    close(fd);
#endif
    return 0;
}
