/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _ERPC_CONFIG_H_
#define _ERPC_CONFIG_H_

#include <zephyr/autoconf.h>

/*!
 * @addtogroup config
 * @{
 * @file
 */

//! @name Configuration options
//@{

// ============================================================================
// Configuration Value Constants
// ============================================================================

//! @name Allocation policy constants
//@{
#define ERPC_ALLOCATION_POLICY_DYNAMIC (0) //!< Use dynamic allocation (malloc/new)
#define ERPC_ALLOCATION_POLICY_STATIC (1)  //!< Use static allocation
//@}

//! @name Threading model constants
//@{
#define ERPC_THREADS_NONE (0)      //!< No threads
#define ERPC_THREADS_PTHREADS (1)  //!< POSIX pthreads
#define ERPC_THREADS_FREERTOS (2)  //!< FreeRTOS
#define ERPC_THREADS_ZEPHYR (3)    //!< Zephyr
#define ERPC_THREADS_MBED (4)      //!< Mbed OS
#define ERPC_THREADS_WIN32 (5)     //!< Win32 threads
#define ERPC_THREADS_THREADX (6)   //!< ThreadX
//@}

//! @name noexcept constants
//@{
#define ERPC_NOEXCEPT_DISABLED (0) //!< Disabled noexcept
#define ERPC_NOEXCEPT_ENABLED (1)  //!< Enabled noexcept
//@}

//! @name Nested calls constants
//@{
#define ERPC_NESTED_CALLS_DISABLED (0)           //!< Disabled nested calls
#define ERPC_NESTED_CALLS_ENABLED (1)            //!< Enabled nested calls
#define ERPC_NESTED_CALLS_DETECTION_DISABLED (0) //!< Disabled nested calls detection
#define ERPC_NESTED_CALLS_DETECTION_ENABLED (1)  //!< Enabled nested calls detection
//@}

//! @name Message logging constants
//@{
#define ERPC_MESSAGE_LOGGING_DISABLED (0) //!< Disabled message logging
#define ERPC_MESSAGE_LOGGING_ENABLED (1)  //!< Enabled message logging
//@}

//! @name Transport MU MCMGR constants
//@{
#define ERPC_TRANSPORT_MU_USE_MCMGR_DISABLED (0) //!< Disabled MCMGR
#define ERPC_TRANSPORT_MU_USE_MCMGR_ENABLED (1)  //!< Enabled MCMGR
//@}

//! @name Pre/post action constants
//@{
#define ERPC_PRE_POST_ACTION_DISABLED (0)         //!< Disabled pre/post callbacks
#define ERPC_PRE_POST_ACTION_ENABLED (1)          //!< Enabled pre/post callbacks
#define ERPC_PRE_POST_ACTION_DEFAULT_DISABLED (0) //!< Disabled default callbacks
#define ERPC_PRE_POST_ACTION_DEFAULT_ENABLED (1)  //!< Enabled default callbacks
//@}

// ============================================================================
// Allocation Policy
// ============================================================================

//! @def ERPC_ALLOCATION_POLICY
//!
//! Allocation policy for eRPC buffers and objects.
#ifdef CONFIG_ERPC_ALLOCATION_POLICY_STATIC
#define ERPC_ALLOCATION_POLICY (ERPC_ALLOCATION_POLICY_STATIC)
#else
#define ERPC_ALLOCATION_POLICY (ERPC_ALLOCATION_POLICY_DYNAMIC)
#endif

// ============================================================================
// Static Allocation Counts
// ============================================================================

//! @def ERPC_CODEC_COUNT
//!
//! Number of codec objects for static allocation.
#ifdef CONFIG_ERPC_CODEC_COUNT
#define ERPC_CODEC_COUNT (CONFIG_ERPC_CODEC_COUNT)
#endif

//! @def ERPC_MESSAGE_LOGGERS_COUNT
//!
//! Number of message logger objects for static allocation.
#ifdef CONFIG_ERPC_MESSAGE_LOGGERS_COUNT
#define ERPC_MESSAGE_LOGGERS_COUNT (CONFIG_ERPC_MESSAGE_LOGGERS_COUNT)
#endif

//! @def ERPC_CLIENTS_THREADS_AMOUNT
//!
//! Number of client thread objects for static allocation.
#ifdef CONFIG_ERPC_CLIENTS_THREADS_AMOUNT
#define ERPC_CLIENTS_THREADS_AMOUNT (CONFIG_ERPC_CLIENTS_THREADS_AMOUNT)
#endif

// ============================================================================
// Threading Model
// ============================================================================

//! @def ERPC_THREADS
//!
//! Threading model used by eRPC.
#ifdef CONFIG_ERPC_THREADS
#define ERPC_THREADS (CONFIG_ERPC_THREADS)
#endif

// ============================================================================
// Buffer Configuration
// ============================================================================

//! @def ERPC_DEFAULT_BUFFER_SIZE
//!
//! Size of buffers allocated by MessageBufferFactory.
//! For RPMsg transport layer, this must be 2^n - 16.
#ifdef CONFIG_ERPC_DEFAULT_BUFFER_SIZE
#define ERPC_DEFAULT_BUFFER_SIZE (CONFIG_ERPC_DEFAULT_BUFFER_SIZE)
#endif

//! @def ERPC_DEFAULT_BUFFERS_COUNT
//!
//! Count of buffers allocated for statically allocated messages.
#ifdef CONFIG_ERPC_DEFAULT_BUFFERS_COUNT
#define ERPC_DEFAULT_BUFFERS_COUNT (CONFIG_ERPC_DEFAULT_BUFFERS_COUNT)
#endif

// ============================================================================
// Feature Enables
// ============================================================================

//! @def ERPC_NOEXCEPT
//!
//! Enable C++ noexcept feature.
#ifdef CONFIG_ERPC_NOEXCEPT
#define ERPC_NOEXCEPT (ERPC_NOEXCEPT_ENABLED)
#else
#define ERPC_NOEXCEPT (ERPC_NOEXCEPT_DISABLED)
#endif

//! @def ERPC_NESTED_CALLS
//!
//! Enable support for nested eRPC calls.
#ifdef CONFIG_ERPC_NESTED_CALLS
#define ERPC_NESTED_CALLS (ERPC_NESTED_CALLS_ENABLED)
#else
#define ERPC_NESTED_CALLS (ERPC_NESTED_CALLS_DISABLED)
#endif

//! @def ERPC_NESTED_CALLS_DETECTION
//!
//! Enable detection of nested calls for debugging.
#ifdef CONFIG_ERPC_NESTED_CALLS_DETECTION
#define ERPC_NESTED_CALLS_DETECTION (ERPC_NESTED_CALLS_DETECTION_ENABLED)
#else
#define ERPC_NESTED_CALLS_DETECTION (ERPC_NESTED_CALLS_DETECTION_DISABLED)
#endif

//! @def ERPC_MESSAGE_LOGGING
//!
//! Enable eRPC message logging for debugging.
#ifdef CONFIG_ERPC_MESSAGE_LOGGING
#define ERPC_MESSAGE_LOGGING (ERPC_MESSAGE_LOGGING_ENABLED)
#else
#define ERPC_MESSAGE_LOGGING (ERPC_MESSAGE_LOGGING_DISABLED)
#endif

// ============================================================================
// Transport-specific Configuration
// ============================================================================

//! @def ERPC_TRANSPORT_MU_USE_MCMGR
//!
//! When enabled, MU transport uses Multicore Manager (MCMGR).
#ifdef CONFIG_ERPC_TRANSPORT_MU_USE_MCMGR
#define ERPC_TRANSPORT_MU_USE_MCMGR (ERPC_TRANSPORT_MU_USE_MCMGR_ENABLED)
#else
#define ERPC_TRANSPORT_MU_USE_MCMGR (ERPC_TRANSPORT_MU_USE_MCMGR_DISABLED)
#endif

// ============================================================================
// Pre/Post Action Callbacks
// ============================================================================

//! @def ERPC_PRE_POST_ACTION
//!
//! Enable pre and post callback functions for eRPC calls.
#ifdef CONFIG_ERPC_PRE_POST_ACTION
#define ERPC_PRE_POST_ACTION (ERPC_PRE_POST_ACTION_ENABLED)
#else
#define ERPC_PRE_POST_ACTION (ERPC_PRE_POST_ACTION_DISABLED)
#endif

//! @def ERPC_PRE_POST_ACTION_DEFAULT
//!
//! Enable default pre and post callback functions.
#ifdef CONFIG_ERPC_PRE_POST_ACTION_DEFAULT
#define ERPC_PRE_POST_ACTION_DEFAULT (ERPC_PRE_POST_ACTION_DEFAULT_ENABLED)
#else
#define ERPC_PRE_POST_ACTION_DEFAULT (ERPC_PRE_POST_ACTION_DEFAULT_DISABLED)
#endif

//@}

/*! @} */

#endif /* _ERPC_CONFIG_H_ */
