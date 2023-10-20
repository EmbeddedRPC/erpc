/*
 * Copyright 2017-2022 NXP
 * Copyright (C) ARM Limited, 2006.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* This code is for Keil uVision only to retarget missing C++ streamed I/O.
   Without this code 'bkpt 0xab' instructions are being placed into the
   application code and the run button in the uVision has to be pressed
   3x-4x before the application starts running.
   This is because of semihosting as described here:
   http://www.keil.com/support/docs/3614.htm
   We can't use the microlib in erpc test project as the microlib does not
   support C++.
   We can't use the #pragma import(__use_no_semihosting_swi) as well due to
   linker issues.
   We decided to implement the retarget code in addition to the basic retarget
   code implemented in fsl_debug_console.c. The missing retarget code has been
   implemented based on this article:
   http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.faqs/ka3844.html
   This code is used just in Keil erpc unit test projects for supported
   MCUXpressoSDK dualcore boards (embedded side).
  */

#if defined(__CC_ARM) || defined(__ARMCC_VERSION)

//#include "fsl_log.h"
#include "fsl_debug_console.h"

//#pragma import(__use_no_semihosting_swi)
#include <rt_sys.h>

#define DEFAULT_HANDLE 0x100;

char *_sys_command_string(char *cmd, int len)
{
    return (cmd);
}
/*
 * These names are special strings which will be recognized by
 * _sys_open and will cause it to return the standard I/O handles, instead
 * of opening a real file.
 */
const char __stdin_name[] = "STDIN";
const char __stdout_name[] = "STDOUT";
const char __stderr_name[] = "STDERR";

/*
 * Open a file. May return -1 if the file failed to open. We do not require
 * this function to do anything. Simply return a dummy handle.
 */
FILEHANDLE _sys_open(const char *name, int openmode)
{
    return DEFAULT_HANDLE;
}

/*
 * Close a file. Should return 0 on success or a negative value on error.
 * Not required in this implementation. Always return success.
 */
int _sys_close(FILEHANDLE fh)
{
    return 0; // return success
}

/*
 * Write to a file. Returns 0 on success, negative on error, and the number
 * of characters _not_ written on partial success. This implementation sends
 * a buffer of size 'len' to the UART.
 */
int _sys_write(FILEHANDLE fh, const unsigned char *buf, unsigned len, int mode)
{
    int i;
    for (i = 0; i < len; i++)
    {
        // UART_write(buf[i]);
        // LOG_Push((uint8_t *)(&buf[i]), 1);
        DbgConsole_Putchar(buf[i]);
    }

    //DbgConsole_SendData((uint8_t *)buf, len);

    return 0;
}

/*
 * Read from a file. Can return:
 *  - zero if the read was completely successful
 *  - the number of bytes _not_ read, if the read was partially successful
 *  - the number of bytes not read, plus the top bit set (0x80000000), if
 *    the read was partially successful due to end of file
 *  - -1 if some error other than EOF occurred
 * This function receives a character from the UART, processes the character
 * if required (backspace) and then echo the character to the Terminal
 * Emulator, printing the correct sequence after successive keystrokes.
 */
int _sys_read(FILEHANDLE fh, unsigned char *buf, unsigned len, int mode)
{
    int pos = 0;

    do
    {

        // buf[pos]=UART_read();
        // LOG_ReadCharacter((uint8_t *)&buf[pos]);
        // DbgConsole_ReadCharacter((uint8_t *)&buf[pos]);
        buf[pos] = DbgConsole_Getchar();

        // Advance position in buffer
        pos++;

        // Handle backspace
        if (buf[pos - 1] == '\b')
        {
            // More than 1 char in buffer
            if (pos > 1)
            {
                // Delete character on terminal
                // UART_write('\b');
                // UART_write(' ');
                // UART_write('\b');

                // Update position in buffer
                pos -= 2;
            }
            else if (pos > 0)
                pos--; // Backspace pressed, empty buffer
        }
        // else UART_write(buf[pos-1]); // Echo normal char to terminal
        else
            // LOG_Push((uint8_t *)(&buf[pos - 1]), 1); // Echo normal char to terminal
            // DbgConsole_SendData((uint8_t *)(&buf[pos - 1]), 1); // Echo normal char to terminal
            DbgConsole_Putchar(buf[pos - 1]); // Echo normal char to terminal

    } while (buf[pos - 1] != '\r');

    buf[pos] = '\0'; // Ensure Null termination

    return 0;
}

/*
 * Writes a character to the output channel. This function is used
 * for last-resort error message output.
 */
void _ttywrch(int ch)
{
    // Convert correctly for endianness change
    char ench = ch;

    // UART_write(ench);
    // LOG_Push((uint8_t *)(&ench), 1);
    // DbgConsole_SendData((uint8_t *)(&ench), 1);
    DbgConsole_Putchar(ench);
}
/*
 * Return non-zero if the argument file is connected to a terminal.
 */
int _sys_istty(FILEHANDLE fh)
{
    return 1; // no interactive device present
}

/*
 * Move the file position to a given offset from the file start.
 * Returns >=0 on success, <0 on failure. Seeking is not supported for the
 * UART.
 */
int _sys_seek(FILEHANDLE fh, long pos)
{
    return -1; // error
}

/*
 * Flush any OS buffers associated with fh, ensuring that the file
 * is up to date on disk. Result is >=0 if OK, negative for an
 * error.
 */
int _sys_ensure(FILEHANDLE fh)
{
    return 0; // success
}

/*
 * Return the current length of a file, or <0 if an error occurred.
 * _sys_flen is allowed to reposition the file pointer (so Unix can
 * implement it with a single lseek, for example), since it is only
 * called when processing SEEK_END relative fseeks, and therefore a
 * call to _sys_flen is always followed by a call to _sys_seek.
 */
long _sys_flen(FILEHANDLE fh)
{
    return 0;
}

/*
 * Return the name for temporary file number sig in the buffer
 * name. Returns 0 on failure. maxlen is the maximum name length
 * allowed.
 */
void _sys_tmpnam(char *name, int sig, unsigned maxlen)
{
    return; // fail, not supported
}

/*
 * Terminate the program, passing a return code back to the user.
 * This function may not return.
 */
void _sys_exit(int returncode)
{
    while (1)
    {
    };
}
#endif /* __CC_ARM */
