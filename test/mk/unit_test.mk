#-------------------------------------------------------------------------------
# Copyright (C) 2014 Freescale Semiconductor, Inc.
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

#Variables

.NOTPARALLEL:

BUILD_TYPE = Debug

TEST_DIR = $(ERPC_ROOT)/test
TEST_NAME = $(CUR_DIR)
CLIENT_NAME = $(TEST_NAME)_client
SERVER_NAME = $(TEST_NAME)_server
ERPCGEN_PATH = $(ERPC_ROOT)/$(BUILD_TYPE)/$(os_name)/erpcgen/erpcgen
UT_OUTPUT_DIR = $(OUTPUT_ROOT)/test/$(TEST_NAME)
TCP_CLIENT_PATH = $(UT_OUTPUT_DIR)/$(os_name)/tcp/gcc/$(CLIENT_NAME)/$(DEBUG_OR_RELEASE)/$(CLIENT_NAME)_tcp_test
TCP_SERVER_PATH = $(UT_OUTPUT_DIR)/$(os_name)/tcp/gcc/$(SERVER_NAME)/$(DEBUG_OR_RELEASE)/$(SERVER_NAME)_tcp_test


.PHONY: all
all: test_lib test_client test_server

.PHONY: test_lib
test_lib: erpcgen
	@$(call printmessage,build,Building, test_lib,gray,,,\n)
	@$(MAKE) $(silent_make) -j$(MAKETHREADS) -r -f $(TEST_DIR)/mk/test_lib.mk

# Unit Test Targets
.PHONY: test-tcp
test-tcp: test_lib test_client_tcp test_server_tcp

.PHONY: test-serial
test-serial: test_lib test_client_serial test_server_serial

.PHONY: fresh
fresh: clean all

.PHONY: test_client
test_client: test_client_tcp test_client_serial

.PHONY: test_client_tcp
test_client_tcp: erpcgen
	@$(call printmessage,build,Building, $(CUR_DIR) $@ ,gray,,,\n)
	@$(MAKE) $(silent_make) -j$(MAKETHREADS) -r -f $(TEST_DIR)/mk/test.mk TEST_NAME=$(CUR_DIR) TYPE=CLIENT TRANSPORT=tcp

.PHONY: test_client_serial
test_client_serial: erpcgen
	@$(call printmessage,build,Building, $(CUR_DIR) $@ ,gray,,,\n)
	@$(MAKE) $(silent_make) -j$(MAKETHREADS) -r -f $(TEST_DIR)/mk/test.mk TEST_NAME=$(CUR_DIR) TYPE=CLIENT TRANSPORT=serial

.PHONY: test_server
test_server: test_server_tcp test_server_serial

.PHONY: test_server_tcp
test_server_tcp: erpcgen
	@$(call printmessage,build,Building, $(CUR_DIR) $@ ,gray,,,\n)
	@$(MAKE) $(silent_make) -j$(MAKETHREADS) -r -f $(TEST_DIR)/mk/test.mk TEST_NAME=$(CUR_DIR) TYPE=SERVER TRANSPORT=tcp

.PHONY: test_server_serial
test_server_serial: erpcgen
	@$(call printmessage,build,Building, $(CUR_DIR) $@ ,gray,,,\n)
	@$(MAKE) $(silent_make) -j$(MAKETHREADS) -r -f $(TEST_DIR)/mk/test.mk TEST_NAME=$(CUR_DIR) TYPE=SERVER TRANSPORT=serial

.PHONY: erpcgen
erpcgen:
	@$(call printmessage,build,Building, $@ ,gray,,,\n)
	@$(MAKE) $(silent_make) -j$(MAKETHREADS) -r -C $(ERPC_ROOT)/erpcgen -f Makefile

.PHONY: test run-ut-server run-ut-client
test: all run-ut-server run-ut-client

run-tcp-server:
	@$(TCP_SERVER_PATH) &

run-tcp-client:
	@$(TCP_CLIENT_PATH) "--gtest_output=xml:$(TEST_DIR)/results/"

run-erpcgen: erpcgen
	@$(ERPCGEN_PATH) $(CUR_DIR).erpc

run-erpcgen-common: erpcgen
	@$(ERPCGEN_PATH) $(ERPC_ROOT)/src/unit_test_common/unit_test_common.erpc

#cleans only output directories related to this unit test
.PHONY: clean clean_serial clean_tcp
clean: clean_serial clean_tcp

clean_tcp:
	@echo Cleaning $(TEST_NAME)_tcp...
	@rm -rf $(UT_OUTPUT_DIR)/$(os_name)/tcp

clean_serial:
	@echo Cleaning $(TEST_NAME)_serial...
	@rm -rf $(UT_OUTPUT_DIR)/$(os_name)/serial
