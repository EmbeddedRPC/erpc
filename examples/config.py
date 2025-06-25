#!/bin/python3

def loadConfig(path: str = "config.h"):
    ''' Load config macros from C header file'''
    retVal = {}

    with open(path, "r") as file:
        configContext = file.read().split("#define ")[1:]
        for c in configContext:
            cfg = c.split(" ")
            if len(cfg) > 1:
                val = cfg[1][:-1].replace("\"","")
                retVal[cfg[0]] = val

    return retVal
