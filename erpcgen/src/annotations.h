/*
 * The Clear BSD License
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
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

#ifndef _EMBEDDED_RPC__ANNOTATIONS_H_
#define _EMBEDDED_RPC__ANNOTATIONS_H_

//! Define union discriminator name for non-encapsulated unions.
#define CRC_ANNOTATION "crc"

//! Define union discriminator name for non-encapsulated unions.
#define DISCRIMINATOR_ANNOTATION "discriminator"

//! Service objects will be dynamically allocated.
#define DYNAMIC_SERVICES_ANNOTATION "dynamic_services"

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
