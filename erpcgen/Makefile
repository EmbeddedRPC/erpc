#-------------------------------------------------------------------------------
# Copyright (C) 2014-2016 Freescale Semiconductor, Inc.
# Copyright 2016 NXP
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

this_makefile := $(firstword $(MAKEFILE_LIST))
ERPC_ROOT := $(abspath $(dir $(lastword $(MAKEFILE_LIST)))../)
include $(ERPC_ROOT)/mk/erpc_common.mk
TARGET_OUTPUT_ROOT = $(OUTPUT_ROOT)/$(DEBUG_OR_RELEASE)/$(os_name)/$(APP_NAME)
#-----------------------------------------------
# setup variables
# ----------------------------------------------

APP_NAME = erpcgen

PYTH_SCRIPT = erpcgen/bin/txt_to_c.py


#-----------------------------------------------
# Include path. Add the include paths like this:
# INCLUDES += ./include/
#-----------------------------------------------
INCLUDES += $(ERPC_ROOT) \
			$(ERPC_ROOT)/erpcgen/src \
			$(ERPC_ROOT)/erpcgen/src/cpptemplate \
			$(ERPC_ROOT)/erpcgen/src/types \
			$(ERPC_ROOT)/erpc_c/infra \
			$(OBJS_ROOT)

ifeq "$(is_mingw)" "1"
INCLUDES += $(VISUAL_STUDIO_ROOT)
endif

SOURCES += $(OBJS_ROOT)/erpcgen_parser.tab.cpp \
			$(OBJS_ROOT)/erpcgen_lexer.cpp \
			$(ERPC_ROOT)/erpcgen/src/cpptemplate/cpptempl.cpp \
			$(ERPC_ROOT)/erpcgen/src/format_string.cpp \
			$(ERPC_ROOT)/erpcgen/src/HexValues.cpp \
			$(ERPC_ROOT)/erpcgen/src/Logging.cpp \
			$(ERPC_ROOT)/erpcgen/src/options.cpp \
			$(ERPC_ROOT)/erpcgen/src/SearchPath.cpp \
			$(ERPC_ROOT)/erpcgen/src/AstNode.cpp \
			$(ERPC_ROOT)/erpcgen/src/AstWalker.cpp \
			$(ERPC_ROOT)/erpcgen/src/UniqueIdChecker.cpp \
			$(ERPC_ROOT)/erpcgen/src/CGenerator.cpp \
			$(ERPC_ROOT)/erpcgen/src/PythonGenerator.cpp \
			$(ERPC_ROOT)/erpcgen/src/erpcgen.cpp \
			$(ERPC_ROOT)/erpcgen/src/ErpcLexer.cpp \
			$(ERPC_ROOT)/erpcgen/src/Generator.cpp \
			$(ERPC_ROOT)/erpcgen/src/InterfaceDefinition.cpp \
			$(ERPC_ROOT)/erpcgen/src/SymbolScanner.cpp \
			$(ERPC_ROOT)/erpcgen/src/Token.cpp \
			$(ERPC_ROOT)/erpcgen/src/ParseErrors.cpp \
			$(ERPC_ROOT)/erpcgen/src/Utils.cpp \
			$(ERPC_ROOT)/erpcgen/src/types/Type.cpp \
			$(ERPC_ROOT)/erpc_c/infra/erpc_crc16.cpp

# Prevent make from deleting these temp files.
.SECONDARY: $(OBJS_ROOT)/erpcgen_parser.tab.cpp \
            $(OBJS_ROOT)/erpcgen_lexer.cpp \
            $(OBJS_ROOT)/erpcgen/src/templates/c_common_header.c \
            $(OBJS_ROOT)/erpcgen/src/templates/cpp_interface_header.c \
            $(OBJS_ROOT)/erpcgen/src/templates/cpp_client_header.c \
            $(OBJS_ROOT)/erpcgen/src/templates/cpp_client_source.c \
            $(OBJS_ROOT)/erpcgen/src/templates/cpp_server_header.c \
            $(OBJS_ROOT)/erpcgen/src/templates/cpp_server_source.c \
            $(OBJS_ROOT)/erpcgen/src/templates/cpp_coders.c \
            $(OBJS_ROOT)/erpcgen/src/templates/cpp_common_functions.c \
            $(OBJS_ROOT)/erpcgen/src/templates/c_client_header.c \
            $(OBJS_ROOT)/erpcgen/src/templates/c_client_source.c \
            $(OBJS_ROOT)/erpcgen/src/templates/c_server_header.c \
            $(OBJS_ROOT)/erpcgen/src/templates/c_server_source.c \
            $(OBJS_ROOT)/erpcgen/src/templates/c_crc.c \
            $(OBJS_ROOT)/erpcgen/src/templates/py_init.c \
            $(OBJS_ROOT)/erpcgen/src/templates/py_common.c \
            $(OBJS_ROOT)/erpcgen/src/templates/py_client.c \
            $(OBJS_ROOT)/erpcgen/src/templates/py_server.c \
            $(OBJS_ROOT)/erpcgen/src/templates/py_interface.c \
            $(OBJS_ROOT)/erpcgen/src/templates/py_coders.c \
            $(OBJS_ROOT)/erpcgen/src/templates/py_global_init.c

SOURCES +=  $(OBJS_ROOT)/erpcgen/src/templates/c_common_header.c \
            $(OBJS_ROOT)/erpcgen/src/templates/cpp_interface_header.c \
            $(OBJS_ROOT)/erpcgen/src/templates/cpp_client_header.c \
            $(OBJS_ROOT)/erpcgen/src/templates/cpp_client_source.c \
            $(OBJS_ROOT)/erpcgen/src/templates/cpp_server_header.c \
            $(OBJS_ROOT)/erpcgen/src/templates/cpp_server_source.c \
            $(OBJS_ROOT)/erpcgen/src/templates/cpp_coders.c \
            $(OBJS_ROOT)/erpcgen/src/templates/cpp_common_functions.c \
            $(OBJS_ROOT)/erpcgen/src/templates/c_client_header.c \
            $(OBJS_ROOT)/erpcgen/src/templates/c_client_source.c \
            $(OBJS_ROOT)/erpcgen/src/templates/c_server_header.c \
            $(OBJS_ROOT)/erpcgen/src/templates/c_server_source.c \
            $(OBJS_ROOT)/erpcgen/src/templates/c_crc.c \
            $(OBJS_ROOT)/erpcgen/src/templates/py_init.c \
            $(OBJS_ROOT)/erpcgen/src/templates/py_common.c \
            $(OBJS_ROOT)/erpcgen/src/templates/py_client.c \
            $(OBJS_ROOT)/erpcgen/src/templates/py_server.c \
            $(OBJS_ROOT)/erpcgen/src/templates/py_interface.c \
            $(OBJS_ROOT)/erpcgen/src/templates/py_coders.c \
            $(OBJS_ROOT)/erpcgen/src/templates/py_global_init.c

OBJECT_DEP := $(OBJS_ROOT)/erpcgen_lexer.cpp

include $(ERPC_ROOT)/mk/targets.mk

# Release should be alwas static. Customers don't need install things.
ifeq "$(build)" "release"
    # Except Darwin.
    ifeq "$(is_darwin)" ""
        LIBRARIES += -static
    endif
endif

# Run flex
$(OBJS_ROOT)/erpcgen_lexer.cpp: $(ERPC_ROOT)/erpcgen/src/erpcgen_lexer.l $(OBJS_ROOT)/erpcgen_parser.tab.hpp | $(OBJECTS_DIRS)
	@$(call printmessage,orange,Generating, $(subst $(ERPC_ROOT)/,,$<))
	$(at)$(FLEX) $(LLFLAGS) -o $(OBJS_ROOT)/erpcgen_lexer.cpp $<

# Run bison
$(OBJS_ROOT)/erpcgen_parser.tab.cpp: $(OBJS_ROOT)/erpcgen_parser.tab.hpp

$(OBJS_ROOT)/erpcgen_parser.tab.hpp: $(ERPC_ROOT)/erpcgen/src/erpcgen_parser.y | $(OBJECTS_DIRS)
	@$(call printmessage,orange,Generating, $(subst $(ERPC_ROOT)/,,$<))
	$(at)$(BISON) $(YYFLAGS) -o $(OBJS_ROOT)/erpcgen_parser.tab.cpp $<

# Convert templates to C sources.
$(OBJS_ROOT)/%.c: $(ERPC_ROOT)/%.template
	@$(call printmessage,orange,Converting, $(subst $(ERPC_ROOT)/,,$<))
	$(at)$(mkdirc) -p $(dir $@) && $(PYTHON) $(ERPC_ROOT)/$(PYTH_SCRIPT) -o $@ $<

.PHONY: install
install: $(MAKE_TARGET)
	@$(call printmessage,c,Installing, erpcgen in $(BIN_INSTALL_DIR))
	$(at)$(mkdirc) -p $(BIN_INSTALL_DIR)
	$(at)install $(MAKE_TARGET) $(BIN_INSTALL_DIR)

clean::
	$(at)$(rmc) $(OBJS_ROOT)/*.cpp $(OBJS_ROOT)/*.hpp $(OBJS_ROOT)/*.c
