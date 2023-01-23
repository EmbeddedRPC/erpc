define PKG_CONFIG
prefix=$(PREFIX)
exec_prefix=$${prefix}
includedir=$${prefix}/$(INC_INSTALL_DIR)
libdir=$${exec_prefix}/$(LIB_INSTALL_DIR)/$(arch_libdir)

Name: $(LIB_NAME)
Description: The embeddedRPC Library
Version: $(ERPC_VERSION)
Cflags: -I$${includedir}
Libs: -L$${libdir} -l$(LIB_NAME)
endef

#arch_libdir ?= x86_64-linux-gnu/
CFG_INSTALL_DIR := $(LIB_INSTALL_DIR)/$(arch_libdir)pkgconfig

ERPC_VERSION := $(shell sed -n 's/^\#define ERPC_VERSION .*"\([^"]*\)".*/\1/p' $(ERPC_C_ROOT)/infra/erpc_version.h)

$(LIB_NAME).pc:
	$(file > $@,$(PKG_CONFIG))
