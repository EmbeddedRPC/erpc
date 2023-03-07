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
include $(ERPC_ROOT)/test/mk/erpc_src.mk

INCLUDES += $(ERPC_ROOT)/test/common/config

MAKE_TARGET = $(TARGET_LIB)($(OBJECTS_ALL))

include $(ERPC_ROOT)/mk/targets.mk

$(TARGET_LIB)(%): %
	@$(call printmessage,ar,Archiving, $(?F) in $(@F))
	$(at)$(mkdirc) -p $(dir $(@))
	$(AR) $(ARFLAGS) $@ $?
