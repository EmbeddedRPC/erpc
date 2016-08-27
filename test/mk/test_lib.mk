#-------------------------------------------------------------------------------
# Copyright (C) 2014-2016 Freescale Semiconductor, Inc.
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

# Parallel building can sometimes fail on certain systems.
.NOTPARALLEL:

include ../../mk/erpc_common.mk
-include $(TEST_ROOT)/$(TEST_NAME)/variables.mk

#-----------------------------------------------
# setup variables
# ----------------------------------------------

LIB_NAME = test

TARGET_OUTPUT_ROOT = $(OUTPUT_ROOT)/$(DEBUG_OR_RELEASE)/$(os_name)/$(LIB_NAME)

TARGET_LIB = $(LIBS_ROOT)/lib$(LIB_NAME).a

OBJS_ROOT = $(TARGET_OUTPUT_ROOT)/obj

LIBS_ROOT = $(TARGET_OUTPUT_ROOT)/lib

UT_COMMON_SRC = $(ERPC_ROOT)/test/common

ERPC_C_ROOT = $(ERPC_ROOT)/erpc_c

RPC_OBJS_ROOT = $(TARGET_OUTPUT_ROOT)
UNIT_OUT_DIR = $(RPC_OBJS_ROOT)/unit_test_common

#-----------------------------------------------
# Include path. Add the include paths like this:
# INCLUDES += ./include/
#-----------------------------------------------
INCLUDES += $(UT_COMMON_SRC) \
            $(ERPC_ROOT)/erpc_c/config \
            $(ERPC_ROOT)/erpc_c/infra \
            $(ERPC_ROOT)/erpc_c/port \
            $(ERPC_ROOT)/erpc_c/setup \
            $(ERPC_ROOT)/erpc_c/transports \
            $(ERPC_ROOT)/erpcgen/src \
            $(RPC_OBJS_ROOT) \
            $(UT_COMMON_SRC)/gtest

SOURCES +=  $(UT_COMMON_SRC)/addOne.cpp \
            $(UT_COMMON_SRC)/gtest/gtest.cpp \
            $(ERPC_ROOT)/erpcgen/src/format_string.cpp \
            $(ERPC_ROOT)/erpcgen/src/Logging.cpp \
			$(ERPC_C_ROOT)/infra/basic_codec.cpp \
			$(ERPC_C_ROOT)/infra/client_manager.cpp \
			$(ERPC_C_ROOT)/infra/server.cpp \
			$(ERPC_C_ROOT)/infra/simple_server.cpp \
			$(ERPC_C_ROOT)/infra/framed_transport.cpp \
			$(ERPC_C_ROOT)/infra/message_buffer.cpp \
			$(ERPC_C_ROOT)/port/erpc_port_stdlib.cpp \
			$(ERPC_C_ROOT)/port/erpc_threading_pthreads.cpp \
			$(ERPC_C_ROOT)/port/serial.cpp \
			$(ERPC_C_ROOT)/transports/serial_transport.cpp \
			$(ERPC_C_ROOT)/transports/tcp_transport.cpp \
			$(UNIT_OUT_DIR)/unit_test_common_client.cpp \
			$(UNIT_OUT_DIR)/unit_test_common_server.cpp

MAKE_TARGET = $(TARGET_LIB)($(OBJECTS_ALL))

.PHONY: all
all: $(UNIT_OUT_DIR)/unit_test_common_client.cpp $(UNIT_OUT_DIR)/unit_test_common/client.py

include $(ERPC_ROOT)/mk/targets.mk

$(TARGET_LIB)(%): %
	@$(call printmessage,ar,Archiving, $(?F) in $(@F))
	$(at)mkdir -p $(dir $(@))
	$(AR) $(ARFLAGS) $@ $?

$(UT_COMMON_SRC)/unit_test_$(TRANSPORT)_$(APP_TYPE).cpp: $(UNIT_OUT_DIR)/unit_test_common_$(APP_TYPE).cpp

# Run erpcgen on common code for C.
$(UNIT_OUT_DIR)/unit_test_common_client.cpp: $(UT_COMMON_SRC)/unit_test_common.erpc
	@$(call printmessage,orange,Running erpcgen-c common, $(subst $(UT_COMMON_SRC)/,,$<))
	$(at)$(ERPCGEN) -gc -o $(RPC_OBJS_ROOT)/ $(UT_COMMON_SRC)/unit_test_common.erpc

# Run erpcgen on common code for Python.
$(UNIT_OUT_DIR)/unit_test_common/client.py: $(UT_COMMON_SRC)/unit_test_common.erpc
	@$(call printmessage,orange,Running erpcgen-py common, $(subst $(UT_COMMON_SRC)/,,$<))
	$(at)$(ERPCGEN) -gpy -o $(RPC_OBJS_ROOT)/ $(UT_COMMON_SRC)/unit_test_common.erpc
