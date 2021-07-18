/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__ANNOTATIONS_H_
#define _EMBEDDED_RPC__ANNOTATIONS_H_

//! Define union discriminator name for non-encapsulated unions.
#define CRC_ANNOTATION "crc"

//! Define union discriminator name for non-encapsulated unions.
#define DISCRIMINATOR_ANNOTATION "discriminator"

//! Set error value for provide error to client
#define ERROR_RETURN_ANNOTATION "error_return"

//! Do not generate the annotated object.
#define EXTERNAL_ANNOTATION "external"

//! Collect specified interfaces into output files by the group name.
#define GROUP_ANNOTATION "group"

//! Set interface or function unique ID.
#define ID_ANNOTATION "id"

//! Add a \#include to the output.
#define INCLUDE_ANNOTATION "include"

//! Sets the parameter/field that specifies a list's length.
#define LENGTH_ANNOTATION "length"

//! Specify the buffer's maximum size.
#define MAX_LENGTH_ANNOTATION "max_length"

//! Specify the symbol name.
#define NAME_ANNOTATION "name"

//! Turn off error checking code for allocations in generated output
#define NO_ALLOC_ERRORS_ANNOTATION "no_alloc_errors"

//! Turn off generating of "const" keyword for function parameters data types.
#define NO_CONST_PARAM "no_const_param"

//! Turn off error checking code for infrastructure in generated output
#define NO_INFRA_ERRORS_ANNOTATION "no_infra_errors"

//! Data not handled through shared memory area
#define NO_SHARED_ANNOTATION "no_shared"

//! Allows a parameter or field to be null.
#define NULLABLE_ANNOTATION "nullable"

//! Sets the path to write output files into.
#define OUTPUT_DIR_ANNOTATION "output_dir"

//! Strip the specified string from type names in Python output.
#define PY_TYPES_NAME_STRIP_SUFFIX_ANNOTATION "py_types_name_strip_suffix"

//! Do not free memory for a parameter in the server shim.
#define RETAIN_ANNOTATION "retain"

//! Data handled through shared memory area
#define SHARED_ANNOTATION "shared"

//! Shared memory area start address
#define SHARED_MEMORY_BEGIN_ANNOTATION "shared_memory_begin"

//! Shared memory area end address
#define SHARED_MEMORY_END_ANNOTATION "shared_memory_end"

//! Place all types definitions (e.g. typedef, structs, etc.) into one file
#define TYPES_HEADER_ANNOTATION "types_header"

#endif /* _EMBEDDED_RPC__ANNOTATIONS_H_ */
