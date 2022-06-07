
#ifndef _ERPC_ENDIANNESS_UNDEFINED_H_
#define _ERPC_ENDIANNESS_UNDEFINED_H_

// Disabling endianness agnostic feature.
#define ERPC_WRITE_AGNOSTIC_16(value)
#define ERPC_WRITE_AGNOSTIC_32(value)
#define ERPC_WRITE_AGNOSTIC_64(value)
#define ERPC_WRITE_AGNOSTIC_FLOAT(value)
#define ERPC_WRITE_AGNOSTIC_DOUBLE(value)
#define ERPC_WRITE_AGNOSTIC_PTR(value)

#define ERPC_READ_AGNOSTIC_16(value)
#define ERPC_READ_AGNOSTIC_32(value)
#define ERPC_READ_AGNOSTIC_64(value)
#define ERPC_READ_AGNOSTIC_FLOAT(value)
#define ERPC_READ_AGNOSTIC_DOUBLE(value)
#define ERPC_READ_AGNOSTIC_PTR(value)

#endif
