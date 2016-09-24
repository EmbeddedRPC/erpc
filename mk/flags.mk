#-------------------------------------------------------------------------------
# Copyright (C) 2014 Freescale Semiconductor, Inc. All Rights Reserved.
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

#Force compilation to 32 or 64 bit architectures
#-m32 or -m64
ifeq "$(is_darwin)" "1"
    MARCH ?= -m64
else
    MARCH ?=    # -m32 or -m64
endif

CXXFLAGS += -std=gnu++11 -D LINUX -Wunused-variable -Wno-deprecated-register -Wno-narrowing -Werror $(MARCH)
CFLAGS   += -std=gnu11 -D LINUX -D _GNU_SOURCE -Werror $(MARCH)
YYFLAGS  += -Wno-other
LLFLAGS  +=
LDFLAGS += $(MARCH)

# Add boost library search path.
# This is the defualt installation location by home brew.
ifeq "$(is_darwin)" "1"
LDFLAGS += -L/usr/local/lib
INCLUDES += /usr/local/opt/flex/include
endif

# Need to pass the -U option to GNU ar to turn off deterministic mode, or
# we'll get a warning.
ifneq "$(is_darwin)" "1"
ARFLAGS = -rcsU
else
ARFLAGS = -rcs
endif

ifeq "$(build)" "debug"
DEBUG_OR_RELEASE := Debug
CFLAGS += -g3 -O0 -DDEBUG
CXXFLAGS += -g3 -O0 -DDEBUG
LDFLAGS +=
else
DEBUG_OR_RELEASE := Release
CFLAGS += -Os -DNDEBUG
CXXFLAGS += -Os -DNDEBUG
endif

ifneq "$(is_mingw)" "1"
LIBRARIES += -lc
endif

LIBRARIES += -lstdc++ -lm

