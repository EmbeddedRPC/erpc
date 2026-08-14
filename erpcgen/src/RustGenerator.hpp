/*
 * Copyright (c) 2024-2025 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__RUSTGENERATOR_H_
#define _EMBEDDED_RPC__RUSTGENERATOR_H_

#include "Generator.hpp"
#include "cpptempl.hpp"

#include <set>
#include <string>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpcgen {

/*!
 * @brief Code generator for Rust.
 */
class RustGenerator : public Generator {
public:
  /*!
   * @brief This function is constructor of RustGenerator class.
   *
   * @param[in] def Contains all Symbols parsed from IDL files.
   */
  explicit RustGenerator(InterfaceDefinition *def);

  /*!
   * @brief This function is destructor of RustGenerator class.
   *
   * This function close opened files.
   */
  virtual ~RustGenerator() {}

  /*!
   * @brief This function generate output code for output files.
   *
   * This code call all necessary functions for prepare output code and parse it
   * into output files.
   */
  virtual void generate() override;

protected:
  /*!
   * @brief This function sets group symbols template data.
   *
   * This function sets group symbols template data with all data, which
   * are necessary for generating output code for output files.
   *
   * @param[in] group Pointer to a group.
   *
   * @return Data map with group symbols templates.
   */
  virtual cpptempl::data_map
  makeGroupSymbolsTemplateData(Group *group) override;

  /*!
   * @brief This function return interface function template data.
   *
   * This function return interface function template data with all data, which
   * are necessary for generating output code for output files.
   *
   * @param[in] group Pointer to a group.
   * @param[in] fn From this are set interface function template data.
   *
   * @return Contains interface function data.
   */
  virtual cpptempl::data_map getFunctionTemplateData(Group *group,
                                                     Function *fn) override;

  /*!
   * @brief This function returns function type (callbacks type) template data.
   *
   * This function returns function type (callbacks type) template data with all
   * data, which are necessary for generating output code for output files. Shim
   * code is generating common function for serialization/deserialization of
   * data.
   *
   * @param[in] group Group to which function belongs.
   * @param[in] fn From this are set function type template data.
   *
   * @return Contains interface function data.
   */
  virtual cpptempl::data_map
  getFunctionTypeTemplateData(Group *group, FunctionType *fn) override;

  /*!
   * @brief This function will get symbol comments and convert to language
   * specific ones
   *
   * @param[in] symbol Pointer to symbol.
   * @param[inout] symbolInfo Data map, which contains information about symbol.
   */
  virtual void setTemplateComments(Symbol *symbol,
                                   cpptempl::data_map &symbolInfo) override;

  /*!
   * @brief This function generates output files.
   *
   * This function call functions for generating client/server header/source
   * files.
   *
   * @param[in] fileNameExtension Extension for file name (for example for case
   * that each interface will be generated in its set of output files).
   */
  virtual void
  generateOutputFiles(const std::string &fileNameExtension) override;

  /*!
   * @brief This function return interface function prototype.
   *
   * @param[in] group Group to which function belongs.
   * @param[in] fn Function for prototyping.
   * @param[in] interfaceName Interface name used for function declaration.
   * @param[in] name Name used for shared code in case of function type.
   * @param[in] insideInterfaceCall interfaceClass specific.
   *
   * @return String prototype representation for given function.
   */
  virtual std::string getFunctionPrototype(
      Group *group, FunctionBase *fn, const std::string &interfaceName = "",
      const std::string &name = "", bool insideInterfaceCall = false) override;

  /*!
   * @brief This function returns Rust equivalent name for given data type.
   *
   * @param[in] dataType Given data type.
   *
   * @return String with Rust equivalent name for given data type.
   */
  std::string getTypeString(DataType *dataType);

  /*!
   * @brief This function returns information if given data type is list.
   *
   * @param[in] dataType Given data type.
   *
   * @retval true Given data type is list.
   * @retval false Given data type is not list.
   */
  bool isListType(DataType *dataType);

  /*!
   * @brief This function returns information if given data type is string.
   *
   * @param[in] dataType Given data type.
   *
   * @retval true Given data type is string.
   * @retval false Given data type is not string.
   */
  bool isStringType(DataType *dataType);

  /*!
   * @brief This function returns direction for function parameter.
   *
   * @param[in] param Function parameter.
   *
   * @return Direction of function parameter.
   */
  param_direction_t getDirection(StructMember *param);

  /*!
   * @brief This function returns direction string for function parameter.
   *
   * @param[in] param Function parameter.
   *
   * @return Direction string of function parameter.
   */
  std::string getDirectionString(StructMember *param);

  /*!
   * @brief This function generates code to write a parameter to the codec.
   *
   * @param[in] param Function parameter to serialize.
   * @param[in] paramName Name of the parameter variable.
   *
   * @return Code string for writing the parameter to codec.
   */
  std::string generateParamWrite(StructMember *param,
                                 const std::string &paramName);

  /*!
   * @brief This function generates code to read a type from the codec.
   *
   * @param[in] dataType Data type to deserialize.
   * @param[in] codecName Name of the codec variable.
   *
   * @return Code string for reading the type from codec.
   */
  std::string generateTypeRead(DataType *dataType,
                               const std::string &codecName);

  /*!
   * @brief This function generates code to write a list item to the codec.
   *
   * @param[in] elementType Type of the list element.
   * @param[in] itemName Name of the item variable.
   *
   * @return Code string for writing the list item to codec.
   */
  std::string generateListItemWrite(DataType *elementType,
                                    const std::string &itemName);

  /*!
   * @brief This function generates code to write a data type to the codec.
   *
   * @param[in] dataType Data type to serialize.
   * @param[in] paramName Name of the parameter variable.
   *
   * @return Code string for writing the data type to codec.
   */
  std::string generateParamWriteForDataType(DataType *dataType,
                                            const std::string &paramName);

  /*!
   * @brief This function generates code to write a struct member to the codec.
   *
   * @param[in] member Struct member to serialize.
   * @param[in] memberName Name of the member variable.
   *
   * @return Code string for writing the member to codec.
   */
  std::string generateMemberWrite(StructMember *member,
                                  const std::string &memberName);

private:
  std::set<std::string>
      m_rustKeywords; /*!< Set of Rust keywords that need to be escaped. */

  /*!
   * @brief This function initialize Rust keywords set.
   */
  void initRustKeywords();

  /*!
   * @brief This function escapes Rust keywords.
   *
   * @param[in] name Input name to check against Rust keywords.
   *
   * @return Escaped name if it was a Rust keyword, original name otherwise.
   */
  std::string escapeKeyword(const std::string &name);

  /*!
   * @brief This function converts PascalCase/camelCase to snake_case.
   *
   * @param[in] name Input name in PascalCase or camelCase.
   *
   * @return snake_case version of the name.
   */
  std::string toSnakeCase(const std::string &name);

  /*!
   * @brief This function converts PascalCase/camelCase to UPPER_SNAKE_CASE.
   *
   * @param[in] name Input name in PascalCase or camelCase.
   *
   * @return UPPER_SNAKE_CASE version of the name.
   */
  std::string toUpperSnakeCase(const std::string &name);

  /*!
   * @brief This function returns the default value for a given data type in
   * Rust.
   *
   * @param[in] dataType Given data type.
   *
   * @return String with Rust default value for given data type.
   */
  std::string getDefaultValue(DataType *dataType);

  /*!
   * @brief This function determines the optimal Rust repr type for an enum
   * based on its values.
   *
   * @param[in] enumType Enum type from lexical analyzer input.
   *
   * @return String with optimal Rust repr type (u8, u16, u32, etc.).
   */
  std::string determineEnumReprType(EnumType *enumType);

  /*!
   * @brief This function returns the codec method name for a given repr type.
   *
   * @param[in] reprType Rust repr type (u8, u16, u32, etc.).
   *
   * @return String with codec method name (uint8, uint16, uint32, etc.).
   */
  std::string getCodecMethodForReprType(const std::string &reprType);

  /*!
   * @brief This function generates the client method request serialization
   * code.
   *
   * @param[in] fn Function to generate request serialization for.
   * @param[in] inParams Vector of input parameters.
   *
   * @return String with request serialization code.
   */
  std::string generateClientRequestSerialization(
      Function *fn, const std::vector<StructMember *> &inParams);

  /*!
   * @brief This function generates the client method response deserialization
   * code.
   *
   * @param[in] fn Function to generate response deserialization for.
   * @param[in] outParamTypes Vector of output parameter types.
   * @param[in] hasOutParams Whether the function has output parameters.
   *
   * @return String with response deserialization code.
   */
  std::string generateClientResponseDeserialization(
      Function *fn, const std::vector<DataType *> &outParamTypes,
      const std::vector<std::string> &outParamNames, bool hasOutParams);

  /*!
   * @brief This function generates code to read a single output parameter.
   *
   * @param[in] paramType Type of the output parameter.
   * @param[in] paramIndex Index of the parameter (for naming).
   *
   * @return String with output parameter read code.
   */
  std::string generateOutParamRead(DataType *paramType,
                                   const std::string &paramName);

  /*!
   * @brief This function determines the appropriate codec name for a data type.
   *
   * @param[in] dataType Type to determine codec name for.
   * @param[in] baseCodecName Base codec variable name.
   *
   * @return String with appropriate codec reference.
   */
  std::string getCodecNameForType(DataType *dataType,
                                  const std::string &baseCodecName);

  /*!
   * @brief This function generates server-side parameter deserialization code.
   *
   * @param[in] fn Function to generate parameter deserialization for.
   * @param[in] codecParamName Name of the codec parameter to use.
   *
   * @return String with parameter deserialization code.
   */
  std::string
  generateServerParameterDeserialization(Function *fn,
                                         const std::string &codecParamName);

  /*!
   * @brief This function generates server-side response serialization code.
   *
   * @param[in] fn Function to generate response serialization for.
   * @param[in] codecParamName Name of the codec parameter to use.
   *
   * @return String with response serialization code.
   */
  std::string
  generateServerResponseSerialization(Function *fn,
                                      const std::string &codecParamName);

  /*!
   * @brief This function generates server-side error response serialization
   * code with default out parameters.
   *
   * @param[in] fn Function to generate error response serialization for.
   * @param[in] codecParamName Name of the codec parameter to use.
   *
   * @return String with error response serialization code.
   */
  std::string
  generateServerErrorResponseSerialization(Function *fn,
                                           const std::string &codecParamName);

  /*!
   * @brief This function generates code to write default error values for a
   * specific type.
   *
   * @param[in] dataType Type to generate default error value for.
   *
   * @return String with default error value write code.
   */
  std::string generateDefaultErrorWrite(DataType *dataType);

  /*!
   * @brief This function generates code to write a value of a specific type to
   * the codec.
   *
   * @param[in] dataType Type of the value to write.
   * @param[in] variableName Name of the variable to write.
   *
   * @return String with type-specific write code.
   */
  std::string generateTypeWrite(DataType *dataType,
                                const std::string &variableName);

  /*!
   * @brief This function converts a name to PascalCase.
   *
   * @param[in] name Name to convert.
   *
   * @return String with PascalCase conversion.
   */
  std::string toPascalCase(const std::string &name);

  /*!
   * @brief This function collects @length annotated parameters to skip.
   *
   * @param[in] fn Function to process.
   *
   * @return Set of parameter names that are referenced by @length annotations.
   */
  std::set<std::string> collectLengthParams(Function *fn);

  /*!
   * @brief This function processes function parameters and builds parameter
   * lists.
   *
   * @param[in] fn Function to process.
   * @param[in] lengthParams Set of @length parameters to skip.
   * @param[out] params Template data list for parameters.
   * @param[out] asyncParams String stream for async parameters.
   * @param[out] clientParams String stream for client parameters.
   * @param[out] outParamNames Vector of output parameter names.
   * @param[out] outParamTypes Vector of output parameter types.
   *
   * @return True if function has output parameters.
   */
  bool processFunctionParameters(Function *fn,
                                 const std::set<std::string> &lengthParams,
                                 cpptempl::data_list &params,
                                 std::stringstream &asyncParams,
                                 std::stringstream &clientParams,
                                 std::vector<std::string> &outParamNames,
                                 std::vector<DataType *> &outParamTypes);

  /*!
   * @brief This function generates parameter call list for function invocation.
   *
   * @param[in] fn Function to process.
   *
   * @return String with comma-separated parameter names.
   */
  std::string generateParameterCallList(Function *fn);

  /*!
   * @brief This function determines return type strings for function.
   *
   * @param[in] fn Function to process.
   * @param[in] hasOutParams Whether function has output parameters.
   * @param[in] outParamTypes Vector of output parameter types.
   * @param[out] rustReturnType String for trait return type.
   * @param[out] clientReturnType String for client return type.
   */
  void determineReturnTypes(Function *fn, bool hasOutParams,
                            const std::vector<DataType *> &outParamTypes,
                            std::string &rustReturnType,
                            std::string &clientReturnType);

  /*!
   * @brief This function generates server handler code for oneway functions.
   *
   * @param[in] fn Function to process.
   * @param[in] codecParamName Name of codec parameter.
   *
   * @return String with server handler code.
   */
  std::string generateOnewayServerHandler(Function *fn,
                                          const std::string &codecParamName);

  /*!
   * @brief This function generates server handler code for regular (non-oneway)
   * functions.
   *
   * @param[in] fn Function to process.
   * @param[in] codecParamName Name of codec parameter.
   * @param[in] sequenceParamName Name of sequence parameter.
   *
   * @return String with server handler code.
   */
  std::string
  generateRegularServerHandler(Function *fn, const std::string &codecParamName,
                               const std::string &sequenceParamName);

  /*!
   * @brief This function generates method call parameters for server handler.
   *
   * @param[in] fn Function to process.
   * @param[in] lengthParams Set of @length parameters to skip.
   *
   * @return String with method call parameters.
   */
  std::string
  generateMethodCallParameters(Function *fn,
                               const std::set<std::string> &lengthParams);

  /*!
   * @brief This function generates client method code.
   *
   * @param[in] fn Function to process.
   * @param[out] outParamTypes Vector to store output parameter types.
   * @param[out] hasOutParams Reference to store whether function has output
   * parameters.
   *
   * @return String with client method code.
   */
  std::string generateClientMethodCode(Function *fn,
                                       std::vector<DataType *> &outParamTypes,
                                       bool &hasOutParams);

  /*!
   * @brief This function processes constants from group symbols.
   *
   * @param[in] group Group to process constants from.
   *
   * @return Template data list for constants.
   */
  cpptempl::data_list processGroupConstants(Group *group);

  /*!
   * @brief This function processes type aliases from group symbols.
   *
   * @param[in] group Group to process type aliases from.
   *
   * @return Template data list for type aliases.
   */
  cpptempl::data_list processGroupTypeAliases(Group *group);

  /*!
   * @brief This function processes interfaces from group.
   *
   * @param[in] group Group to process interfaces from.
   *
   * @return Template data list for interfaces.
   */
  cpptempl::data_list processGroupInterfaces(Group *group);

  /*!
   * @brief This function processes enums from group symbols.
   *
   * @param[in] group Group to process enums from.
   *
   * @return Template data list for enums.
   */
  cpptempl::data_list processGroupEnums(Group *group);

  /*!
   * @brief This function processes structs from group symbols.
   *
   * @param[in] group Group to process structs from.
   *
   * @return Template data list for structs.
   */
  cpptempl::data_list processGroupStructs(Group *group);

  /*!
   * @brief This function generates codec write operation for a builtin type.
   *
   * @param[in] builtinType The builtin type to generate write operation for.
   * @param[in] codecName Name of the codec variable.
   * @param[in] variableName Name of the variable to write.
   * @param[in] addErrorHandling Whether to add ? for error handling.
   *
   * @return String with codec write operation.
   */
  std::string generateBuiltinTypeWrite(BuiltinType *builtinType,
                                       const std::string &codecName,
                                       const std::string &variableName,
                                       bool addErrorHandling = true);

  /*!
   * @brief This function generates codec read operation for a builtin type.
   *
   * @param[in] builtinType The builtin type to generate read operation for.
   * @param[in] codecName Name of the codec variable.
   *
   * @return String with codec read operation.
   */
  std::string generateBuiltinTypeRead(BuiltinType *builtinType,
                                      const std::string &codecName);

  /*!
   * @brief This function generates default value write operation for a builtin
   * type.
   *
   * @param[in] builtinType The builtin type to generate default write for.
   * @param[in] codecName Name of the codec variable.
   *
   * @return String with default value write operation.
   */
  std::string generateBuiltinTypeDefaultWrite(BuiltinType *builtinType,
                                              const std::string &codecName);
};

} // namespace erpcgen

#endif // _EMBEDDED_RPC__RUSTGENERATOR_H_
