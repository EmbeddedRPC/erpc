/*
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * This is example file for endianness agnostic communication based on byteswap.h.
 * Other approach can be done with htons(), htonl(), ntohs(), ntohl() and such.
 */

#ifndef _ERPC_ENDIANNESS_AGNOSTIC_EXAMPLE_H_
#define _ERPC_ENDIANNESS_AGNOSTIC_EXAMPLE_H_

#if ERPC_PROCESSOR_ENDIANNESS_LITTLE != ERPC_COMMUNICATION_LITTLE
#include <byteswap.h>

#define ERPC_WRITE_AGNOSTIC_16(value) (value) = __bswap_16(value);
#define ERPC_WRITE_AGNOSTIC_32(value) (value) = __bswap_32(value);
#define ERPC_WRITE_AGNOSTIC_64(value) (value) = __bswap_64(value);

#define ERPC_READ_AGNOSTIC_16(value) (value) = __bswap_16(value);
#define ERPC_READ_AGNOSTIC_32(value) (value) = __bswap_32(value);
#define ERPC_READ_AGNOSTIC_64(value) (value) = __bswap_64(value);

#if ERPC_POINTER_SIZE_16
#define ERPC_WRITE_AGNOSTIC_PTR(value) ERPC_WRITE_AGNOSTIC_16(value)
#define ERPC_READ_AGNOSTIC_PTR(value) ERPC_READ_AGNOSTIC_16(value)
#elif ERPC_POINTER_SIZE_32
#define ERPC_WRITE_AGNOSTIC_PTR(value) ERPC_WRITE_AGNOSTIC_32(value)
#define ERPC_READ_AGNOSTIC_PTR(value) ERPC_READ_AGNOSTIC_32(value)
#elif ERPC_POINTER_SIZE_64
#define ERPC_WRITE_AGNOSTIC_PTR(value) ERPC_WRITE_AGNOSTIC_64(value)
#define ERPC_READ_AGNOSTIC_PTR(value) ERPC_READ_AGNOSTIC_64(value)
#else
#error unknown pointer size
#endif

#else

#define ERPC_WRITE_AGNOSTIC_16(value)
#define ERPC_WRITE_AGNOSTIC_32(value)
#define ERPC_WRITE_AGNOSTIC_64(value)
#define ERPC_WRITE_AGNOSTIC_PTR(value)

#define ERPC_READ_AGNOSTIC_16(value)
#define ERPC_READ_AGNOSTIC_32(value)
#define ERPC_READ_AGNOSTIC_64(value)
#define ERPC_READ_AGNOSTIC_PTR(value)
#endif

#define ERPC_WRITE_AGNOSTIC_FLOAT(value) erpc_assert(0);
#define ERPC_WRITE_AGNOSTIC_DOUBLE(value) erpc_assert(0);
#define ERPC_READ_AGNOSTIC_FLOAT(value) erpc_assert(0);
#define ERPC_READ_AGNOSTIC_DOUBLE(value) erpc_assert(0);

#endif
