/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
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

//! Turn off error checking code for infrastructure in generated output
#define NO_INFRA_ERRORS "no_infra_errors"

//! Turn off error checking code for allocations in generated output
#define NO_ALLOC_ERRORS "no_alloc_errors"

//! Specify the buffer's maximum size.
#define MAX_LENGTH_ANNOTATION "max_length"

//! Allows a parameter or field to be null.
#define NULLABLE_ANNOTATION "nullable"

//! Sets the path to write output files into.
#define OUTPUT_DIR_ANNOTATION "outputDir"

//! Strip the specified string from type names in Python output.
#define PY_TYPES_NAME_STRIP_SUFFIX_ANNOTATION "py_types_name_strip_suffix"

//! Do not free memory for a parameter in the server shim.
#define RETAIN_ANNOTATION "retain"

//! Place all types definitions (e.g. typedef, structs, etc.) into one file
#define TYPES_HEADER_ANNOTATION "types_header"

#endif /* _EMBEDDED_RPC__ANNOTATIONS_H_ */
