#
# Copyright 2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Find executables
find_program(FLEX NAMES flex win_flex.exe HINTS ${ERPC_ERPCGEN}/VisualStudio_v14)
find_program(BISON NAMES bison win_bison.exe HINTS ${ERPC_ERPCGEN}/VisualStudio_v14)

if(NOT BISON)
    message(FATAL_ERROR "Executable 'bison' not found.")
endif()

if(NOT FLEX)
    message(FATAL_ERROR "Executable 'flex' not found.")
endif()

# FLEX & BISON flags
set(YYFLAGS "-Wno-other")
set(LLFLAGS "")

# Python text-to-c script
set(PYTHON_SCRIPT ${ERPC_ERPCGEN}/bin/txt_to_c.py)

# erpcgen sources
set(ERPCGEN_SOURCE
    ${ERPC_ERPCGEN}/src/cpptemplate/cpptempl.cpp
    ${ERPC_ERPCGEN}/src/format_string.cpp
    ${ERPC_ERPCGEN}/src/HexValues.cpp
    ${ERPC_ERPCGEN}/src/Logging.cpp
    ${ERPC_ERPCGEN}/src/options.cpp
    ${ERPC_ERPCGEN}/src/SearchPath.cpp
    ${ERPC_ERPCGEN}/src/AstNode.cpp
    ${ERPC_ERPCGEN}/src/AstWalker.cpp
    ${ERPC_ERPCGEN}/src/UniqueIdChecker.cpp
    ${ERPC_ERPCGEN}/src/CGenerator.cpp
    ${ERPC_ERPCGEN}/src/PythonGenerator.cpp
    ${ERPC_ERPCGEN}/src/JavaGenerator.cpp
    ${ERPC_ERPCGEN}/src/erpcgen.cpp
    ${ERPC_ERPCGEN}/src/ErpcLexer.cpp
    ${ERPC_ERPCGEN}/src/Generator.cpp
    ${ERPC_ERPCGEN}/src/InterfaceDefinition.cpp
    ${ERPC_ERPCGEN}/src/SymbolScanner.cpp
    ${ERPC_ERPCGEN}/src/Token.cpp
    ${ERPC_ERPCGEN}/src/ParseErrors.cpp
    ${ERPC_ERPCGEN}/src/Utils.cpp
    ${ERPC_ERPCGEN}/src/types/Type.cpp
    ${ERPC_BASE}/erpc_c/infra/erpc_crc16.cpp

    # Generated files
    ${CMAKE_CURRENT_BINARY_DIR}/erpcgen_parser.tab.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/erpcgen_lexer.cpp
)

set(ERPCGEN_INCLUDES
    ${ERPC_ERPCGEN}/src
    ${ERPC_ERPCGEN}/src/cpptemplate 
    ${ERPC_ERPCGEN}/src/types 
    ${ERPC_ERPCGEN}/VisualStudio_v14
    ${ERPC_BASE}/erpc_c/infra

    # Include generated files
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Define templates source and destination folder
set(ERPCGEN_TEMPLATE_DIR ${ERPC_ERPCGEN}/src/templates)
set(ERPCGEN_TEMPLATE_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/src/templates)

# List of all templates 
set(ERPCGEN_TEMPLATE_NAMES
    c_common_header.template
    cpp_interface_header.template
    cpp_interface_source.template
    cpp_client_header.template
    cpp_client_source.template
    cpp_server_header.template
    cpp_server_source.template
    cpp_coders.template
    cpp_common_functions.template
    c_client_header.template
    c_client_source.template
    c_server_header.template
    c_server_source.template
    c_crc.template
    py_init.template
    py_common.template
    py_client.template
    py_server.template
    py_interface.template
    py_coders.template
    py_global_init.template
    java_struct.template
    java_server.template
    java_interface.template
    java_enum.template
    java_const.template
    java_coders.template
    java_client.template
)

# For each name in ERPCGEN_TEMPLATE_NAMES add custom command
# that generates .c file in ERPCGEN_TEMPLATE_OUT_DIR folder
foreach(template ${ERPCGEN_TEMPLATE_NAMES})
    string(REGEX REPLACE "[.]template$" ".c" c_file ${template})

    add_custom_command(
        OUTPUT ${ERPCGEN_TEMPLATE_OUT_DIR}/${c_file}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${ERPCGEN_TEMPLATE_OUT_DIR}
        COMMAND ${PYTHON_EXECUTABLE} ${PYTHON_SCRIPT} -o ${ERPCGEN_TEMPLATE_OUT_DIR}/${c_file} 
                                           ${ERPCGEN_TEMPLATE_DIR}/${template}
        DEPENDS ${ERPCGEN_TEMPLATE_DIR}/${template}
        COMMENT "Generating ${template} -> ${c_file}"
    )

    # Add full path to .c file to the sources
    list(APPEND ERPCGEN_TEMPLATES_C ${ERPCGEN_TEMPLATE_OUT_DIR}/${c_file})
endforeach(template)

# Run flex
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/erpcgen_lexer.cpp
    COMMAND ${FLEX} ${LLFLAGS} -o ${CMAKE_CURRENT_BINARY_DIR}/erpcgen_lexer.cpp ${ERPC_ERPCGEN}/src/erpcgen_lexer.l
)

# Run bison
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/erpcgen_parser.tab.cpp ${CMAKE_CURRENT_BINARY_DIR}/erpcgen_parser.tab.hpp
    COMMAND ${BISON} ${YYFLAGS} -o ${CMAKE_CURRENT_BINARY_DIR}/erpcgen_parser.tab.cpp ${ERPC_ERPCGEN}/src/erpcgen_parser.y
)

# Create executable
add_executable(erpcgen 
    ${ERPCGEN_SOURCE}
    ${ERPCGEN_TEMPLATES_C}
)

# Include directories
target_include_directories(erpcgen PRIVATE ${ERPCGEN_INCLUDES})

# Set erpcgen to use C++ 17
set_property(TARGET erpcgen PROPERTY CXX_STANDARD 17)

# Link all libraries statically
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")

# Install erpcgen
install(TARGETS erpcgen)
