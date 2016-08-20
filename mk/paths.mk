#-------------------------------------------------------------------------------
# Copyright (C) 2014-2015 Freescale Semiconductor, Inc. All Rights Reserved.
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

ERPC_ROOT := $(PROJ_ROOT)/erpc
OUTPUT_ROOT := $(ERPC_ROOT)
TEST_ROOT :=  $(ERPC_ROOT)/test

ifeq "$(is_mingw)" "1"
BOOST_ROOT ?= c:/boost_1_57_0
else
BOOST_ROOT ?= /usr/local/opt/boost
endif

TARGET_OUTPUT_ROOT = $(OUTPUT_ROOT)/$(DEBUG_OR_RELEASE)/$(os_name)/$(APP_NAME)
MAKE_TARGET = $(TARGET_OUTPUT_ROOT)/$(APP_NAME)

OBJS_ROOT = $(TARGET_OUTPUT_ROOT)/obj

#-----------------------------------------------
# tool paths
# ----------------------------------------------

ERPCGEN ?= $(OUTPUT_ROOT)/$(DEBUG_OR_RELEASE)/$(os_name)/erpcgen/erpcgen
LD := g++
PYTHON ?= python

# Tool paths. Use different paths for OS X.
ifeq "$(is_darwin)" "1"
FLEX ?= /usr/local/opt/flex/bin/flex
BISON ?= /usr/local/opt/bison/bin/bison
else ifeq "$(is_linux)" "1"
FLEX ?= /usr/bin/flex
BISON ?= /usr/bin/bison
else ifeq "$(is_cygwin)" "1"
FLEX ?= /bin/flex
BISON ?= /bin/bison
else ifeq "$(is_mingw)" "1"
FLEX ?= $(ERPC_ROOT)/erpcgen/VisualStudio_v12/win_flex.exe
BISON ?= $(ERPC_ROOT)/erpcgen/VisualStudio_v12/win_bison.exe
endif

ifeq "$(is_mingw)" "1"
mkdirc = C:\MinGW\msys\1.0\bin\mkdir.exe
CC+=gcc
else
mkdirc = mkdir
endif

#-----------------------------------------------
# Debug or Release
# Release by default
#-----------------------------------------------
build ?= debug

