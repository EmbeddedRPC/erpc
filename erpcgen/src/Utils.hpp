/* Copyright 2023 NXP
 * Copyright 2023 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__UTILS_H_
#define _EMBEDDED_RPC__UTILS_H_

#include <iostream>
#include <string>

namespace erpcgen {

void replaceAll(std::string &str, const std::string &from, const std::string &to);

} // namespace erpcgen

#endif //_EMBEDDED_RPC__UTILS_H_
