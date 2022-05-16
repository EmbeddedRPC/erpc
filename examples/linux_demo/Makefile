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

# cd ..../erpc
# make install PREFIX=/tmp/erpc
# make examples/linux_demo PREFIX=/tmp/erpc (Sometimes need to run twice??)
# example/linux_demo/test_erpcdemo_server	# start server
# example/linux_demo/test_erpcdemo_client	# in different window, start server
# > RD_demoHello response:hello!!!!!@2022-05-11 10:47:31
# make examples/linux_demo clean PREFIX=/tmp/erpc

include ../../mk/erpc_common.mk

MYFLAGS = -I $(ERPC_ROOT)/erpc_c/setup  -I $(ERPC_ROOT)/erpc_c/infra -I $(ERPC_ROOT)/erpc_c/port -I $(ERPC_ROOT)/erpc_c/config -I $(ERPC_ROOT)/erpc_c/setup -DTRANSPORT_TCP
CXXFLAGS = $(MYFLAGS)
CFLAGS = $(MYFLAGS)

%.o : %.cpp
	@echo $@
	g++ -c $(CPPFLAGS) $(CXXFLAGS) $< -o $@

%.o : %.c
	@echo $@
	gcc -c $(CFLAGS) $(CFLAGS) $< -o $@

PROG_PREFIX = erpcdemo
SERVER_OBJS = $(PROG_PREFIX)_server.o test_server.o
CLIENT_OBJS = $(PROG_PREFIX)_client.o test_client.o
GENERATED_FILES = $(PROG_PREFIX).h $(PROG_PREFIX)_client.cpp $(PROG_PREFIX)_server.cpp $(PROG_PREFIX)_server.h

RPC_LIB = $(PREFIX)/lib/liberpc.a
	
default: all

all: test_server test_client
	
$(PREFIX)/bin/erpcgen:
	@echo erpcgen not found.
	@echo "Run 'make install PREFIX=$(PREFIX)'"

erpcdemo.h: $(PREFIX)/bin/erpcgen
	@echo Generate files using $(PREFIX)/bin/erpcgen
	$(PREFIX)/bin/erpcgen -g c erpcdemo.erpc
	
test_server: erpcdemo.h $(SERVER_OBJS)
	@echo Create $@
	g++ -o $@ $(SERVER_OBJS) $(RPC_LIB) -pthread

test_client: erpcdemo.h $(CLIENT_OBJS)
	@echo Create $@
	g++ -o $@ $(CLIENT_OBJS) $(RPC_LIB) -pthread

clean:
	@echo rm $(SERVER_OBJS) $(CLIENT_OBJS) test_client test_server
	@echo rm -f $(GENERATED_FILES)
	rm -f $(SERVER_OBJS) $(CLIENT_OBJS) test_client test_server
	rm -f $(GENERATED_FILES)
#	rm -rf $(PREFIX)

