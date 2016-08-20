#-------------------------------------------------------------------------------
# Copyright (C) 2014-2016 Freescale Semiconductor, Inc. All Rights Reserved.
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

include ../../mk/erpc_common.mk

GENERATE_DOXY_NAME = test_doxyComments
GENERATE_DOXY_IN_DIR = $(TEST_ROOT)/$(GENERATE_DOXY_NAME)

.PHONY: all
all: erpcgen
	@$(call printmessage,orange,Running erpcgen-ut, test_doxyComments, $(subst $(ERPC_ROOT)/,,$<))
	$(at)$(ERPCGEN) -o $(GENERATE_DOXY_IN_DIR) $(GENERATE_DOXY_IN_DIR)/$(GENERATE_DOXY_NAME).erpc

clean:
	@echo Cleaning $(GENERATE_DOXY_NAME)...
	@rm -rf $(GENERATE_DOXY_IN_DIR)/erpc_outputs

# Unit Test Targets
.PHONY: test-tcp
test-tcp: all

.PHONY: test-ipbt
test-ipbt: all

.PHONY: test-serial
test-serial: all

.PHONY: erpcgen
erpcgen:
	@$(call printmessage,build,Building, $@ ,gray,,,\n)
	@$(MAKE) $(silent_make) -r -C $(ERPC_ROOT) -f Makefile $(APPTARGET)

.PHONY: fresh
fresh: clean all
