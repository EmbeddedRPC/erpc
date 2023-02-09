#-------------------------------------------------------------------------------
# Copyright (C) 2014-2015 Freescale Semiconductor, Inc.
# Copyright 2016-2021 NXP
# All Rights Reserved.
#
# THIS SOFTWARE IS PROVIDED BY FREESCALE "AS IS" AND ANY EXPRESS OR IMPLIED
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
# SHALL FREESCALE BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
# OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
# IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
# OF SUCH DAMAGE.
#-------------------------------------------------------------------------------

#-----------------------------------------------
# path roots
# ----------------------------------------------

CUR_DIR := $(notdir $(CURDIR))

OUTPUT_ROOT := $(ERPC_ROOT)
TEST_ROOT :=  $(ERPC_ROOT)/test

ifeq "$(is_mingw)" "1"
    VISUAL_STUDIO_ROOT ?= $(ERPC_ROOT)/erpcgen/VisualStudio_v14
    MINGW64 ?= $(ERPC_ROOT)/mingw64
    CC = gcc
    CXX = g++
endif

TARGET_OUTPUT_ROOT = $(OUTPUT_ROOT)/$(DEBUG_OR_RELEASE)/$(os_name)/$(APP_NAME)
MAKE_TARGET = $(TARGET_OUTPUT_ROOT)/$(APP_NAME)

OBJS_ROOT = $(TARGET_OUTPUT_ROOT)/obj

#-----------------------------------------------
# install paths
# ----------------------------------------------

# Default install location.
PREFIX ?= /usr/local

BIN_INSTALL_DIR = $(PREFIX)/bin
LIB_INSTALL_DIR = $(PREFIX)/lib
INC_INSTALL_DIR = $(PREFIX)/include/erpc

#-----------------------------------------------
# tool paths
# ----------------------------------------------

ERPCGEN ?= $(OUTPUT_ROOT)/$(DEBUG_OR_RELEASE)/$(os_name)/erpcgen/erpcgen
LD = $(CXX)
PYTHON ?= python
ifeq "$(is_mingw)" "1"
    PYTHON=$(MINGW64)/opt/bin/python3
else ifeq (, $(shell which $(PYTHON)))
    PYTHON=python3
    ifeq (, $(shell which $(PYTHON)))
        $(error "No python found. Please install python3.")
    endif
else
    ifneq (3, $(shell $(PYTHON) --version | cut -c 8-8))
        ifeq (, $(shell which python3))
            $(error "Please install python3.")
        else
            PYTHON=python3
        endif
    endif
endif

# Tool paths. Use different paths for OS X.
ifeq "$(is_darwin)" "1"
    ifndef FLEX_ROOT
        FLEX_ROOT := $(shell brew --prefix flex)
    endif
    ifndef BISON_ROOT
        BISON_ROOT := $(shell brew --prefix bison)
    endif

    FLEX ?= $(FLEX_ROOT)/bin/flex
    BISON ?= $(BISON_ROOT)/bin/bison
else ifeq "$(is_linux)" "1"
    FLEX ?= /usr/bin/flex
    BISON ?= /usr/bin/bison
else ifeq "$(is_cygwin)" "1"
    FLEX ?= /bin/flex
    BISON ?= /bin/bison
else ifeq "$(is_mingw)" "1"
    FLEX ?= $(VISUAL_STUDIO_ROOT)/win_flex.exe
    BISON ?= $(VISUAL_STUDIO_ROOT)/win_bison.exe
endif

ifeq "$(is_mingw)" "1"
    MAKE := mingw32-make
    POWERSHELL ?= powershell
    mkdirc = $(POWERSHELL) mkdir -Force
    rmc = $(POWERSHELL) rm -Recurse -Force -ErrorAction Ignore
else
    rmc = rm -rf
    mkdirc = mkdir
endif

#-----------------------------------------------
# Debug or Release
# Release by default
#-----------------------------------------------
build ?= release
