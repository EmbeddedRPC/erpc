#-------------------------------------------------------------------------------
# Copyright (C) 2016 Freescale Semiconductor, Inc.
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

SOURCES +=  $(ERPC_OUT_DIR)/$(ERPC_NAME)_firstInterface_$(APP_TYPE).cpp \
            $(ERPC_OUT_DIR)/$(ERPC_NAME)_secondInterface_server.cpp\
            $(CUR_DIR)_$(APP_TYPE)_impl.cpp \
            $(UT_COMMON_SRC)/unit_test_$(TRANSPORT)_arbitrator_$(APP_TYPE).cpp

.PHONY: all
all: $(ERPC_OUT_DIR)/$(ERPC_NAME)_firstInterface_$(APP_TYPE).cpp $(ERPC_OUT_DIR)/$(ERPC_NAME)_secondInterface_server.cpp

# Define dependency.
$(OUTPUT_ROOT)/test/$(TEST_NAME)/$(CUR_DIR)_$(APP_TYPE)_impl.cpp: $(ERPC_OUT_DIR)/$(ERPC_NAME)_firstInterface_$(APP_TYPE).cpp $(ERPC_OUT_DIR)/$(ERPC_NAME)_secondInterface_server.cpp
$(UT_COMMON_SRC)/unit_test_$(TRANSPORT)_arbitrator_$(APP_TYPE).cpp: $(ERPC_OUT_DIR)/$(ERPC_NAME)_firstInterface_$(APP_TYPE).cpp $(ERPC_OUT_DIR)/$(ERPC_NAME)_secondInterface_server.cpp
$(ERPC_OUT_DIR)/$(ERPC_NAME)_firstInterface_$(APP_TYPE).cpp: $(ERPC_OUT_DIR)/$(ERPC_NAME)_secondInterface_server.cpp

# Run erpcgen for C.
$(ERPC_OUT_DIR)/$(ERPC_NAME)_secondInterface_server.cpp: $(IDL_FILE)
	@$(call printmessage,orange,Running erpcgen-c $(TEST_NAME), $(subst $(ERPC_ROOT)/,,$<))
	$(at)$(ERPCGEN) -gc -o $(RPC_OBJS_ROOT)/ $(IDL_FILE)
