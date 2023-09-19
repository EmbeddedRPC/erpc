#-------------------------------------------------------------------------------
# Copyright (C) 2014-2016 Freescale Semiconductor, Inc.
# Copyright 2016 NXP
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
# Include path. Add the include paths like this:
# INCLUDES += ./include/
#-----------------------------------------------

INCLUDES += $(UT_COMMON_SRC) \
            $(ERPC_ROOT)/erpc_c/infra \
            $(ERPC_ROOT)/erpc_c/port \
            $(ERPC_ROOT)/erpc_c/setup \
            $(ERPC_ROOT)/erpc_c/transports \
            $(ERPC_ROOT)/erpcgen/src \
            $(ERPC_ROOT)/test/common \
            $(RPC_OBJS_ROOT) \
            $(UT_COMMON_SRC)/gtest

SOURCES +=  $(UT_COMMON_SRC)/addOne.cpp \
            $(UT_COMMON_SRC)/gtest/gtest.cpp \
            $(ERPC_ROOT)/erpcgen/src/format_string.cpp \
            $(ERPC_ROOT)/erpcgen/src/Logging.cpp \
            $(ERPC_C_ROOT)/infra/erpc_arbitrated_client_manager.cpp \
            $(ERPC_C_ROOT)/infra/erpc_basic_codec.cpp \
            $(ERPC_C_ROOT)/infra/erpc_client_manager.cpp \
            $(ERPC_C_ROOT)/infra/erpc_crc16.cpp \
            $(ERPC_C_ROOT)/infra/erpc_server.cpp \
            $(ERPC_C_ROOT)/infra/erpc_simple_server.cpp \
            $(ERPC_C_ROOT)/infra/erpc_framed_transport.cpp \
            $(ERPC_C_ROOT)/infra/erpc_message_buffer.cpp \
            $(ERPC_C_ROOT)/infra/erpc_message_loggers.cpp \
            $(ERPC_C_ROOT)/infra/erpc_transport_arbitrator.cpp \
            $(ERPC_C_ROOT)/port/erpc_port_stdlib.cpp \
            $(ERPC_C_ROOT)/port/erpc_threading_pthreads.cpp \
            $(ERPC_C_ROOT)/transports/erpc_tcp_transport.cpp
ifeq "$(is_mingw)" ""
    SOURCES += $(ERPC_C_ROOT)/transports/erpc_serial_transport.cpp \
               $(ERPC_C_ROOT)/port/erpc_serial.cpp
endif
