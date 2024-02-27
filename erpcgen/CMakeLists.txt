#
# Copyright 2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#


set(ERPCGEN_ROOT ${ERPC_ROOT}/erpcgen)

# Paths to executable
set(PYTHON_SCRIPT ${ERPCGEN_ROOT}/bin/txt_to_c.py)
set(FLEX ${ERPCGEN_ROOT}/VisualStudio_v14/win_flex.exe)
set(BISON ${ERPCGEN_ROOT}/VisualStudio_v14/win_bison.exe)

# FLEX & BISON flags
set(YYFLAGS "-Wno-other")
set(LLFLAGS )

# erpcgen sources
set(ERPCGEN_SOURCE
    ${ERPCGEN_ROOT}/src/cpptemplate/cpptempl.cpp
    ${ERPCGEN_ROOT}/src/format_string.cpp
    ${ERPCGEN_ROOT}/src/HexValues.cpp
    ${ERPCGEN_ROOT}/src/Logging.cpp
    ${ERPCGEN_ROOT}/src/options.cpp
    ${ERPCGEN_ROOT}/src/SearchPath.cpp
    ${ERPCGEN_ROOT}/src/AstNode.cpp
    ${ERPCGEN_ROOT}/src/AstWalker.cpp
    ${ERPCGEN_ROOT}/src/UniqueIdChecker.cpp
    ${ERPCGEN_ROOT}/src/CGenerator.cpp
    ${ERPCGEN_ROOT}/src/PythonGenerator.cpp
    ${ERPCGEN_ROOT}/src/JavaGenerator.cpp
    ${ERPCGEN_ROOT}/src/erpcgen.cpp
    ${ERPCGEN_ROOT}/src/ErpcLexer.cpp
    ${ERPCGEN_ROOT}/src/Generator.cpp
    ${ERPCGEN_ROOT}/src/InterfaceDefinition.cpp
    ${ERPCGEN_ROOT}/src/SymbolScanner.cpp
    ${ERPCGEN_ROOT}/src/Token.cpp
    ${ERPCGEN_ROOT}/src/ParseErrors.cpp
    ${ERPCGEN_ROOT}/src/Utils.cpp
    ${ERPCGEN_ROOT}/src/types/Type.cpp
    ${ERPC_ROOT}/erpc_c/infra/erpc_crc16.cpp

    # Generated files
    ${CMAKE_CURRENT_BINARY_DIR}/erpcgen_parser.tab.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/erpcgen_lexer.cpp
)

set(ERPCGEN_INCLUDES
    ${ERPCGEN_ROOT}/src
    ${ERPCGEN_ROOT}/src/cpptemplate 
    ${ERPCGEN_ROOT}/src/types 
    ${ERPCGEN_ROOT}/VisualStudio_v14
    ${ERPC_ROOT}/erpc_c/infra

    # Include generated files
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Define templates source and destination folder
set(ERPCGEN_TEMPLATE_DIR ${ERPCGEN_ROOT}/src/templates)
set(ERPCGEN_TEMPLATE_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/src/templates)

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
        COMMAND python ${PYTHON_SCRIPT} -o ${ERPCGEN_TEMPLATE_OUT_DIR}/${c_file} 
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
    COMMAND ${FLEX} ${LLFLAGS} -o ${CMAKE_CURRENT_BINARY_DIR}/erpcgen_lexer.cpp ${ERPCGEN_ROOT}/src/erpcgen_lexer.l
)

# Run bison
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/erpcgen_parser.tab.cpp ${ERPCGEN_ROOT}/erpcgen_parser.tab.hpp
    COMMAND ${BISON} ${YYFLAGS} -o ${CMAKE_CURRENT_BINARY_DIR}/erpcgen_parser.tab.cpp ${ERPCGEN_ROOT}/src/erpcgen_parser.y
)

# Create executable
add_executable(erpcgen 
    ${ERPCGEN_SOURCE}
    ${ERPCGEN_TEMPLATES_C}
)

# Include directories
target_include_directories(erpcgen PRIVATE ${ERPCGEN_INCLUDES})
