#-------------------------------------------------------------------------------
# Copyright (C) 2014-2016 Freescale Semiconductor
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

#
# Top-level Makefile.
#
# This file is responsible for building all libraries and applications.
#

include mk/erpc_common.mk

# Turn off parallel jobs for this makefile only. Child makefiles will still use the
# specified number of jobs. This isn't strictly necessary, and actually slows the build
# a little bit, but greatly improves the readability of the log output.
#.NOTPARALLEL:

ifeq "$(is_linux)" "1"
ERPCSNIFFER = erpcsniffer
endif

# Subdirectories to run make on.
TESTDIR = test
SUBDIRS_LIST = erpcgen $(ERPCSNIFFER) $(TESTDIR)
# test if all exists
SUBDIRS = $(foreach f,$(SUBDIRS_LIST), $(if $(wildcard $(f)), $(f)))

# Default target.
.PHONY: default
default: erpc $(ERPCSNIFFER) erpcgen

erpcsniffer: erpc

.PHONY: erpc
erpc:
	@$(MAKE) $(silent_make) -j$(MAKETHREADS) -r -C erpc_c

.PHONY: install
install: erpc erpc_c erpcgen $(ERPCSNIFFER)
	@$(MAKE) $(silent_make) -j$(MAKETHREADS) -r -C erpc_c install
	@$(MAKE) $(silent_make) -j$(MAKETHREADS) -r -C erpcgen install
ifeq "$(is_linux)" "1"
	@$(MAKE) $(silent_make) -j$(MAKETHREADS) -r -C erpcsniffer install
endif

#make all target
.PHONY: all
all: erpc $(SUBDIRS)

# Unit Test Targets
.PHONY: test-tcp
test-tcp: TESTTARGET := test-tcp
test-tcp: $(TESTDIR)

.PHONY: test-serial
test-serial: TESTTARGET := test-serial
test-serial: $(TESTDIR)

# Force rebuild
.PHONY: fresh
fresh: clean all

# define clean_fun
# 	$(warning "$(at)$(POWERSHELL) "if (Test-Path $(1)) { $(rmc) $(1) }"")
# 	$(at)$(POWERSHELL) 'if (Test-Path $(1)) { $(rmc) $(1) }'
# endef

# Target to clean everything.
.PHONY: clean
clean::
	@echo "Deleting output directories..."
	@$(rmc) Debug Release
	@$(rmc) out*.*
	@$(rmc) erpc_outputs
# 	@$(call clean_fun,Debug,)
# 	@$(call clean_fun,Release,)
# 	@$(call clean_fun,erpc_outputs,)
	@echo "done."

# Process subdirs
include $(ERPC_ROOT)/mk/subdirs.mk

.PHONY: os_name
os_name:
	@echo $(os_name)
