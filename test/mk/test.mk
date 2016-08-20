#-------------------------------------------------------------------------------
# Copyright (C) 2014 Freescale Semiconductor, Inc.
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

ifeq "$(TYPE)" "CLIENT"
APP_TYPE=client
INCLUDES +=  $(ERPC_ROOT)/test/common/gtest
SOURCES +=  $(ERPC_ROOT)/test/common/gtest/gtest.cpp \
            $(ERPC_ROOT)/erpc_c/infra/client_manager.cpp

#Added client setup file
#ifeq "$(TRANSPORT)" "serial"
#SOURCES += $(ERPC_ROOT)/erpc_c/setup/client_setup_serial.cpp
#else ifeq "$(TRANSPORT)" "tcp"
#do nothing - we currently do not have a setup file for tcp
#endif

else
APP_TYPE=server
SOURCES += $(ERPC_ROOT)/erpc_c/infra/server.cpp \
            $(ERPC_ROOT)/erpc_c/infra/simple_server.cpp

#Added server setup file
#SOURCES += $(ERPC_ROOT)/src/erpc/server/server_setup_common.cpp
#ifeq "$(TRANSPORT)" "serial"
#SOURCES += $(ERPC_ROOT)/src/erpc/server/server_setup_serial.cpp
#else ifeq "$(TRANSPORT)" "tcp"
#do nothing - we currently do not have a setup file for tcp
#endif

endif # client or server

ifeq "$(TRANSPORT)" "tcp"
CXXFLAGS += -DTRANSPORT_TCP
SOURCES += $(ERPC_ROOT)/erpc_c/infra/framed_transport.cpp \
            $(ERPC_ROOT)/erpc_c/transports/tcp_transport.cpp \
            $(ERPC_ROOT)/erpc_c/port/erpc_threading_pthreads.cpp
else ifeq "$(TRANSPORT)" "serial"
CXXFLAGS += -DSERIAL
SOURCES += $(ERPC_ROOT)/erpc_c/infra/framed_transport.cpp \
            $(ERPC_ROOT)/erpc_c/transports/serial_transport.cpp \
            $(ERPC_ROOT)/erpc_c/port/erpc_threading_pthreads.cpp \
            $(ERPC_ROOT)/erpc_c/port/serial.cpp
endif # transport

APP_NAME ?= $(TEST_NAME)_$(APP_TYPE)_$(TRANSPORT)_test
ERPC_OUT_DIR ?= $(RPC_OBJS_ROOT)/erpc_outputs
UNIT_OUT_DIR ?= $(RPC_OBJS_ROOT)/unit_test_common
ERPC_NAME ?= $(TEST_NAME)

TEST_DIR = $(OUTPUT_ROOT)/test/$(TEST_NAME)/$(os_name)/$(TRANSPORT)/gcc/$(TEST_NAME)_$(APP_TYPE)/$(DEBUG_OR_RELEASE)
RPC_OBJS_ROOT = $(TEST_DIR)
TARGET_OUTPUT_ROOT = $(RPC_OBJS_ROOT)

UT_COMMON_SRC = $(ERPC_ROOT)/test/common

#-----------------------------------------------
# Include path. Add the include paths like this:
# INCLUDES += ./include/
#-----------------------------------------------
INCLUDES += $(TARGET_OUTPUT_ROOT)\
            $(ERPC_OUT_DIR) \
            $(UNIT_OUT_DIR) \
            $(UT_COMMON_SRC) \
            $(ERPC_ROOT)/erpc_c/config \
            $(ERPC_ROOT)/erpc_c/infra \
            $(ERPC_ROOT)/erpc_c/port \
            $(ERPC_ROOT)/erpc_c/setup \
            $(ERPC_ROOT)/erpc_c/transports \
            $(ERPC_ROOT)/erpcgen/src \
            $(OBJS_ROOT)

#-------------------------------
# Set IDL file
#-------------------------------
IDL_FILE = $(CUR_DIR).erpc

SOURCES +=  $(ERPC_OUT_DIR)/$(ERPC_NAME)_$(APP_TYPE).cpp \
            $(CUR_DIR)_$(APP_TYPE)_impl.cpp \
            $(UT_COMMON_SRC)/unit_test_$(TRANSPORT)_$(APP_TYPE).cpp \
            $(ERPC_ROOT)/erpcgen/src/format_string.cpp \
            $(ERPC_ROOT)/erpcgen/src/Logging.cpp \
            $(ERPC_ROOT)/erpc_c/infra/basic_codec.cpp \
            $(ERPC_ROOT)/erpc_c/infra/message_buffer.cpp \
            $(ERPC_ROOT)/erpc_c/port/erpc_port_stdlib.cpp \
            $(UT_COMMON_SRC)/addOne.cpp \
            $(UNIT_OUT_DIR)/unit_test_common_$(APP_TYPE).cpp

ifeq "$(is_linux)" "1"
LIBRARIES += -lpthread -lrt
endif

.PHONY: all
all: $(UNIT_OUT_DIR)/unit_test_common_$(APP_TYPE).cpp $(ERPC_OUT_DIR)/$(ERPC_NAME)_$(APP_TYPE).cpp  $(UNIT_OUT_DIR)/unit_test_common/$(APP_TYPE).py $(ERPC_OUT_DIR)/$(ERPC_NAME)/$(APP_TYPE).py

include $(ERPC_ROOT)/mk/targets.mk

# Run erpcgen on common code for C.
$(UNIT_OUT_DIR)/unit_test_common_$(APP_TYPE).cpp: $(UT_COMMON_SRC)/unit_test_common.erpc
	@$(call printmessage,orange,Running erpcgen-common C, $(subst $(UT_COMMON_SRC)/,,$<))
	$(at)$(ERPCGEN) -gc -o $(RPC_OBJS_ROOT)/ $(UT_COMMON_SRC)/unit_test_common.erpc

# Run erpcgen on common code for Python.
$(UNIT_OUT_DIR)/unit_test_common/$(APP_TYPE).py: $(UT_COMMON_SRC)/unit_test_common.erpc
	@$(call printmessage,orange,Running erpcgen-common Py, $(subst $(UT_COMMON_SRC)/,,$<))
	$(at)$(ERPCGEN) -gpy -o $(RPC_OBJS_ROOT)/ $(UT_COMMON_SRC)/unit_test_common.erpc

# Run erpcgen for C.
$(ERPC_OUT_DIR)/$(ERPC_NAME)_$(APP_TYPE).cpp: $(IDL_FILE) # $(UNIT_OUT_DIR)/unit_test_common_$(APP_TYPE).cpp
	@$(call printmessage,orange,Running erpcgen-ut C, $(subst $(ERPC_ROOT)/,,$<))
	$(at)$(ERPCGEN) -gc -o $(RPC_OBJS_ROOT)/ $(IDL_FILE)

# Run erpcgen for Python.
$(ERPC_OUT_DIR)/$(ERPC_NAME)/$(APP_TYPE).py: $(IDL_FILE)
	@$(call printmessage,orange,Running erpcgen-ut Py, $(subst $(ERPC_ROOT)/,,$<))
	$(at)$(ERPCGEN) -gpy -o $(RPC_OBJS_ROOT)/ $(IDL_FILE)
