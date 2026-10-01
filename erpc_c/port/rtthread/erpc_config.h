/*
 * RT-Thread platform configuration for eRPC.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _ERPC_CONFIG_H_
#define _ERPC_CONFIG_H_

#include "rtconfig.h"

/*!
 * @addtogroup config
 * @{
 * @file
 */

//! @name Configuration value constants
//@{
#define ERPC_ALLOCATION_POLICY_DYNAMIC (0U) //!< Dynamic allocation policy.
#define ERPC_ALLOCATION_POLICY_STATIC  (1U) //!< Static allocation policy.

#define ERPC_THREADS_NONE     (0U) //!< No threads.
#define ERPC_THREADS_PTHREADS (1U) //!< POSIX pthreads.
#define ERPC_THREADS_FREERTOS (2U) //!< FreeRTOS.
#define ERPC_THREADS_ZEPHYR   (3U) //!< Zephyr.
#define ERPC_THREADS_MBED     (4U) //!< Mbed OS.
#define ERPC_THREADS_WIN32    (5U) //!< Win32.
#define ERPC_THREADS_THREADX  (6U) //!< ThreadX.
#define ERPC_THREADS_RTTHREAD (7U) //!< RT-Thread.

#define ERPC_NOEXCEPT_DISABLED (0U) //!< Disable noexcept support.
#define ERPC_NOEXCEPT_ENABLED  (1U) //!< Enable noexcept support.

#define ERPC_NESTED_CALLS_DISABLED           (0U) //!< Disable nested call support.
#define ERPC_NESTED_CALLS_ENABLED            (1U) //!< Enable nested call support.
#define ERPC_NESTED_CALLS_DETECTION_DISABLED (0U) //!< Disable nested call detection.
#define ERPC_NESTED_CALLS_DETECTION_ENABLED  (1U) //!< Enable nested call detection.

#define ERPC_MESSAGE_LOGGING_DISABLED (0U) //!< Disable message logging.
#define ERPC_MESSAGE_LOGGING_ENABLED  (1U) //!< Enable message logging.

#define ERPC_TRANSPORT_MU_USE_MCMGR_DISABLED (0U) //!< Do not use MCMGR for MU ISR management.
#define ERPC_TRANSPORT_MU_USE_MCMGR_ENABLED  (1U) //!< Use MCMGR for MU ISR management.

#define ERPC_PRE_POST_ACTION_DISABLED         (0U) //!< Disable pre/post shim callbacks.
#define ERPC_PRE_POST_ACTION_ENABLED          (1U) //!< Enable pre/post shim callbacks.
#define ERPC_PRE_POST_ACTION_DEFAULT_DISABLED (0U) //!< Disable default pre/post callbacks.
#define ERPC_PRE_POST_ACTION_DEFAULT_ENABLED  (1U) //!< Enable default pre/post callbacks.
//@}

//! @name RT-Thread platform defaults
//@{

/**
 * @brief Disable POSIX auto-detection for RT-Thread package builds.
 *
 * RT-Thread MCU builds must not select pthreads just because the host compiler
 * or toolchain exposes POSIX macros.
 */
#define ERPC_HAS_POSIX (0)

/**
 * @brief Disable Win32 auto-detection for RT-Thread package builds.
 *
 * This package does not build the Win32 port, so Win32 detection is disabled
 * unconditionally for the RT-Thread configuration entry.
 */
#define ERPC_HAS_WIN32 (0)

/** @brief Force eRPC to use the native RT-Thread threading backend. */
#define ERPC_THREADS (ERPC_THREADS_RTTHREAD)

/** @brief Force eRPC to use dynamic allocation through the RT-Thread heap port. */
#define ERPC_ALLOCATION_POLICY (ERPC_ALLOCATION_POLICY_DYNAMIC)

#ifndef PKG_ERPC_RTTHREAD_THREAD_STACK_SIZE
#define PKG_ERPC_RTTHREAD_THREAD_STACK_SIZE (2048U)
#endif /* PKG_ERPC_RTTHREAD_THREAD_STACK_SIZE */

#ifndef PKG_ERPC_RTTHREAD_THREAD_PRIORITY
#define PKG_ERPC_RTTHREAD_THREAD_PRIORITY (20U)
#endif /* PKG_ERPC_RTTHREAD_THREAD_PRIORITY */

#ifndef PKG_ERPC_RTTHREAD_THREAD_TIMESLICE
#define PKG_ERPC_RTTHREAD_THREAD_TIMESLICE (10U)
#endif /* PKG_ERPC_RTTHREAD_THREAD_TIMESLICE */

#ifndef PKG_ERPC_RTTHREAD_UART_TRANSPORT_COUNT
#define PKG_ERPC_RTTHREAD_UART_TRANSPORT_COUNT (1U)
#endif /* PKG_ERPC_RTTHREAD_UART_TRANSPORT_COUNT */

/** @brief Default stack size for eRPC threads created by the RT-Thread backend. */
#define ERPC_RTTHREAD_THREAD_STACK_SIZE (PKG_ERPC_RTTHREAD_THREAD_STACK_SIZE)

/** @brief Default native priority for eRPC threads created with priority 0. */
#define ERPC_RTTHREAD_THREAD_PRIORITY (PKG_ERPC_RTTHREAD_THREAD_PRIORITY)

/** @brief Default timeslice for eRPC threads created by the RT-Thread backend. */
#define ERPC_RTTHREAD_THREAD_TIMESLICE (PKG_ERPC_RTTHREAD_THREAD_TIMESLICE)

/** @brief Maximum number of simultaneously active RT-Thread UART transports. */
#define ERPC_RTTHREAD_UART_TRANSPORT_COUNT (PKG_ERPC_RTTHREAD_UART_TRANSPORT_COUNT)
//@}

//! @name Message buffer settings
//@{
#ifndef PKG_ERPC_DEFAULT_BUFFER_SIZE
#define PKG_ERPC_DEFAULT_BUFFER_SIZE (256U)
#endif /* PKG_ERPC_DEFAULT_BUFFER_SIZE */

#ifndef PKG_ERPC_DEFAULT_BUFFERS_COUNT
#define PKG_ERPC_DEFAULT_BUFFERS_COUNT (2U)
#endif /* PKG_ERPC_DEFAULT_BUFFERS_COUNT */

/** @brief eRPC message buffer size in bytes. */
#define ERPC_DEFAULT_BUFFER_SIZE (PKG_ERPC_DEFAULT_BUFFER_SIZE)

/** @brief Number of buffers owned by the static message buffer factory. */
#define ERPC_DEFAULT_BUFFERS_COUNT (PKG_ERPC_DEFAULT_BUFFERS_COUNT)
//@}

//! @name eRPC feature settings
//@{
/** @brief Configure eRPC noexcept support. */
#if defined(PKG_ERPC_NOEXCEPT)
#define ERPC_NOEXCEPT (ERPC_NOEXCEPT_ENABLED)
#else
#define ERPC_NOEXCEPT (ERPC_NOEXCEPT_DISABLED)
#endif /* PKG_ERPC_NOEXCEPT */

/** @brief Configure nested call support. */
#if defined(PKG_ERPC_NESTED_CALLS)
#define ERPC_NESTED_CALLS (ERPC_NESTED_CALLS_ENABLED)
#else
#define ERPC_NESTED_CALLS (ERPC_NESTED_CALLS_DISABLED)
#endif /* PKG_ERPC_NESTED_CALLS */

/** @brief Configure nested call detection. */
#if defined(PKG_ERPC_NESTED_CALLS_DETECTION)
#define ERPC_NESTED_CALLS_DETECTION (ERPC_NESTED_CALLS_DETECTION_ENABLED)
#else
#define ERPC_NESTED_CALLS_DETECTION (ERPC_NESTED_CALLS_DETECTION_DISABLED)
#endif /* PKG_ERPC_NESTED_CALLS_DETECTION */

/** @brief Configure eRPC message logging support. */
#if defined(PKG_ERPC_MESSAGE_LOGGING)
#define ERPC_MESSAGE_LOGGING (ERPC_MESSAGE_LOGGING_ENABLED)
#else
#define ERPC_MESSAGE_LOGGING (ERPC_MESSAGE_LOGGING_DISABLED)
#endif /* PKG_ERPC_MESSAGE_LOGGING */

/** @brief Configure MCMGR usage in the MU transport. */
#if defined(PKG_ERPC_TRANSPORT_MU_USE_MCMGR)
#define ERPC_TRANSPORT_MU_USE_MCMGR (ERPC_TRANSPORT_MU_USE_MCMGR_ENABLED)
#else
#define ERPC_TRANSPORT_MU_USE_MCMGR (ERPC_TRANSPORT_MU_USE_MCMGR_DISABLED)
#endif /* PKG_ERPC_TRANSPORT_MU_USE_MCMGR */

/** @brief Configure eRPC pre/post action shim support. */
#if defined(PKG_ERPC_PRE_POST_ACTION)
#define ERPC_PRE_POST_ACTION (ERPC_PRE_POST_ACTION_ENABLED)
#else
#define ERPC_PRE_POST_ACTION (ERPC_PRE_POST_ACTION_DISABLED)
#endif /* PKG_ERPC_PRE_POST_ACTION */

/** @brief Configure default eRPC pre/post action callback support. */
#if defined(PKG_ERPC_PRE_POST_ACTION_DEFAULT)
#define ERPC_PRE_POST_ACTION_DEFAULT (ERPC_PRE_POST_ACTION_DEFAULT_ENABLED)
#else
#define ERPC_PRE_POST_ACTION_DEFAULT (ERPC_PRE_POST_ACTION_DEFAULT_DISABLED)
#endif /* PKG_ERPC_PRE_POST_ACTION_DEFAULT */
//@}

//! @name eRPC endianness helper settings
//@{
#ifndef PKG_ERPC_ENDIANNESS_HEADER
#define PKG_ERPC_ENDIANNESS_HEADER "erpc_endianness_agnostic_example.h"
#endif /* PKG_ERPC_ENDIANNESS_HEADER */

#define ENDIANNESS_HEADER PKG_ERPC_ENDIANNESS_HEADER

#if defined(PKG_ERPC_PROCESSOR_ENDIANNESS_BIG)
#define ERPC_PROCESSOR_ENDIANNESS_LITTLE (0)
#else
#define ERPC_PROCESSOR_ENDIANNESS_LITTLE (1)
#endif /* PKG_ERPC_PROCESSOR_ENDIANNESS_BIG */

#if defined(PKG_ERPC_COMMUNICATION_BIG)
#define ERPC_COMMUNICATION_LITTLE (0)
#else
#define ERPC_COMMUNICATION_LITTLE (1)
#endif /* PKG_ERPC_COMMUNICATION_BIG */

#if defined(PKG_ERPC_POINTER_SIZE_16)
#define ERPC_POINTER_SIZE_16 (1)
#define ERPC_POINTER_SIZE_32 (0)
#define ERPC_POINTER_SIZE_64 (0)
#elif defined(PKG_ERPC_POINTER_SIZE_64)
#define ERPC_POINTER_SIZE_16 (0)
#define ERPC_POINTER_SIZE_32 (0)
#define ERPC_POINTER_SIZE_64 (1)
#else
#define ERPC_POINTER_SIZE_16 (0)
#define ERPC_POINTER_SIZE_32 (1)
#define ERPC_POINTER_SIZE_64 (0)
#endif /* PKG_ERPC_POINTER_SIZE_16 */
//@}

/*! @} */

#endif /* _ERPC_CONFIG_H_ */
