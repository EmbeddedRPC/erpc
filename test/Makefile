#-------------------------------------------------------------------------------
# Copyright (C) 2014 Freescale Semiconductor, Inc.
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

include ../mk/erpc_common.mk

.NOTPARALLEL:

# Unit test directories.
SUBDIRS =  $(shell echo test_*)

.PHONY: all
all : $(SUBDIRS)

# Unit Test Targets
.PHONY: test-tcp
test-tcp: TESTTARGET := test-tcp
test-tcp: all

.PHONY: test-serial
test-serial: TESTTARGET := test-serial
test-serial: all

clean::
	@echo "Cleaning test results..."
	@$(rmc) results

include $(ERPC_ROOT)/mk/subdirs.mk
