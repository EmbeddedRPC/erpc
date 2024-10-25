#
# Copyright 2023 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
def lead_config(path: str = "config.h") -> dict:
    """ Load config macros from C header file"""
    result = {}

    with open(path, "r") as file:
        context = file.read().split("#define ")[1:]
        for c in context:
            cfg = c.split(" ")
            if len(cfg) > 1:
                val = cfg[1].strip().replace("\"", "")
                result[cfg[0]] = val

    return result
