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

#-------------------------------------------------------------------------------
# Sources to objects
#-------------------------------------------------------------------------------

INCLUDES := $(foreach includes, $(INCLUDES), -I $(includes))

# Strip sources.
SOURCES := $(strip $(SOURCES))

# Convert sources list to absolute paths and root-relative paths.
SOURCES_ABS := $(foreach s,$(SOURCES),$(abspath $(s)))
SOURCES_REL := $(subst $(ERPC_ROOT)/,,$(SOURCES_ABS))

# Get a list of unique directories containing the source files.
SOURCE_DIRS_ABS := $(sort $(foreach f,$(SOURCES_ABS),$(dir $(f))))
SOURCE_DIRS_REL := $(subst $(ERPC_ROOT)/,,$(SOURCE_DIRS_ABS))

OBJECTS_DIRS := $(addprefix $(OBJS_ROOT)/,$(SOURCE_DIRS_REL))

# Filter source files list into separate source types.
C_SOURCES = $(filter %.c,$(SOURCES_REL))
CXX_SOURCES = $(filter %.cpp,$(SOURCES_REL))
ASM_s_SOURCES = $(filter %.s,$(SOURCES_REL))
ASM_S_SOURCES = $(filter %.S,$(SOURCES_REL))

# Convert sources to objects.
OBJECTS_C := $(addprefix $(OBJS_ROOT)/,$(C_SOURCES:.c=.o))
OBJECTS_CXX := $(addprefix $(OBJS_ROOT)/,$(CXX_SOURCES:.cpp=.o))
OBJECTS_ASM := $(addprefix $(OBJS_ROOT)/,$(ASM_s_SOURCES:.s=.o))
OBJECTS_ASM_S := $(addprefix $(OBJS_ROOT)/,$(ASM_S_SOURCES:.S=.o))

# Complete list of all object files.
OBJECTS_ALL := $(sort $(OBJECTS_C) $(OBJECTS_CXX) $(OBJECTS_ASM) $(OBJECTS_ASM_S))

#-------------------------------------------------------------------------------
# Default target
#-------------------------------------------------------------------------------

# Note that prerequisite order is important here. The subdirectories must be built first, or you
# may end up with files in the current directory not getting added to libraries. This would happen
# if subdirs modified the library file after local files were compiled but before they were added
# to the library.
.PHONY: all $(APP_NAME)
all: $(MAKE_TARGET) $(OBJECTS_DIRS) $(SUBDIRS)

ifneq "x$(APP_NAME)x" "xx"
$(APP_NAME): $(MAKE_TARGET)
endif

## Recipe to create the output object file directories.
$(OBJECTS_DIRS) :
	$(at)$(mkdirc) -p $@

# Object files depend on the directories where they will be created.
#
# The dirs are made order-only prerequisites (by being listed after the '|') so they won't cause
# the objects to be rebuilt, as the modification date on a directory changes whenver its contents
# change. This would cause the objects to always be rebuilt if the dirs were normal prerequisites.
$(OBJECTS_ALL): $(OBJECT_DEP) | $(OBJECTS_DIRS)

#-------------------------------------------------------------------------------
# Pattern rules for compilation
#-------------------------------------------------------------------------------
# We cd into the source directory before calling the appropriate compiler. This must be done
# on a single command line since make calls individual recipe lines in separate shells, so
# '&&' is used to chain the commands.
#
# Generate make dependencies while compiling using the -MMD option, which excludes system headers.
# If system headers are included, there are path problems on cygwin. The -MP option creates empty
# targets for each header file so that a rebuild will be forced if the file goes missing, but
# no error will occur.

# Compile C sources.
$(OBJS_ROOT)/%.o: $(ERPC_ROOT)/%.c
	@$(call printmessage,c,Compiling, $(subst $(ERPC_ROOT)/,,$<))
	$(at)$(CC) $(CFLAGS) $(SYSTEM_INC) $(INCLUDES) $(DEFINES) -MMD -MF $(basename $@).d -MP -o $@ -c $<

# Compile C++ sources.
$(OBJS_ROOT)/%.o: $(ERPC_ROOT)/%.cpp
	@$(call printmessage,cxx,Compiling, $(subst $(ERPC_ROOT)/,,$<))
	$(at)$(CXX) $(CXXFLAGS) $(SYSTEM_INC) $(INCLUDES) $(DEFINES) -MMD -MF $(basename $@).d -MP -o $@ -c $<

# For .S assembly files, first run through the C preprocessor then assemble.
$(OBJS_ROOT)/%.o: $(ERPC_ROOT)/%.S
	@$(call printmessage,asm,Assembling, $(subst $(ERPC_ROOT)/,,$<))
	$(at)$(CPP) -D__LANGUAGE_ASM__ $(INCLUDES) $(DEFINES) -o $(basename $@).s $< \
	&& $(AS) $(ASFLAGS) $(INCLUDES) -MD $(OBJS_ROOT)/$*.d -o $@ $(basename $@).s

# Assembler sources.
$(OBJS_ROOT)/%.o: $(ERPC_ROOT)/%.s
	@$(call printmessage,asm,Assembling, $(subst $(ERPC_ROOT)/,,$<))
	$(at)$(AS) $(ASFLAGS) $(INCLUDES) -MD $(basename $@).d -o $@ $<

#------------------------------------------------------------------------
# Build the target
#------------------------------------------------------------------------

# Only link if an APP_NAME is provided.
ifneq "$(APP_NAME)" ""

# Wrap the link objects in start/end group so that ld re-checks each
# file for dependencies.  Otherwise linking static libs can be a pain
# since order matters.
$(MAKE_TARGET): $(OBJECTS_ALL)
	@$(call printmessage,link,Linking, $(APP_NAME))
	$(at)$(LD) $(LDFLAGS) \
          $(OBJECTS_ALL) \
          $(LIBRARIES) \
          -o $@
	@echo "Output binary:" ; echo "  $(APP_NAME)"

endif

#-------------------------------------------------------------------------------
# Clean
#-------------------------------------------------------------------------------
.PHONY: clean
clean::
	@echo "Cleaning $(APP_NAME)"
	$(at)rm -rf $(OBJECTS_ALL) $(OBJECTS_DIRS) $(MAKE_TARGET)

# Include dependency files.
-include $(OBJECTS_ALL:.o=.d)

