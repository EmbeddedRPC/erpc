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

# Parallel building can sometimes fail on certain systems.

include ../../mk/erpc_common.mk
-include $(TEST_ROOT)/$(TEST_NAME)/variables.mk

#-----------------------------------------------
# setup variables
# ----------------------------------------------

# client or server
ifeq "$(TYPE)" "CLIENT"
    APP_TYPE=client
    INCLUDES +=  $(ERPC_ROOT)/test/common/gtest
else
    APP_TYPE=server
endif

# transport
ifeq "$(TRANSPORT)" "tcp"
    CXXFLAGS += -DTRANSPORT_TCP
else ifeq "$(TRANSPORT)" "serial"
    CXXFLAGS += -DSERIAL
endif

APP_NAME ?= $(TEST_NAME)_$(APP_TYPE)_$(TRANSPORT)_test
ERPC_OUT_DIR ?= $(RPC_OBJS_ROOT)/erpc_outputs
UNIT_OUT_DIR = $(OUTPUT_ROOT)/$(DEBUG_OR_RELEASE)/$(os_name)/test/
ERPC_NAME ?= test
ERPC_NAME_APP ?= $(ERPC_NAME)

TEST_DIR = $(OUTPUT_ROOT)/test/$(TEST_NAME)/$(os_name)/$(TRANSPORT)/$(CC)/$(TEST_NAME)_$(APP_TYPE)/$(DEBUG_OR_RELEASE)
RPC_OBJS_ROOT = $(TEST_DIR)
TARGET_OUTPUT_ROOT = $(RPC_OBJS_ROOT)

UT_COMMON_SRC = $(ERPC_ROOT)/test/common

USE_MESSAGE_LOGGING?= 0
CXXFLAGS += -DUSE_MESSAGE_LOGGING=$(USE_MESSAGE_LOGGING)

#-----------------------------------------------
# Include path. Add the include paths like this:
# INCLUDES += ./include/
#-----------------------------------------------
INCLUDES += $(TARGET_OUTPUT_ROOT) \
            $(ERPC_OUT_DIR) \
            $(UNIT_OUT_DIR) \
            $(UT_COMMON_SRC) \
            $(ERPC_ROOT)/erpc_c/infra \
            $(ERPC_ROOT)/erpc_c/port \
            $(ERPC_ROOT)/erpc_c/setup \
            $(ERPC_ROOT)/erpc_c/transports \
            $(ERPC_ROOT)/erpcgen/src \
            $(ERPC_ROOT)/test/common \
            $(OBJS_ROOT)

#-------------------------------
# Set IDL file
#-------------------------------
IDL_FILE = $(CUR_DIR).erpc

ifeq (,$(filter $(TEST_NAME),test_arbitrator test_callbacks))

    INCLUDES += $(ERPC_ROOT)/test/common/config

    SOURCES +=  $(ERPC_OUT_DIR)/$(ERPC_NAME_APP)_$(APP_TYPE).cpp \
                $(ERPC_OUT_DIR)/$(ERPC_NAME)_unit_test_common_$(APP_TYPE).cpp \
                $(ERPC_OUT_DIR)/$(ERPC_NAME_APP)_interface.cpp \
                $(ERPC_OUT_DIR)/$(ERPC_NAME)_unit_test_common_interface.cpp \
                $(ERPC_OUT_DIR)/c_$(ERPC_NAME_APP)_$(APP_TYPE).cpp \
                $(ERPC_OUT_DIR)/c_$(ERPC_NAME)_unit_test_common_$(APP_TYPE).cpp \
                $(CUR_DIR)_$(APP_TYPE)_impl.cpp \
                $(UT_COMMON_SRC)/unit_test_$(TRANSPORT)_$(APP_TYPE).cpp

.PHONY: all
all: $(ERPC_OUT_DIR)/$(ERPC_NAME_APP)_$(APP_TYPE).cpp $(ERPC_OUT_DIR)/$(ERPC_NAME)/$(APP_TYPE).py


# Define dependency.
$(OUTPUT_ROOT)/test/$(TEST_NAME)/$(CUR_DIR)_$(APP_TYPE)_impl.cpp: $(UT_COMMON_SRC)/unit_test_$(TRANSPORT)_$(APP_TYPE).cpp
$(UT_COMMON_SRC)/unit_test_$(TRANSPORT)_$(APP_TYPE).cpp: $(ERPC_OUT_DIR)/$(ERPC_NAME_APP)_$(APP_TYPE).cpp
$(ERPC_OUT_DIR)/$(ERPC_NAME)_unit_test_common_interface.cpp $(ERPC_OUT_DIR)/$(ERPC_NAME_APP)_interface.cpp $(ERPC_OUT_DIR)/$(ERPC_NAME_APP)_$(APP_TYPE).cpp $(ERPC_OUT_DIR)/c_$(ERPC_NAME_APP)_$(APP_TYPE).cpp $(ERPC_OUT_DIR)/c_$(ERPC_NAME)_unit_test_common_$(APP_TYPE).cpp: $(ERPC_OUT_DIR)/$(ERPC_NAME)_unit_test_common_$(APP_TYPE).cpp

# Run erpcgen for C.
$(ERPC_OUT_DIR)/$(ERPC_NAME)_unit_test_common_$(APP_TYPE).cpp: $(IDL_FILE)
	@$(call printmessage,orange,Running erpcgen-c $(TEST_NAME), $(subst $(ERPC_ROOT)/,,$<))
	$(at)$(ERPCGEN) -gc -o $(RPC_OBJS_ROOT)/ $(IDL_FILE)

# Run erpcgen for Python.
$(ERPC_OUT_DIR)/$(ERPC_NAME)/$(APP_TYPE).py: $(IDL_FILE)
	@$(call printmessage,orange,Running erpcgen-py $(TEST_NAME), $(subst $(ERPC_ROOT)/,,$<))
	$(at)$(ERPCGEN) -gpy -o $(RPC_OBJS_ROOT)/ $(IDL_FILE)

    # Add libtest.a to build.
    LIBRARIES += -ltest
    LDFLAGS += -L$(OUTPUT_ROOT)/$(DEBUG_OR_RELEASE)/$(os_name)/test/lib
else
ifeq (,$(filter $(TEST_NAME),test_arbitrator))
    INCLUDES += $(ERPC_ROOT)/test/common/config
else
    INCLUDES += $(OUTPUT_ROOT)/test/$(TEST_NAME)/config
endif

    ERPC_C_ROOT = $(ERPC_ROOT)/erpc_c

    include $(ERPC_ROOT)/test/mk/erpc_src.mk

    ifeq "$(TYPE)" "CLIENT"
        include $(TEST_ROOT)/$(TEST_NAME)/client.mk
    else
        include $(TEST_ROOT)/$(TEST_NAME)/server.mk
    endif
endif

include $(ERPC_ROOT)/mk/targets.mk
