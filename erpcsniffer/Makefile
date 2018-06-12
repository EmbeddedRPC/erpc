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

APP_NAME = erpcsniffer
LIB_NAME = erpc

PYTH_SCRIPT = erpcgen/bin/txt_to_c.py
ERPC_LIB_OUTPUT_DIR = $(OUTPUT_ROOT)/$(DEBUG_OR_RELEASE)/$(os_name)/$(LIB_NAME)/lib

#-----------------------------------------------
# Include path. Add the include paths like this:
# INCLUDES += ./include/
#-----------------------------------------------
INCLUDES += $(ERPC_ROOT) \
			$(ERPC_ROOT)/erpcgen/src \
			$(ERPC_ROOT)/erpcgen/src/cpptemplate \
			$(ERPC_ROOT)/erpcgen/src/types \
			$(ERPC_ROOT)/erpc_c/infra \
			$(ERPC_ROOT)/erpc_c/port \
			$(ERPC_ROOT)/erpc_c/setup \
			$(ERPC_ROOT)/erpc_c/transports \
			$(ERPC_ROOT)/erpcsniffer/src \
			$(OBJS_ROOT) \
			$(BOOST_ROOT)/include

ifeq "$(is_mingw)" "1"
INCLUDES += $(ERPC_ROOT)/windows \
			$(BOOST_ROOT)
endif

SOURCES += 	$(OBJS_ROOT)/erpcgen_parser.tab.cpp \
			$(OBJS_ROOT)/erpcgen_lexer.cpp \
			$(ERPC_ROOT)/erpcgen/src/format_string.cpp \
			$(ERPC_ROOT)/erpcgen/src/HexValues.cpp \
			$(ERPC_ROOT)/erpcgen/src/Logging.cpp \
			$(ERPC_ROOT)/erpcgen/src/options.cpp \
			$(ERPC_ROOT)/erpcgen/src/SearchPath.cpp \
			$(ERPC_ROOT)/erpcgen/src/AstNode.cpp \
			$(ERPC_ROOT)/erpcgen/src/AstWalker.cpp \
			$(ERPC_ROOT)/erpcgen/src/UniqueIdChecker.cpp \
			$(ERPC_ROOT)/erpcgen/src/ErpcLexer.cpp \
			$(ERPC_ROOT)/erpcgen/src/InterfaceDefinition.cpp \
			$(ERPC_ROOT)/erpcgen/src/SymbolScanner.cpp \
			$(ERPC_ROOT)/erpcgen/src/Token.cpp \
			$(ERPC_ROOT)/erpcgen/src/ParseErrors.cpp \
			$(ERPC_ROOT)/erpcgen/src/types/Type.cpp \
			$(ERPC_ROOT)/erpcsniffer/src/erpcsniffer.cpp \
			$(ERPC_ROOT)/erpcsniffer/src/Sniffer.cpp

# Prevent make from deleting these temp files.
.SECONDARY: $(OBJS_ROOT)/erpcgen_parser.tab.cpp \
            $(OBJS_ROOT)/erpcgen_lexer.cpp

LIBRARIES += -L$(ERPC_LIB_OUTPUT_DIR) -lerpc

OBJECT_DEP := $(OBJS_ROOT)/erpcgen_lexer.cpp

include $(ERPC_ROOT)/mk/targets.mk

ifeq "$(is_mingw)" "1"
    LIBRARIES += -L$(BOOST_ROOT)/stage/lib
    ifeq "$(build)" "debug"
        LIBRARIES += -lboost_system-mgw48-mt-1_57 -lboost_filesystem-mgw48-mt-1_57
    else
        LIBRARIES += -lboost_system-mgw48-mt-d-1_57 -lboost_filesystem-mgw48-mt-d-1_57
    endif
else
    ifeq "$(is_linux)" "1"
        LIBRARIES += -lpthread -lrt
    endif

    static_libs := NO
    ifneq "$(build)" "debug"
        ifeq "$(is_darwin)" "1"
            static_libs := $(BOOST_ROOT)/lib/libboost_system.a $(BOOST_ROOT)/lib/libboost_filesystem.a
        endif
    endif
    ifneq "$(static_libs)" "NO"
        LIBRARIES += $(static_libs)
    else
        LIBRARIES += -lboost_system -lboost_filesystem
    endif
endif

# Release should be alwas static. Customers don't need install things.
ifneq "$(build)" "debug"
    # Except Darwin. Darwin has static libs defined above.
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

.PHONY: install
install: $(MAKE_TARGET)
	@$(call printmessage,c,Installing, erpcsniffer in $(BIN_INSTALL_DIR))
	$(at)mkdir -p $(BIN_INSTALL_DIR)
	$(at)install $(MAKE_TARGET) $(BIN_INSTALL_DIR)

clean::
	$(at)rm -rf $(OBJS_ROOT)/*.cpp $(OBJS_ROOT)/*.hpp $(OBJS_ROOT)/*.c