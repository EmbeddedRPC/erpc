/*
 * The Clear BSD License
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted (subject to the limitations in the disclaimer below) provided
 * that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _ERPC_CONFIG_H_
#define _ERPC_CONFIG_H_

////////////////////////////////////////////////////////////////////////////////
// Declarations
////////////////////////////////////////////////////////////////////////////////

//! @name Threading model options
//@{
#define ERPC_THREADS_NONE (0)     //!< No threads.
#define ERPC_THREADS_PTHREADS (1) //!< POSIX pthreads.
#define ERPC_THREADS_FREERTOS (2) //!< FreeRTOS.

#define ERPC_NOEXCEPT_DISABLED (0) //!< Disabling noexcept feature.
#define ERPC_NOEXCEPT_ENABLED (1)  //!<  Enabling noexcept feature.

#define ERPC_NESTED_CALLS_DISABLED (0) //!< No nested calls support.
#define ERPC_NESTED_CALLS_ENABLED (1)  //!< Nested calls support.

#define ERPC_NESTED_CALLS_DETECTION_DISABLED (0) //!< Nested calls detection disabled.
#define ERPC_NESTED_CALLS_DETECTION_ENABLED (1)  //!< Nested calls detection enabled.

#define ERPC_MESSAGE_LOGGING_DISABLED (0) //!< Trace functions disabled.
#define ERPC_MESSAGE_LOGGING_ENABLED (1)  //!< Trace functions enabled.
//@}

//! @name Configuration options
//@{

//! @def ERPC_THREADS
//!
//! @brief Select threading model.
//!
//! Set to one of the @c ERPC_THREADS_x macros to specify the threading model used by eRPC.
//!
//! Leave commented out to attempt to auto-detect. Auto-detection works well for pthreads.
//! FreeRTOS can be detected when building with compilers that support __has_include().
//! Otherwise, the default is no threading.
//#define ERPC_THREADS (ERPC_THREADS_FREERTOS)

//! @def ERPC_DEFAULT_BUFFER_SIZE
//!
//! Uncomment to change the size of buffers allocated by BasicMessageBufferFactory in the client
//! and server setup functions (@ref client_setup and @ref server_setup). The default size is 256.
#define ERPC_DEFAULT_BUFFER_SIZE (1024)

//! @def ERPC_DEFAULT_BUFFERS_COUNT
//!
//! Uncomment to change the count of buffers allocated by StaticMessageBufferFactory
//#define ERPC_DEFAULT_BUFFERS_COUNT (2)

//! @def ERPC_NOEXCEPT
//!
//! @brief Disable/enable noexcept support.
//!
//! Uncomment for using noexcept feature.
//#define ERPC_NOEXCEPT (ERPC_NOEXCEPT_ENABLED)

//! @def ERPC_NESTED_CALLS
//!
//! Default set to ERPC_NESTED_CALLS_DISABLED. Uncomment when callbacks, or other eRPC
//! functions are called from server implementation of another eRPC call. Nested functions
//! need to be marked as @nested in IDL.
//#define ERPC_NESTED_CALLS (ERPC_NESTED_CALLS_ENABLED)

//! @def ERPC_NESTED_CALLS_DETECTION
//!
//! Default set to ERPC_NESTED_CALLS_DETECTION_ENABLED when NDEBUG macro is presented.
//! This serve for locating nested calls in code. Nested calls are calls where inside eRPC function
//! on server side is called another eRPC function (like callbacks). Code need be a bit changed
//! to support nested calls. See ERPC_NESTED_CALLS macro.
//#define ERPC_NESTED_CALLS_DETECTION (ERPC_NESTED_CALLS_DETECTION_DISABLED)

//! @def ERPC_MESSAGE_LOGGING
//!
//! Enable eRPC message logging code through the eRPC. Take look into "message_logging.h". Can be used for base printing
//! messages, or sending data to another system for data analysis. Default set to ERPC_MESSAGE_LOGGING_DISABLED.
//!
//! Uncomment for using logging feature.
//#define ERPC_MESSAGE_LOGGING (ERPC_MESSAGE_LOGGING_ENABLED)

//@}

/*! @} */
#endif // _ERPC_CONFIG_H_
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
