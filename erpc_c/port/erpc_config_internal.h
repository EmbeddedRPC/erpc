/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * Copyright 2020 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _ERPC_DETECT_H_
#define _ERPC_DETECT_H_

#include "erpc_config.h"

////////////////////////////////////////////////////////////////////////////////
// Declarations
////////////////////////////////////////////////////////////////////////////////
/* clang-format off */

// Determine if this is a POSIX system.
#if !defined(ERPC_HAS_POSIX)
    // Detect Linux, BSD, Cygwin, and Mac OS X.
    #if defined(__linux__) || defined(__GNU__) || defined(__FreeBSD__) || defined(__NetBSD__) || \
        defined(__OpenBSD__) || defined(__DragonFly__) || defined(__CYGWIN__) || defined(__MACH__)
        #define ERPC_HAS_POSIX (1)
    #else
        #define ERPC_HAS_POSIX (0)
    #endif
#endif

// Determine if we are targeting WIN32 environment
#if !defined(ERPC_HAS_WIN32)
    #if defined(_WIN32)
        #define ERPC_HAS_WIN32 (1)
    #else
        #define ERPC_HAS_WIN32 (0)
    #endif
#endif

// Safely detect FreeRTOSConfig.h.
#define ERPC_HAS_FREERTOSCONFIG_H (0)
#if defined(__has_include)
    #if __has_include("FreeRTOSConfig.h")
        #undef ERPC_HAS_FREERTOSCONFIG_H
        #define ERPC_HAS_FREERTOSCONFIG_H (1)
    #endif
#endif

// Detect allocation policy if not already set.
#if !defined(ERPC_ALLOCATION_POLICY)
    #if defined(__has_include)
        #if __has_include("FreeRTOSConfig.h")
            #include "FreeRTOSConfig.h"
            #if defined(configSUPPORT_STATIC_ALLOCATION) && configSUPPORT_STATIC_ALLOCATION
                #define ERPC_ALLOCATION_POLICY (ERPC_STATIC_POLICY)
            #else
                #define ERPC_ALLOCATION_POLICY (ERPC_DYNAMIC_POLICY)
            #endif
        #endif
    #endif
#endif

#if ERPC_ALLOCATION_POLICY == ERPC_STATIC_POLICY
#if !defined(ERPC_CODEC_COUNT)
#define ERPC_CODEC_COUNT (2U)
#warning "ERPC_CODEC_COUNT is not defined. Default is used."
#endif
#if !defined(ERPC_MESSAGE_LOGGERS_COUNT)
#define ERPC_MESSAGE_LOGGERS_COUNT (0U)
#warning "ERPC_MESSAGE_LOGGERS_COUNT is not defined. Default is used."
#endif
#if !defined(ERPC_CLIENTS_THREADS_AMOUNT)
#define ERPC_CLIENTS_THREADS_AMOUNT (1U)
#warning "ERPC_CLIENTS_THREADS_AMOUNT is not defined. Default is used."
#endif
#endif

// Detect threading model if not already set.
#if !defined(ERPC_THREADS)
    #if ERPC_HAS_POSIX
        // Default to pthreads for POSIX systems.
        #define ERPC_THREADS (ERPC_THREADS_PTHREADS)
    #elif ERPC_HAS_FREERTOSCONFIG_H
        // Use FreeRTOS if we can auto detect it.
        #define ERPC_THREADS (ERPC_THREADS_FREERTOS)
    #elif ERPC_HAS_WIN32
        #define ERPC_THREADS (ERPC_THREADS_WIN32)
    #else
        // Otherwise default to no threads.
        #define ERPC_THREADS (ERPC_THREADS_NONE)
    #endif
#endif

// Handy macro to test threading model. You can also ERPC_THREADS directly to test for threading
// support, i.e. "#if ERPC_THREADS", because ERPC_THREADS_NONE has a value of 0.
#define ERPC_THREADS_IS(_n_) (ERPC_THREADS == (ERPC_THREADS_##_n_))

// Set default buffer size.
#if !defined(ERPC_DEFAULT_BUFFER_SIZE)
    //! @brief Size of buffers allocated by BasicMessageBufferFactory in setup functions.
    #define ERPC_DEFAULT_BUFFER_SIZE (256U)
#endif

// Set default buffers count.
#if !defined(ERPC_DEFAULT_BUFFERS_COUNT)
    //! @brief Count of buffers allocated by StaticMessageBufferFactory.
    #define ERPC_DEFAULT_BUFFERS_COUNT (2U)
#endif

// Disable/enable noexcept.
#if !defined(ERPC_NOEXCEPT)
    #if ERPC_HAS_POSIX
        #define ERPC_NOEXCEPT (ERPC_NOEXCEPT_ENABLED)
    #else
        #define ERPC_NOEXCEPT (ERPC_NOEXCEPT_DISABLED)
    #endif
#endif

//NOEXCEPT support
#if defined(__cplusplus) && __cplusplus >= 201103 && ERPC_NOEXCEPT
#define NOEXCEPT noexcept
#else
#define NOEXCEPT
#endif // NOEXCEPT

// Disabling nesting calls support as default.
#if !defined(ERPC_NESTED_CALLS)
    #define ERPC_NESTED_CALLS (ERPC_NESTED_CALLS_DISABLED)
#endif

#if ERPC_NESTED_CALLS && !ERPC_THREADS
    #error "Nested calls currently working only with Threads."
#endif

// Enabling nesting calls detection as default for debug.
#if !defined(ERPC_NESTED_CALLS_DETECTION)
    #if defined(NDEBUG) || (ERPC_NESTED_CALLS == ERPC_NESTED_CALLS_ENABLED)
        #define ERPC_NESTED_CALLS_DETECTION (ERPC_NESTED_CALLS_DETECTION_DISABLED)
    #else
        #define ERPC_NESTED_CALLS_DETECTION (ERPC_NESTED_CALLS_DETECTION_ENABLED)
    #endif
#endif

// Disabling tracing the eRPC.
#if !defined(ERPC_MESSAGE_LOGGING)
    #define ERPC_MESSAGE_LOGGING (ERPC_MESSAGE_LOGGING_DISABLED)
#endif

#if defined(__CC_ARM) || defined(__ARMCC_VERSION) /* Keil MDK */
#define THROW_BADALLOC throw(std::bad_alloc)
#define THROW throw()
#else
#define THROW_BADALLOC
#define THROW
#endif

#ifndef ERPC_TRANSPORT_MU_USE_MCMGR
    #if defined(__has_include)
        #if (__has_include("mcmgr.h"))
            #define ERPC_TRANSPORT_MU_USE_MCMGR (ERPC_TRANSPORT_MU_USE_MCMGR_ENABLED)
        #else
            #define ERPC_TRANSPORT_MU_USE_MCMGR (ERPC_TRANSPORT_MU_USE_MCMGR_DISABLED)
        #endif
    #endif
#else
    #if defined(__has_include)
        #if ((!(__has_include("mcmgr.h"))) && (ERPC_TRANSPORT_MU_USE_MCMGR == ERPC_TRANSPORT_MU_USE_MCMGR_ENABLED))
            #error "Do not forget to add the MCMGR library into your project!"
        #endif
    #endif
#endif

// Disabling pre and post callback function related code.
#if !defined(ERPC_PRE_POST_ACTION)
    #define ERPC_PRE_POST_ACTION (ERPC_PRE_POST_ACTION_DISABLED)
#endif

// Disabling pre and post default callback function code.
#if !defined(ERPC_PRE_POST_ACTION_DEFAULT)
    #define ERPC_PRE_POST_ACTION_DEFAULT (ERPC_PRE_POST_ACTION_DEFAULT_DISABLED)
#endif

/* clang-format on */
#endif // _ERPC_DETECT_H_
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
