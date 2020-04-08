#-------------------------------------------------------------------------------
# Copyright (C) 2016 Freescale Semiconductor, Inc.
# Copyright 2016-2017 NXP
# All rights reserved.
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

.NOTPARALLEL:

this_makefile := $(firstword $(MAKEFILE_LIST))
ERPC_ROOT := $(abspath $(dir $(lastword $(MAKEFILE_LIST)))../)

include $(ERPC_ROOT)/mk/erpc_common.mk

ERPC_C_ROOT = $(ERPC_ROOT)/erpc_c

# TARGET_OUTPUT_ROOT = $(OUTPUT_ROOT)/$(DEBUG_OR_RELEASE)/$(os_name)/$(APP_NAME)

#-----------------------------------------------
# setup variables
# ----------------------------------------------

LIB_NAME = erpc

TARGET_OUTPUT_ROOT = $(OUTPUT_ROOT)/$(DEBUG_OR_RELEASE)/$(os_name)/$(LIB_NAME)

TARGET_LIB = $(LIBS_ROOT)/lib$(LIB_NAME).a

OBJS_ROOT = $(TARGET_OUTPUT_ROOT)/obj

LIBS_ROOT = $(TARGET_OUTPUT_ROOT)/lib

#-----------------------------------------------
# Include path. Add the include paths like this:
# INCLUDES += ./include/
#-----------------------------------------------
INCLUDES += $(ERPC_C_ROOT)/config \
			$(ERPC_C_ROOT)/infra \
			$(ERPC_C_ROOT)/port \
			$(ERPC_C_ROOT)/setup \
			$(ERPC_C_ROOT)/transports \
			/Users/sramnath/Downloads/FreeRTOSv10.2.1/FreeRTOS/Source/include \
			/Users/sramnath/Downloads/FreeRTOSv10.2.1/FreeRTOS/Source/portable/GCC/ARM_CM4F \
			/Users/sramnath/STM32CubeF4/Projects/STM32F413ZH-Nucleo/Applications/FreeRTOS/FreeRTOS_LowPower/Inc			
			
SOURCES += 	$(ERPC_C_ROOT)/infra/erpc_arbitrated_client_manager.cpp \
			$(ERPC_C_ROOT)/infra/erpc_basic_codec.cpp \
			$(ERPC_C_ROOT)/infra/erpc_client_manager.cpp \
			$(ERPC_C_ROOT)/infra/erpc_crc16.cpp \
			$(ERPC_C_ROOT)/infra/erpc_framed_transport.cpp \
			$(ERPC_C_ROOT)/infra/erpc_message_buffer.cpp \
			$(ERPC_C_ROOT)/infra/erpc_message_loggers.cpp \
			$(ERPC_C_ROOT)/infra/erpc_server.cpp \
			$(ERPC_C_ROOT)/infra/erpc_simple_server.cpp \
			$(ERPC_C_ROOT)/infra/erpc_transport_arbitrator.cpp \
			$(ERPC_C_ROOT)/port/erpc_port_stdlib.cpp \
			$(ERPC_C_ROOT)/port/erpc_threading_pthreads.cpp \
			$(ERPC_C_ROOT)/port/erpc_serial.cpp \
			$(ERPC_C_ROOT)/setup/erpc_arbitrated_client_setup.cpp \
			$(ERPC_C_ROOT)/setup/erpc_client_setup.cpp \
			$(ERPC_C_ROOT)/setup/erpc_server_setup.cpp \
			$(ERPC_C_ROOT)/setup/erpc_setup_serial.cpp \
			$(ERPC_C_ROOT)/transports/erpc_inter_thread_buffer_transport.cpp \
			$(ERPC_C_ROOT)/transports/erpc_serial_transport.cpp \
			$(ERPC_C_ROOT)/transports/erpc_tcp_transport.cpp

HEADERS += 	$(ERPC_C_ROOT)/config/erpc_config.h \
			$(ERPC_C_ROOT)/infra/erpc_arbitrated_client_manager.h \
			$(ERPC_C_ROOT)/infra/erpc_basic_codec.h \
			$(ERPC_C_ROOT)/infra/erpc_client_manager.h \
			$(ERPC_C_ROOT)/infra/erpc_codec.h \
			$(ERPC_C_ROOT)/infra/erpc_crc16.h \
			$(ERPC_C_ROOT)/infra/erpc_common.h \
			$(ERPC_C_ROOT)/infra/erpc_version.h \
			$(ERPC_C_ROOT)/infra/erpc_framed_transport.h \
			$(ERPC_C_ROOT)/infra/erpc_manually_constructed.h \
			$(ERPC_C_ROOT)/infra/erpc_message_buffer.h \
			$(ERPC_C_ROOT)/infra/erpc_message_loggers.h \
			$(ERPC_C_ROOT)/infra/erpc_server.h \
			$(ERPC_C_ROOT)/infra/erpc_static_queue.h \
			$(ERPC_C_ROOT)/infra/erpc_transport_arbitrator.h \
			$(ERPC_C_ROOT)/infra/erpc_transport.h \
			$(ERPC_C_ROOT)/port/erpc_config_internal.h \
			$(ERPC_C_ROOT)/port/erpc_port.h \
			$(ERPC_C_ROOT)/port/erpc_threading.h \
			$(ERPC_C_ROOT)/port/erpc_serial.h \
			$(ERPC_C_ROOT)/setup/erpc_arbitrated_client_setup.h \
			$(ERPC_C_ROOT)/setup/erpc_client_setup.h \
			$(ERPC_C_ROOT)/setup/erpc_server_setup.h \
			$(ERPC_C_ROOT)/setup/erpc_transport_setup.h \
			$(ERPC_C_ROOT)/transports/erpc_inter_thread_buffer_transport.h \
			$(ERPC_C_ROOT)/transports/erpc_serial_transport.h \
			$(ERPC_C_ROOT)/transports/erpc_tcp_transport.h

MAKE_TARGET = $(TARGET_LIB)($(OBJECTS_ALL))

include $(ERPC_ROOT)/mk/targets.mk

$(TARGET_LIB)(%): %
	@$(call printmessage,ar,Archiving, $(?F) in $(@F))
	$(at)mkdir -p $(dir $(@))
	$(AR) $(ARFLAGS) $@ $?

.PHONY: install
install: install_headers install_lib

.PHONY: install_headers
install_headers: $(HEADERS) |  $(INC_INSTALL_DIR)
	@$(call printmessage,c,Installing, headers in $(INC_INSTALL_DIR))
	$(at)mkdir -p $(INC_INSTALL_DIR)
	$(at)install $(?) $(INC_INSTALL_DIR)

$(INC_INSTALL_DIR):
	$(at)mkdir -p $(INC_INSTALL_DIR)

.PHONY: install_lib
install_lib: $(TARGET_LIB)
	@$(call printmessage,c,Installing, $(subst $(LIBS_ROOT)/,,$<) in $(LIB_INSTALL_DIR))
	$(at)mkdir -p $(LIB_INSTALL_DIR)
	$(at)install $(TARGET_LIB) $(LIB_INSTALL_DIR)

clean::
	$(at)rm -rf $(OBJS_ROOT)/*.cpp $(OBJS_ROOT)/*.hpp $(OBJS_ROOT)/*.c
