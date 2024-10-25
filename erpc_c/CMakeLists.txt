#
# Copyright 2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

include(erpc_c_sources.cmake)
# Create erpc library
add_library(erpc STATIC ${ERPC_C_SOURCES})
set_target_properties(erpc PROPERTIES PUBLIC_HEADER "${ERPC_C_HEADERS}")

target_include_directories(erpc PUBLIC ${ERPC_C_INCLUDES} ${ERPC_C}/config)

# Required for TCP transport
if(WIN32)
	target_link_libraries(erpc PRIVATE wsock32 ws2_32)
endif()

# Install erpc as library
install(TARGETS erpc PUBLIC_HEADER DESTINATION include/erpc/)



