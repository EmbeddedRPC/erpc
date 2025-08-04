/*
 * Copyright (c) 2024-2025 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "RustGenerator.hpp"
#include "Logging.hpp"
#include "ParseErrors.hpp"
#include "annotations.h"
#include "erpc_version.h"
#include "format_string.hpp"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <set>
#include <sstream>

using namespace erpcgen;
using namespace cpptempl;
using namespace std;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

// Templates strings converted from text files by txt_to_c.py.
extern const char *const kRustTemplate;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

RustGenerator::RustGenerator(InterfaceDefinition *def)
    : Generator(def, generator_type_t::kRust) {
  /* Set copyright rules. */
  m_templateData["erpcVersion"] = ERPC_VERSION;
  m_templateData["todaysDate"] = getTime();

  initRustKeywords();
}

void RustGenerator::initRustKeywords() {
  // Rust keywords that need to be escaped
  const char *keywords[] = {
      "as",      "break",  "const",    "continue", "crate",  "else",
      "enum",    "extern", "false",    "fn",       "for",    "if",
      "impl",    "in",     "let",      "loop",     "match",  "mod",
      "move",    "mut",    "pub",      "ref",      "return", "self",
      "Self",    "static", "struct",   "super",    "trait",  "true",
      "type",    "unsafe", "use",      "where",    "while",  "async",
      "await",   "dyn",    "abstract", "become",   "box",    "do",
      "final",   "macro",  "override", "priv",     "typeof", "unsized",
      "virtual", "yield",  "try",      "union"};

  for (const char *keyword : keywords) {
    m_rustKeywords.insert(keyword);
  }
}

string RustGenerator::escapeKeyword(const string &name) {
  if (m_rustKeywords.find(name) != m_rustKeywords.end()) {
    return "r#" + name;
  }
  return name;
}

string RustGenerator::toSnakeCase(const string &name) {
  string result;
  bool previousWasUpper = false;

  for (size_t i = 0; i < name.length(); ++i) {
    char c = name[i];

    if (isupper(c)) {
      // Insert underscore before uppercase letter if:
      // 1. It's not the first character AND
      // 2. The previous character was lowercase OR the next character is
      // lowercase
      if (i > 0 && (!previousWasUpper ||
                    (i + 1 < name.length() && islower(name[i + 1])))) {
        result += '_';
      }
      result += tolower(c);
      previousWasUpper = true;
    } else {
      result += c;
      previousWasUpper = false;
    }
  }

  return result;
}

string RustGenerator::toUpperSnakeCase(const string &name) {
  string snakeCase = toSnakeCase(name);
  transform(snakeCase.begin(), snakeCase.end(), snakeCase.begin(), ::toupper);
  return snakeCase;
}

string RustGenerator::toPascalCase(const string &name) {
  string result;
  bool capitalizeNext = true;

  for (size_t i = 0; i < name.length(); ++i) {
    char c = name[i];

    if (c == '_') {
      capitalizeNext = true;
    } else if (capitalizeNext) {
      result += toupper(c);
      capitalizeNext = false;
    } else if (isupper(c)) {
      // Handle camelCase -> PascalCase: preserve uppercase letters as word
      // boundaries
      result += c;
      capitalizeNext = false;
    } else {
      result += tolower(c);
    }
  }

  return result;
}

string RustGenerator::getDefaultValue(DataType *dataType) {
  if (!dataType) {
    return "Default::default()";
  }

  switch (dataType->getDataType()) {
  case DataType::data_type_t::kBuiltinType: {
    BuiltinType *builtinType = dynamic_cast<BuiltinType *>(dataType);
    switch (builtinType->getBuiltinType()) {
    case BuiltinType::builtin_type_t::kBoolType:
      return "false";
    case BuiltinType::builtin_type_t::kInt8Type:
    case BuiltinType::builtin_type_t::kInt16Type:
    case BuiltinType::builtin_type_t::kInt32Type:
    case BuiltinType::builtin_type_t::kInt64Type:
    case BuiltinType::builtin_type_t::kUInt8Type:
    case BuiltinType::builtin_type_t::kUInt16Type:
    case BuiltinType::builtin_type_t::kUInt32Type:
    case BuiltinType::builtin_type_t::kUInt64Type:
      return "0";
    case BuiltinType::builtin_type_t::kFloatType:
    case BuiltinType::builtin_type_t::kDoubleType:
      return "0.0";
    case BuiltinType::builtin_type_t::kStringType:
      return "String::new()";
    case BuiltinType::builtin_type_t::kBinaryType:
      return "Vec::new()";
    default:
      return "Default::default()";
    }
  }
  case DataType::data_type_t::kArrayType:
  case DataType::data_type_t::kListType:
    return "Vec::new()";
  case DataType::data_type_t::kStructType:
  case DataType::data_type_t::kEnumType:
  case DataType::data_type_t::kUnionType:
    return escapeKeyword(dataType->getName()) + "::default()";
  case DataType::data_type_t::kVoidType:
    return "()";
  case DataType::data_type_t::kAliasType: {
    AliasType *aliasType = dynamic_cast<AliasType *>(dataType);
    return getDefaultValue(aliasType->getElementType());
  }
  default:
    return "Default::default()";
  }
}

void RustGenerator::generate() {
  findGroupDataTypes();

  for (Group *group : m_groups) {
    Log::info("Generating Rust code for group: %s\n", group->getName().c_str());

    // Set the group's template data
    group->setTemplate(makeGroupSymbolsTemplateData(group));

    generateGroupOutputFiles(group);
  }
}

void RustGenerator::generateOutputFiles(const string &fileNameExtension) {
  // Add program multiline comment if it exists
  if (m_def->hasProgramSymbol()) {
    Program *program = m_def->getProgramSymbol();
    m_templateData["mlComment"] = program->getMlComment();
  } else {
    m_templateData["mlComment"] = "";
  }

  // Convert filename to snake_case for Rust naming conventions
  string rustFileName = toSnakeCase(fileNameExtension);

  // For Rust, we generate a single .rs file
  generateOutputFile(rustFileName + ".rs", "rust_template", m_templateData,
                     kRustTemplate);
}

cpptempl::data_map RustGenerator::makeGroupSymbolsTemplateData(Group *group) {
  data_map groupSymbols;

  Log::info("Generating symbols template data for group %s\n",
            group->getName().c_str());

  groupSymbols["name"] = group->getName();
  groupSymbols["id"] = 0; // Groups don't have unique IDs like interfaces

  // Process each symbol type
  groupSymbols["constants"] = processGroupConstants(group);
  groupSymbols["typeAliases"] = processGroupTypeAliases(group);
  groupSymbols["interfaces"] = processGroupInterfaces(group);
  groupSymbols["enums"] = processGroupEnums(group);
  groupSymbols["structs"] = processGroupStructs(group);

  return groupSymbols;
}

cpptempl::data_map RustGenerator::getFunctionTemplateData(Group *group,
                                                          Function *fn) {
  data_map functionInfo;

  functionInfo["name"] = toSnakeCase(fn->getName());
  functionInfo["id"] = fn->getUniqueId();
  functionInfo["constName"] = toPascalCase(fn->getName());
  functionInfo["handlerName"] = toPascalCase(fn->getName()) + "Handler";
  functionInfo["prototype"] = getFunctionPrototype(group, fn);

  setTemplateComments(fn, functionInfo);

  // Build parameters string for async trait method
  stringstream asyncParams;
  stringstream clientParams;

  // Collect @length annotated parameters to skip in trait method signatures
  std::set<std::string> lengthParams = collectLengthParams(fn);

  // Process parameters
  data_list params;
  vector<string> outParamNames;
  vector<DataType *> outParamTypes;
  bool hasOutParams =
      processFunctionParameters(fn, lengthParams, params, asyncParams,
                                clientParams, outParamNames, outParamTypes);

  functionInfo["parameters"] = asyncParams.str();
  functionInfo["clientParameters"] = clientParams.str();
  functionInfo["paramCallList"] = generateParameterCallList(fn);

  // Return type handling
  string rustReturnType;
  string clientReturnType;
  determineReturnTypes(fn, hasOutParams, outParamTypes, rustReturnType,
                       clientReturnType);

  functionInfo["returnType"] = rustReturnType;
  functionInfo["clientReturnType"] = clientReturnType;

  // Determine if codec parameter should be prefixed with underscore
  // This happens when method has no parameters and no response
  bool hasParamsForCodec = !fn->getParameters().getMembers().empty();
  DataType *funcReturnType = fn->getReturnType();
  bool hasReturnValue = funcReturnType && funcReturnType->getDataType() !=
                                              DataType::data_type_t::kVoidType;
  bool hasResponse = !fn->isOneway() || hasReturnValue;

  bool shouldPrefixCodec = !hasParamsForCodec && !hasResponse;
  string codecParamName = shouldPrefixCodec ? "_codec" : "codec";
  functionInfo["codecParam"] = codecParamName;

  // Determine if sequence parameter should be prefixed with underscore
  // This happens for oneway methods (they don't send responses so sequence is
  // unused)
  bool shouldPrefixSequence = fn->isOneway();
  string sequenceParamName = shouldPrefixSequence ? "_sequence" : "sequence";
  functionInfo["sequenceParam"] = sequenceParamName;

  // Generate server handler code
  string serverHandlerCode;
  if (fn->isOneway()) {
    serverHandlerCode = generateOnewayServerHandler(fn, codecParamName);
  } else {
    serverHandlerCode =
        generateRegularServerHandler(fn, codecParamName, sequenceParamName);
  }
  functionInfo["serverHandlerCode"] = serverHandlerCode;

  // Generate client method code
  vector<DataType *> clientOutParamTypes;
  bool clientHasOutParams;
  string clientMethodCode =
      generateClientMethodCode(fn, clientOutParamTypes, clientHasOutParams);
  functionInfo["clientMethodCode"] = clientMethodCode;
  functionInfo["params"] = params;

  return functionInfo;
}

cpptempl::data_map
RustGenerator::getFunctionTypeTemplateData(Group *group, FunctionType *fn) {
  // For callback/function types - similar to getFunctionTemplateData but for
  // function pointers
  data_map functionTypeInfo;

  functionTypeInfo["name"] = escapeKeyword(fn->getName());

  setTemplateComments(fn, functionTypeInfo);

  // Add parameters
  data_list params;
  for (auto param : fn->getParameters().getMembers()) {
    data_map paramInfo;
    paramInfo["name"] = escapeKeyword(toSnakeCase(param->getName()));
    paramInfo["type"] = getTypeString(param->getDataType());
    paramInfo["direction"] = getDirectionString(param);
    setTemplateComments(param, paramInfo);
    params.push_back(paramInfo);
  }
  functionTypeInfo["parameters"] = params;

  // Return type
  DataType *returnType = fn->getReturnType();
  if (returnType &&
      returnType->getDataType() != DataType::data_type_t::kVoidType) {
    functionTypeInfo["returnType"] = getTypeString(returnType);
    functionTypeInfo["hasReturn"] = true;
  } else {
    functionTypeInfo["hasReturn"] = false;
  }

  return functionTypeInfo;
}

void RustGenerator::setTemplateComments(Symbol *symbol,
                                        cpptempl::data_map &symbolInfo) {
  string comment = symbol->getDescription();
  if (!comment.empty()) {
    // Convert to Rust doc comments
    if (symbol->isDatatypeSymbol()) {
      // For datatypes (structs, enums, etc.), use outer doc comments (///)
      // Convert inner doc comments (//!) to outer doc comments (///)
      string mlComment = symbol->getMlComment();
      // Only replace the beginning of mlComment with "///" if it starts with
      // "//!"
      if (mlComment.rfind("//!", 0) == 0) {
        mlComment.replace(0, 3, "///");
      }
      symbolInfo["comment"] = mlComment;

    } else {
      symbolInfo["comment"] = "/// " + comment;
    }
  } else {
    symbolInfo["comment"] = "";
  }
}

string RustGenerator::getFunctionPrototype(Group *group, FunctionBase *fn,
                                           const string &interfaceName,
                                           const string &name,
                                           bool insideInterfaceCall) {
  stringstream prototype;

  // Cast fn to Function to access getName() - FunctionBase doesn't have
  // getName()
  Function *function = dynamic_cast<Function *>(fn);
  if (function) {
    prototype << "fn " << escapeKeyword(function->getName()) << "(";
  } else {
    // Fallback for non-Function types
    prototype << "fn unknown_function(";
  }

  // Add parameters
  bool first = true;
  for (auto param : fn->getParameters().getMembers()) {
    if (!first) {
      prototype << ", ";
    }
    first = false;

    prototype << escapeKeyword(toSnakeCase(param->getName())) << ": ";

    param_direction_t dir = getDirection(param);
    DataType *paramType = param->getDataType();

    if (dir == param_direction_t::kOutDirection ||
        dir == param_direction_t::kInoutDirection) {
      prototype << "&mut ";
      prototype << getTypeString(paramType);
    } else {
      // For input parameters, determine the correct parameter type
      if (paramType->getDataType() == DataType::data_type_t::kAliasType) {
        AliasType *aliasType = dynamic_cast<AliasType *>(paramType);
        if (isStringType(aliasType->getElementType())) {
          // String alias types use the alias name directly (owned)
          prototype << escapeKeyword(paramType->getName());
        } else {
          // Non-string alias types use the alias name directly (owned)
          prototype << escapeKeyword(paramType->getName());
        }
      } else if (isStringType(paramType)) {
        // Regular strings use references
        prototype << "&" << getTypeString(paramType);
      } else {
        // All other types (primitives, structs, enums) use owned values
        prototype << getTypeString(paramType);
      }
    }
  }

  prototype << ")";

  // Return type
  DataType *returnType = fn->getReturnType();
  if (returnType &&
      returnType->getDataType() != DataType::data_type_t::kVoidType) {
    prototype << " -> " << getTypeString(returnType);
  }

  return prototype.str();
}

string RustGenerator::getTypeString(DataType *dataType) {
  if (!dataType) {
    return "()";
  }

  switch (dataType->getDataType()) {
  case DataType::data_type_t::kBuiltinType: {
    BuiltinType *builtinType = dynamic_cast<BuiltinType *>(dataType);
    switch (builtinType->getBuiltinType()) {
    case BuiltinType::builtin_type_t::kBoolType:
      return "bool";
    case BuiltinType::builtin_type_t::kInt8Type:
      return "i8";
    case BuiltinType::builtin_type_t::kInt16Type:
      return "i16";
    case BuiltinType::builtin_type_t::kInt32Type:
      return "i32";
    case BuiltinType::builtin_type_t::kInt64Type:
      return "i64";
    case BuiltinType::builtin_type_t::kUInt8Type:
      return "u8";
    case BuiltinType::builtin_type_t::kUInt16Type:
      return "u16";
    case BuiltinType::builtin_type_t::kUInt32Type:
      return "u32";
    case BuiltinType::builtin_type_t::kUInt64Type:
      return "u64";
    case BuiltinType::builtin_type_t::kFloatType:
      return "f32";
    case BuiltinType::builtin_type_t::kDoubleType:
      return "f64";
    case BuiltinType::builtin_type_t::kStringType:
      return "String";
    case BuiltinType::builtin_type_t::kBinaryType:
      return "Vec<u8>";
    default:
      return "()";
    }
  }
  case DataType::data_type_t::kArrayType: {
    ArrayType *arrayType = dynamic_cast<ArrayType *>(dataType);
    return format_string("[%s; %d]",
                         getTypeString(arrayType->getElementType()).c_str(),
                         arrayType->getElementCount());
  }
  case DataType::data_type_t::kListType: {
    ListType *listType = dynamic_cast<ListType *>(dataType);
    return format_string("Vec<%s>",
                         getTypeString(listType->getElementType()).c_str());
  }
  case DataType::data_type_t::kStructType:
  case DataType::data_type_t::kEnumType:
  case DataType::data_type_t::kUnionType:
    return escapeKeyword(dataType->getName());
  case DataType::data_type_t::kVoidType:
    return "()";
  case DataType::data_type_t::kAliasType: {
    AliasType *aliasType = dynamic_cast<AliasType *>(dataType);
    return getTypeString(aliasType->getElementType());
  }
  case DataType::data_type_t::kFunctionType: {
    FunctionType *fnType = dynamic_cast<FunctionType *>(dataType);
    (void)fnType; // Suppress unused variable warning
    return format_string(
        "fn(%s) -> %s", "/* params */",
        "/* return */"); // TODO: Implement proper function type
  }
  default:
    return "()";
  }
}

bool RustGenerator::isListType(DataType *dataType) {
  return dataType &&
         (dataType->getDataType() == DataType::data_type_t::kListType ||
          dataType->getDataType() == DataType::data_type_t::kArrayType);
}

bool RustGenerator::isStringType(DataType *dataType) {
  if (!dataType)
    return false;

  if (dataType->getDataType() == DataType::data_type_t::kBuiltinType) {
    BuiltinType *builtinType = dynamic_cast<BuiltinType *>(dataType);
    return builtinType->getBuiltinType() ==
           BuiltinType::builtin_type_t::kStringType;
  }

  return false;
}

param_direction_t RustGenerator::getDirection(StructMember *param) {
  // Check for direction annotations
  auto direction = param->getDirection();
  if (direction != param_direction_t::kInDirection &&
      direction != param_direction_t::kOutDirection &&
      direction != param_direction_t::kInoutDirection) {
    // Default to input if no direction specified
    return param_direction_t::kInDirection;
  }
  return direction;
}

std::string RustGenerator::getDirectionString(StructMember *param) {
  param_direction_t direction = getDirection(param);
  switch (direction) {
  case param_direction_t::kInDirection:
    return "in";
  case param_direction_t::kOutDirection:
    return "out";
  case param_direction_t::kInoutDirection:
    return "inout";
  case param_direction_t::kReturn:
    return "return";
  default:
    return "in";
  }
}

std::string RustGenerator::generateParamWrite(StructMember *param,
                                              const std::string &paramName) {
  DataType *dataType = param->getDataType();

  if (!dataType)
    return "/* unknown parameter type */";

  switch (dataType->getDataType()) {
  case DataType::data_type_t::kBuiltinType: {
    BuiltinType *builtinType = dynamic_cast<BuiltinType *>(dataType);
    return generateBuiltinTypeWrite(builtinType, "request_codec", paramName,
                                    false);
  }
  case DataType::data_type_t::kStructType:
  case DataType::data_type_t::kEnumType:
    return paramName + ".write(&mut request_codec)";
  case DataType::data_type_t::kListType: {
    ListType *listType = dynamic_cast<ListType *>(dataType);
    return string("request_codec.start_write_list(") + paramName +
           ".len() as u32)?;\n" + "            for item in &" + paramName +
           " {\n" + "                " +
           generateListItemWrite(listType->getElementType(), "item") + "?;\n" +
           "            }";
  }
  case DataType::data_type_t::kAliasType: {
    AliasType *aliasType = dynamic_cast<AliasType *>(dataType);
    return generateParamWriteForDataType(aliasType->getElementType(),
                                         paramName);
  }
  default:
    return "/* unsupported parameter type */";
  }
}

std::string RustGenerator::generateTypeRead(DataType *dataType,
                                            const std::string &codecName) {
  if (!dataType)
    return "()";

  switch (dataType->getDataType()) {
  case DataType::data_type_t::kBuiltinType: {
    BuiltinType *builtinType = dynamic_cast<BuiltinType *>(dataType);
    string mutCodecName = codecName;
    // Only add &mut for response_codec, not for codec parameter which is
    // already &mut
    if (mutCodecName == "response_codec") {
      mutCodecName = "(&mut " + mutCodecName + ")";
    }
    return generateBuiltinTypeRead(builtinType, mutCodecName);
  }
  case DataType::data_type_t::kStructType:
  case DataType::data_type_t::kEnumType: {
    string mutCodecName = codecName;
    // Only add &mut for response_codec, not for codec parameter which is
    // already &mut
    if (mutCodecName == "response_codec") {
      mutCodecName = "(&mut " + mutCodecName + ")";
    }
    return escapeKeyword(dataType->getName()) + "::read(" + mutCodecName + ")";
  }
  case DataType::data_type_t::kListType: {
    ListType *listType = dynamic_cast<ListType *>(dataType);
    string mutCodecName = codecName;
    // Only add &mut for response_codec, not for codec parameter which is
    // already &mut
    if (mutCodecName == "response_codec") {
      mutCodecName = "(&mut " + mutCodecName + ")";
    }
    string elementRead =
        generateTypeRead(listType->getElementType(), codecName);
    return string("(|| -> ErpcResult<Vec<_>> {\n") +
           "                        let list_len = " + mutCodecName +
           ".start_read_list()?;\n" +
           "                        let mut list = Vec::new();\n" +
           "                        for _ in 0..list_len {\n" +
           "                            list.push(" + elementRead + "?);\n" +
           "                        }\n" +
           "                        Ok(list)\n" + "                    })()";
  }
  case DataType::data_type_t::kAliasType: {
    AliasType *aliasType = dynamic_cast<AliasType *>(dataType);
    return generateTypeRead(aliasType->getElementType(), codecName);
  }
  case DataType::data_type_t::kVoidType:
    return "()";
  default:
    return "Default::default()";
  }
}

std::string RustGenerator::generateListItemWrite(DataType *elementType,
                                                 const std::string &itemName) {
  return generateParamWriteForDataType(elementType, itemName);
}

std::string
RustGenerator::generateParamWriteForDataType(DataType *dataType,
                                             const std::string &paramName) {
  if (!dataType)
    return "/* unknown type */";

  switch (dataType->getDataType()) {
  case DataType::data_type_t::kBuiltinType: {
    BuiltinType *builtinType = dynamic_cast<BuiltinType *>(dataType);
    string actualParam = paramName;
    // For list items, dereference primitive types
    if (paramName == "item") {
      actualParam = "*" + paramName;
    }

    // Special handling for string and binary types
    if (builtinType->getBuiltinType() ==
            BuiltinType::builtin_type_t::kStringType ||
        builtinType->getBuiltinType() ==
            BuiltinType::builtin_type_t::kBinaryType) {
      actualParam = paramName; // Don't dereference for strings/binary
    }

    return generateBuiltinTypeWrite(builtinType, "request_codec", actualParam,
                                    false);
  }
  case DataType::data_type_t::kStructType:
  case DataType::data_type_t::kEnumType:
    return paramName + ".write(&mut request_codec)";
  case DataType::data_type_t::kAliasType: {
    AliasType *aliasType = dynamic_cast<AliasType *>(dataType);
    return generateParamWriteForDataType(aliasType->getElementType(),
                                         paramName);
  }
  default:
    return "/* unsupported type for list item */";
  }
}

std::string RustGenerator::generateMemberWrite(StructMember *member,
                                               const std::string &memberName) {
  DataType *dataType = member->getDataType();

  if (!dataType)
    return "/* unknown member type */";

  switch (dataType->getDataType()) {
  case DataType::data_type_t::kBuiltinType: {
    BuiltinType *builtinType = dynamic_cast<BuiltinType *>(dataType);
    return generateBuiltinTypeWrite(builtinType, "codec", memberName, true) +
           ";";
  }
  case DataType::data_type_t::kStructType:
  case DataType::data_type_t::kEnumType:
    return memberName + ".write(codec)?;";
  case DataType::data_type_t::kListType: {
    ListType *listType = dynamic_cast<ListType *>(dataType);
    DataType *elementType = listType->getElementType();

    string code = "codec.start_write_list(" + memberName + ".len() as u32)?;\n";
    code += "        for item in &" + memberName + " {\n";

    // Check if we need to dereference (for primitive types) or not (for
    // structs)
    string itemRef = "item";
    if (elementType->getDataType() == DataType::data_type_t::kBuiltinType ||
        (elementType->getDataType() == DataType::data_type_t::kAliasType &&
         dynamic_cast<AliasType *>(elementType)
                 ->getElementType()
                 ->getDataType() == DataType::data_type_t::kBuiltinType)) {
      itemRef = "*item";
    }

    code += "            " + generateTypeWrite(elementType, itemRef) + ";\n";
    code += "        }";
    return code;
  }
  case DataType::data_type_t::kAliasType: {
    // For alias types, handle the underlying type directly
    AliasType *aliasType = dynamic_cast<AliasType *>(dataType);
    StructMember aliasedMember(*member);
    aliasedMember.setDataType(aliasType->getElementType());
    return generateMemberWrite(&aliasedMember, memberName);
  }
  default:
    return "/* unsupported member type */";
  }
}

string RustGenerator::determineEnumReprType(EnumType *enumType) {
  // Use the actual underlying data type from the enum definition
  // In eRPC, enums follow C/C++ convention where the underlying type is int32
  // This ensures wire compatibility with C++ and Java implementations
  (void)enumType; // Suppress unused parameter warning

  return "i32";
}

string RustGenerator::getCodecMethodForReprType(const string &reprType) {
  // Map Rust repr types to codec method names
  if (reprType == "u8") {
    return "uint8";
  } else if (reprType == "u16") {
    return "uint16";
  } else if (reprType == "u32") {
    return "uint32";
  } else if (reprType == "u64") {
    return "uint64";
  } else if (reprType == "i8") {
    return "int8";
  } else if (reprType == "i16") {
    return "int16";
  } else if (reprType == "i32") {
    return "int32";
  } else if (reprType == "i64") {
    return "int64";
  } else {
    // Default to uint32 for unknown types
    return "uint32";
  }
}

std::string RustGenerator::generateClientRequestSerialization(
    Function *fn, const std::vector<StructMember *> &inParams) {
  std::ostringstream requestCode;

  Interface *interface = fn->getInterface();
  string interfaceServiceId = "ServiceId::" + interface->getName();
  string interfaceMethodId =
      interface->getName() + "Method::" + toPascalCase(fn->getName());

  // Check if we have parameters to determine if codec needs to be mutable
  bool hasParams = !inParams.empty();

  requestCode << "            // Serialize parameters to request_data\n";
  if (hasParams) {
    requestCode << "            let mut request_codec = BasicCodec::new();\n";
  } else {
    requestCode << "            let request_codec = BasicCodec::new();\n";
  }
  requestCode << "            \n";

  // Write each input parameter
  for (StructMember *param : inParams) {
    string paramName = escapeKeyword(toSnakeCase(param->getName()));
    requestCode << "            // Write " << paramName << "\n";
    string paramWriteCode = generateParamWrite(param, paramName);
    if (paramWriteCode.find("for item in") != string::npos) {
      // This is a list with a for loop, don't add ?;
      requestCode << "            " << paramWriteCode << "\n";
    } else {
      // Regular parameter, add ?;
      requestCode << "            " << paramWriteCode << "?;\n";
    }
  }

  if (!hasParams) {
    requestCode << "            // No parameters to serialize\n";
  }

  requestCode << "            \n";
  requestCode
      << "            let request_data = request_codec.as_bytes().to_vec();\n";
  requestCode << "            \n";
  requestCode << "            let "
              << (fn->isOneway() ? "_" : "response_data")
              << " = self.client\n";
  requestCode << "                .perform_request(\n";
  requestCode << "                    " << interfaceServiceId << ".as_u8(),\n";
  requestCode << "                    " << interfaceMethodId << ".as_u8(),\n";
  requestCode << "                    " << (fn->isOneway() ? "true" : "false")
              << ",\n";
  requestCode << "                    request_data\n";
  requestCode << "                )\n";
  requestCode << "                .await?;\n";

  return requestCode.str();
}

std::string RustGenerator::generateClientResponseDeserialization(
    Function *fn, const std::vector<DataType *> &outParamTypes,
    const std::vector<std::string> &outParamNames, bool hasOutParams) {
  std::ostringstream responseCode;

  DataType *returnType = fn->getReturnType();
  bool isVoidReturn = !returnType || returnType->getDataType() ==
                                         DataType::data_type_t::kVoidType;

  // For void return types without out params, or oneway methods, no response
  // deserialization needed
  if (isVoidReturn || fn->isOneway()) {
    return "            Ok(())"; // No response data to deserialize
  }

  responseCode << "            // Deserialize response_data\n";
  responseCode << "            if response_data.is_empty() {\n";
  responseCode << "                return Err(\"Empty response data "
                  "received\".into());\n";
  responseCode << "            }\n";
  responseCode << "            \n";
  responseCode << "            let mut response_codec = "
                  "BasicCodec::from_data(response_data);\n";
  responseCode << "            \n";

  // Collect @length annotated parameters to skip reading them separately
  std::set<std::string> lengthParams;
  for (auto param : fn->getParameters().getMembers()) {
    param_direction_t direction = getDirection(param);
    if (direction == param_direction_t::kOutDirection ||
        direction == param_direction_t::kInoutDirection) {
      // Check for @length annotation
      string annotations = param->getDescription();
      if (annotations.find("@length(") != string::npos) {
        // Extract the length parameter name from @length(paramName)
        size_t start = annotations.find("@length(") + 8;
        size_t end = annotations.find(")", start);
        if (end != string::npos) {
          string lengthParamName = annotations.substr(start, end - start);
          lengthParams.insert(lengthParamName);
        }
      }
    }
  }

  if (hasOutParams && returnType &&
      returnType->getDataType() != DataType::data_type_t::kVoidType) {
    // Function has both return value and out parameters
    responseCode << "            // Read return value and out parameters\n";

    // Read out parameters first, deriving @length parameter values from lists
    for (size_t i = 0; i < outParamTypes.size(); ++i) {
      string paramName = outParamNames[i];

      // Skip reading @length parameters separately - they're derived from list
      // headers
      if (lengthParams.find(paramName) != lengthParams.end()) {
        // This is a @length parameter - derive its value from the corresponding
        // list
        responseCode << "            // Note: " << paramName
                     << " is derived from list length\n";
        continue;
      }

      responseCode << generateOutParamRead(outParamTypes[i], outParamNames[i]);
    }

    // Then read return value
    responseCode << "            let return_value = "
                 << generateTypeRead(returnType, "&mut response_codec")
                 << "?;\n";

    // Derive @length parameter values from lists
    for (size_t i = 0; i < outParamNames.size(); ++i) {
      string paramName = outParamNames[i];
      if (lengthParams.find(paramName) != lengthParams.end()) {
        // Find the corresponding list parameter to get length from
        for (size_t j = 0; j < outParamNames.size(); ++j) {
          if (i != j && outParamTypes[j]->getDataType() ==
                            DataType::data_type_t::kListType) {
            responseCode << "            let " << paramName << " = "
                         << outParamNames[j] << ".len() as u32;\n";
            break;
          }
        }
      }
    }

    // Return tuple with return value and all out parameters
    responseCode << "            Ok((return_value";
    for (const auto &paramName : outParamNames) {
      responseCode << ", " << paramName;
    }
    responseCode << "))";
  } else if (hasOutParams) {
    // Function has only out parameters
    if (outParamTypes.size() == 1) {
      responseCode << "            // Read out parameter\n";
      string paramName = outParamNames[0];

      // Skip reading @length parameters separately
      if (lengthParams.find(paramName) == lengthParams.end()) {
        responseCode << generateOutParamRead(outParamTypes[0],
                                             outParamNames[0]);
      }
      responseCode << "            Ok(" << outParamNames[0] << ")";
    } else {
      responseCode << "            // Read out parameters\n";
      for (size_t i = 0; i < outParamTypes.size(); ++i) {
        string paramName = outParamNames[i];

        // Skip reading @length parameters separately
        if (lengthParams.find(paramName) != lengthParams.end()) {
          responseCode << "            // Note: " << paramName
                       << " is derived from list length\n";
          continue;
        }

        responseCode << generateOutParamRead(outParamTypes[i],
                                             outParamNames[i]);
      }

      // Derive @length parameter values from lists
      for (size_t i = 0; i < outParamNames.size(); ++i) {
        string paramName = outParamNames[i];
        if (lengthParams.find(paramName) != lengthParams.end()) {
          // Find the corresponding list parameter to get length from
          for (size_t j = 0; j < outParamNames.size(); ++j) {
            if (i != j && outParamTypes[j]->getDataType() ==
                              DataType::data_type_t::kListType) {
              responseCode << "            let " << paramName << " = "
                           << outParamNames[j] << ".len() as u32;\n";
              break;
            }
          }
        }
      }

      responseCode << "            Ok((";
      for (size_t i = 0; i < outParamNames.size(); ++i) {
        if (i > 0)
          responseCode << ", ";
        responseCode << outParamNames[i];
      }
      responseCode << "))";
    }
  } else if (returnType &&
             returnType->getDataType() != DataType::data_type_t::kVoidType) {
    responseCode << "            // Read return value\n";
    responseCode << "            let result = "
                 << generateTypeRead(returnType, "response_codec") << "?;\n";
    responseCode << "            Ok(result)";
  } else {
    responseCode << "            Ok(())";
  }

  return responseCode.str();
}

std::string RustGenerator::generateOutParamRead(DataType *paramType,
                                                const std::string &paramName) {
  std::ostringstream outParamCode;

  string codecName = getCodecNameForType(paramType, "response_codec");
  string readCode = generateTypeRead(paramType, codecName);

  if (paramType->getDataType() == DataType::data_type_t::kListType) {
    outParamCode << "            let " << paramName << " = " << readCode
                 << "?;\n";
  } else {
    outParamCode << "            let " << paramName << " = " << readCode
                 << "?;\n";
  }

  return outParamCode.str();
}

std::string
RustGenerator::getCodecNameForType(DataType *dataType,
                                   const std::string &baseCodecName) {
  if (dataType->getDataType() == DataType::data_type_t::kStructType ||
      dataType->getDataType() == DataType::data_type_t::kEnumType) {
    return "&mut " + baseCodecName;
  } else {
    return baseCodecName;
  }
}

std::string RustGenerator::generateServerParameterDeserialization(
    Function *fn, const std::string &codecParamName) {
  stringstream code;

  // Collect @length annotated parameters to skip reading them separately
  std::set<std::string> lengthParams;
  for (auto param : fn->getParameters().getMembers()) {
    // Check if this parameter is referenced by any @length annotation
    StructMember *referencedFrom = findParamReferencedFromAnn(
        fn->getParameters().getMembers(), param->getName(), LENGTH_ANNOTATION);
    if (referencedFrom) {
      lengthParams.insert(param->getName());
    }
  }

  // Collect input parameters
  std::vector<StructMember *> inParams;
  for (auto param : fn->getParameters().getMembers()) {
    param_direction_t direction = getDirection(param);
    if (direction == param_direction_t::kInDirection ||
        direction == param_direction_t::kInoutDirection) {
      inParams.push_back(param);
    }
  }

  for (auto param : inParams) {
    string paramName = escapeKeyword(toSnakeCase(param->getName()));

    // Skip @length parameters - they're transmitted in list headers
    if (lengthParams.find(param->getName()) != lengthParams.end()) {
      continue;
    }

    DataType *paramType = param->getDataType();

    code << "                    // Deserialize " << paramName
         << " parameter\n";

    if (isStringType(paramType)) {
      code << "                    let " << paramName << " = " << codecParamName
           << ".read_string()?;\n";
    } else if (paramType->getDataType() == DataType::data_type_t::kAliasType) {
      // Handle type aliases - get the underlying type
      AliasType *aliasType = dynamic_cast<AliasType *>(paramType);
      DataType *underlyingType = aliasType->getElementType();

      if (isStringType(underlyingType)) {
        code << "                    let " << paramName << " = "
             << codecParamName << ".read_string()?;\n";
      } else if (underlyingType->getDataType() ==
                 DataType::data_type_t::kBuiltinType) {
        BuiltinType *builtinType = dynamic_cast<BuiltinType *>(underlyingType);
        switch (builtinType->getBuiltinType()) {
        case BuiltinType::builtin_type_t::kUInt8Type:
          code << "                    let " << paramName << " = "
               << codecParamName << ".read_uint8()?;\n";
          break;
        case BuiltinType::builtin_type_t::kUInt16Type:
          code << "                    let " << paramName << " = "
               << codecParamName << ".read_uint16()?;\n";
          break;
        case BuiltinType::builtin_type_t::kUInt32Type:
          code << "                    let " << paramName << " = "
               << codecParamName << ".read_uint32()?;\n";
          break;
        case BuiltinType::builtin_type_t::kUInt64Type:
          code << "                    let " << paramName << " = "
               << codecParamName << ".read_uint64()?;\n";
          break;
        case BuiltinType::builtin_type_t::kInt8Type:
          code << "                    let " << paramName << " = "
               << codecParamName << ".read_int8()?;\n";
          break;
        case BuiltinType::builtin_type_t::kInt16Type:
          code << "                    let " << paramName << " = "
               << codecParamName << ".read_int16()?;\n";
          break;
        case BuiltinType::builtin_type_t::kInt32Type:
          code << "                    let " << paramName << " = "
               << codecParamName << ".read_int32()?;\n";
          break;
        case BuiltinType::builtin_type_t::kInt64Type:
          code << "                    let " << paramName << " = "
               << codecParamName << ".read_int64()?;\n";
          break;
        case BuiltinType::builtin_type_t::kFloatType:
          code << "                    let " << paramName << " = "
               << codecParamName << ".read_float()?;\n";
          break;
        case BuiltinType::builtin_type_t::kDoubleType:
          code << "                    let " << paramName << " = "
               << codecParamName << ".read_double()?;\n";
          break;
        case BuiltinType::builtin_type_t::kBoolType:
          code << "                    let " << paramName << " = "
               << codecParamName << ".read_bool()?;\n";
          break;
        case BuiltinType::builtin_type_t::kBinaryType:
          code << "                    let " << paramName << " = "
               << codecParamName << ".read_binary()?;\n";
          break;
        default:
          code << "                    let " << paramName
               << " = Default::default(); // TODO: Handle "
               << builtinType->getName() << "\n";
          break;
        }
      } else {
        string typeName = getTypeString(underlyingType);
        code << "                    let " << paramName << " = " << typeName
             << "::read(codec)?;\n";
      }
    } else if (paramType->getDataType() ==
               DataType::data_type_t::kBuiltinType) {
      BuiltinType *builtinType = dynamic_cast<BuiltinType *>(paramType);
      switch (builtinType->getBuiltinType()) {
      case BuiltinType::builtin_type_t::kUInt8Type:
        code << "                    let " << paramName << " = "
             << codecParamName << ".read_uint8()?;\n";
        break;
      case BuiltinType::builtin_type_t::kUInt16Type:
        code << "                    let " << paramName << " = "
             << codecParamName << ".read_uint16()?;\n";
        break;
      case BuiltinType::builtin_type_t::kUInt32Type:
        code << "                    let " << paramName << " = "
             << codecParamName << ".read_uint32()?;\n";
        break;
      case BuiltinType::builtin_type_t::kUInt64Type:
        code << "                    let " << paramName << " = "
             << codecParamName << ".read_uint64()?;\n";
        break;
      case BuiltinType::builtin_type_t::kInt8Type:
        code << "                    let " << paramName << " = "
             << codecParamName << ".read_int8()?;\n";
        break;
      case BuiltinType::builtin_type_t::kInt16Type:
        code << "                    let " << paramName << " = "
             << codecParamName << ".read_int16()?;\n";
        break;
      case BuiltinType::builtin_type_t::kInt32Type:
        code << "                    let " << paramName << " = "
             << codecParamName << ".read_int32()?;\n";
        break;
      case BuiltinType::builtin_type_t::kInt64Type:
        code << "                    let " << paramName << " = "
             << codecParamName << ".read_int64()?;\n";
        break;
      case BuiltinType::builtin_type_t::kFloatType:
        code << "                    let " << paramName << " = "
             << codecParamName << ".read_float()?;\n";
        break;
      case BuiltinType::builtin_type_t::kDoubleType:
        code << "                    let " << paramName << " = "
             << codecParamName << ".read_double()?;\n";
        break;
      case BuiltinType::builtin_type_t::kBoolType:
        code << "                    let " << paramName << " = "
             << codecParamName << ".read_bool()?;\n";
        break;
      case BuiltinType::builtin_type_t::kBinaryType:
        code << "                    let " << paramName << " = "
             << codecParamName << ".read_binary()?;\n";
        break;
      default:
        code << "                    let " << paramName
             << " = Default::default(); // TODO: Handle "
             << builtinType->getName() << "\n";
        break;
      }
    } else if (paramType->getDataType() == DataType::data_type_t::kStructType ||
               paramType->getDataType() == DataType::data_type_t::kEnumType) {
      string typeName = getTypeString(paramType);
      code << "                    let " << paramName << " = " << typeName
           << "::read(codec)?;\n";
    } else if (paramType->getDataType() == DataType::data_type_t::kListType) {
      code << "                    let " << paramName << " = "
           << generateTypeRead(paramType, codecParamName) << "?;\n";
    } else {
      code << "                    let " << paramName
           << " = Default::default(); // TODO: Handle type "
           << getTypeString(paramType) << "\n";
    }

    code << "                    \n";
  }

  return code.str();
}

std::string RustGenerator::generateServerResponseSerialization(
    Function *fn, const std::string &codecParamName) {
  stringstream code;

  // Collect @length annotated parameters to skip serializing them separately
  std::set<std::string> lengthParams;
  for (auto param : fn->getParameters().getMembers()) {
    param_direction_t direction = getDirection(param);
    if (direction == param_direction_t::kOutDirection ||
        direction == param_direction_t::kInoutDirection) {
      // Check for @length annotation
      string annotations = param->getDescription();
      if (annotations.find("@length(") != string::npos) {
        // Extract the length parameter name from @length(paramName)
        size_t start = annotations.find("@length(") + 8;
        size_t end = annotations.find(")", start);
        if (end != string::npos) {
          string lengthParamName = annotations.substr(start, end - start);
          lengthParams.insert(lengthParamName);
        }
      }
    }
  }

  // Collect output parameters with names, filtering out @length parameters
  std::vector<std::pair<std::string, DataType *>> outParams;
  bool hasOutParams = false;
  for (auto param : fn->getParameters().getMembers()) {
    param_direction_t direction = getDirection(param);
    if (direction == param_direction_t::kOutDirection ||
        direction == param_direction_t::kInoutDirection) {
      string paramName = escapeKeyword(toSnakeCase(param->getName()));

      // Skip @length parameters - they're transmitted in list headers
      if (lengthParams.find(param->getName()) != lengthParams.end()) {
        continue;
      }

      outParams.push_back(std::make_pair(paramName, param->getDataType()));
      hasOutParams = true;
    }
  }

  DataType *returnType = fn->getReturnType();
  bool hasReturnValue = (returnType && returnType->getDataType() !=
                                           DataType::data_type_t::kVoidType);

  code << "                            // Serialize response\n";

  if (hasReturnValue && hasOutParams) {
    // Function has both return value and out parameters - extract tuple
    // variables for clarity
    code << "                            let (";
    code << "return_value";
    for (const auto &outParam : outParams) {
      code << ", " << outParam.first; // Use actual parameter name
    }
    code << ") = response;\n";

    // Serialize out parameters first
    for (const auto &outParam : outParams) {
      code << "                            "
           << generateTypeWrite(outParam.second, outParam.first)
           << "; // Out parameter: " << outParam.first << "\n";
    }

    // Then serialize return value
    code << "                            "
         << generateTypeWrite(returnType, "return_value")
         << "; // Return value\n";
  } else if (hasOutParams) {
    // Function has only out parameters
    if (outParams.size() == 1) {
      code << "                            "
           << generateTypeWrite(outParams[0].second, "response")
           << "; // Single out parameter: " << outParams[0].first << "\n";
    } else {
      // Multiple out parameters - extract tuple variables
      code << "                            let (";
      for (size_t i = 0; i < outParams.size(); ++i) {
        if (i > 0)
          code << ", ";
        code << outParams[i].first; // Use actual parameter name
      }
      code << ") = response;\n";

      for (const auto &outParam : outParams) {
        code << "                            "
             << generateTypeWrite(outParam.second, outParam.first)
             << "; // Out parameter: " << outParam.first << "\n";
      }
    }
  } else if (hasReturnValue) {
    // Function has only return value
    code << "                            "
         << generateTypeWrite(returnType, "response") << "; // Return value\n";
  }
  // If no return value and no out parameters, nothing to serialize

  return code.str();
}

std::string RustGenerator::generateServerErrorResponseSerialization(
    Function *fn, const std::string &codecParamName) {
  stringstream code;

  // Collect @length annotated parameters to skip serializing them separately
  std::set<std::string> lengthParams;
  for (auto param : fn->getParameters().getMembers()) {
    param_direction_t direction = getDirection(param);
    if (direction == param_direction_t::kOutDirection ||
        direction == param_direction_t::kInoutDirection) {
      // Check for @length annotation
      string annotations = param->getDescription();
      if (annotations.find("@length(") != string::npos) {
        // Extract the length parameter name from @length(paramName)
        size_t start = annotations.find("@length(") + 8;
        size_t end = annotations.find(")", start);
        if (end != string::npos) {
          string lengthParamName = annotations.substr(start, end - start);
          lengthParams.insert(lengthParamName);
        }
      }
    }
  }

  // Collect output parameters with names, filtering out @length parameters
  std::vector<std::pair<std::string, DataType *>> outParams;
  bool hasOutParams = false;
  for (auto param : fn->getParameters().getMembers()) {
    param_direction_t direction = getDirection(param);
    if (direction == param_direction_t::kOutDirection ||
        direction == param_direction_t::kInoutDirection) {
      string paramName = escapeKeyword(toSnakeCase(param->getName()));

      // Skip @length parameters - they're transmitted in list headers
      if (lengthParams.find(param->getName()) != lengthParams.end()) {
        continue;
      }

      outParams.push_back(std::make_pair(paramName, param->getDataType()));
      hasOutParams = true;
    }
  }

  DataType *returnType = fn->getReturnType();
  bool hasReturnValue = (returnType && returnType->getDataType() !=
                                           DataType::data_type_t::kVoidType);

  code << "                            // Send error response with default out "
          "parameters\n";

  if (hasOutParams) {
    // Serialize default values for out parameters first
    for (const auto &outParam : outParams) {
      code << "                            "
           << generateDefaultErrorWrite(outParam.second)
           << "; // Default out parameter: " << outParam.first << "\n";
    }
  }

  if (hasReturnValue) {
    // Then serialize error return value
    code << "                            "
         << generateDefaultErrorWrite(returnType)
         << "; // Error return value\n";
  }

  return code.str();
}

std::string RustGenerator::generateDefaultErrorWrite(DataType *dataType) {
  stringstream code;

  if (dataType->getDataType() == DataType::data_type_t::kListType) {
    // For lists, write empty list
    code << "codec.start_write_list(0)?";
  } else if (dataType->getDataType() == DataType::data_type_t::kBuiltinType) {
    BuiltinType *builtinType = dynamic_cast<BuiltinType *>(dataType);
    code << generateBuiltinTypeDefaultWrite(builtinType, "codec");
  } else if (dataType->getDataType() == DataType::data_type_t::kAliasType) {
    // Handle type aliases - get the underlying type
    AliasType *aliasType = dynamic_cast<AliasType *>(dataType);
    DataType *underlyingType = aliasType->getElementType();
    return generateDefaultErrorWrite(underlyingType);
  } else if (dataType->getDataType() == DataType::data_type_t::kStructType) {
    // Use the struct's type name with Default::default()
    string typeName = escapeKeyword(dataType->getName());
    code << typeName << "::default().write(codec)?";
  } else if (dataType->getDataType() == DataType::data_type_t::kEnumType) {
    // Use the enum's type name with Default::default()
    string typeName = escapeKeyword(dataType->getName());
    code << typeName << "::default().write(codec)?";
  } else {
    code << "Default::default().write(codec)?";
  }

  return code.str();
}

std::string RustGenerator::generateTypeWrite(DataType *dataType,
                                             const std::string &variableName) {
  stringstream code;

  if (dataType->getDataType() == DataType::data_type_t::kListType) {
    // For lists, we need to write the length and then each element
    ListType *listType = dynamic_cast<ListType *>(dataType);
    DataType *elementType = listType->getElementType();

    code << "codec.start_write_list(" << variableName << ".len() as u32)?;\n";
    code << "                            for item in &" << variableName
         << " {\n";

    // Check if we need to dereference (for primitive types) or not (for
    // structs)
    string itemRef = "item";
    if (elementType->getDataType() == DataType::data_type_t::kBuiltinType ||
        (elementType->getDataType() == DataType::data_type_t::kAliasType &&
         dynamic_cast<AliasType *>(elementType)
                 ->getElementType()
                 ->getDataType() == DataType::data_type_t::kBuiltinType)) {
      itemRef = "*item";
    }

    code << "                                "
         << generateTypeWrite(elementType, itemRef) << ";\n";
    code << "                            }";
  } else if (dataType->getDataType() == DataType::data_type_t::kBuiltinType) {
    BuiltinType *builtinType = dynamic_cast<BuiltinType *>(dataType);
    code << generateBuiltinTypeWrite(builtinType, "codec", variableName, true);
  } else if (dataType->getDataType() == DataType::data_type_t::kAliasType) {
    // Handle type aliases - get the underlying type
    AliasType *aliasType = dynamic_cast<AliasType *>(dataType);
    DataType *underlyingType = aliasType->getElementType();
    return generateTypeWrite(underlyingType, variableName);
  } else if (dataType->getDataType() == DataType::data_type_t::kStructType) {
    code << variableName << ".write(codec)?";
  } else if (dataType->getDataType() == DataType::data_type_t::kEnumType) {
    code << "codec.write_int32(" << variableName << " as i32)?";
  } else {
    code << variableName << ".write(codec)? // TODO: Handle type";
  }

  return code.str();
}

std::set<std::string> RustGenerator::collectLengthParams(Function *fn) {
  std::set<std::string> lengthParams;
  for (auto param : fn->getParameters().getMembers()) {
    // Check if this parameter is referenced by any @length annotation
    StructMember *referencedFrom = findParamReferencedFromAnn(
        fn->getParameters().getMembers(), param->getName(), LENGTH_ANNOTATION);
    if (referencedFrom) {
      lengthParams.insert(param->getName());
    }
  }
  return lengthParams;
}

bool RustGenerator::processFunctionParameters(
    Function *fn, const std::set<std::string> &lengthParams,
    cpptempl::data_list &params, std::stringstream &asyncParams,
    std::stringstream &clientParams, std::vector<std::string> &outParamNames,
    std::vector<DataType *> &outParamTypes) {
  bool hasOutParams = false;

  for (auto param : fn->getParameters().getMembers()) {
    cpptempl::data_map paramInfo;
    paramInfo["name"] = escapeKeyword(toSnakeCase(param->getName()));
    paramInfo["type"] = getTypeString(param->getDataType());
    paramInfo["direction"] = getDirectionString(param);

    setTemplateComments(param, paramInfo);
    params.push_back(paramInfo);

    param_direction_t direction = getDirection(param);
    if (direction == param_direction_t::kInDirection ||
        direction == param_direction_t::kInoutDirection) {
      // Skip @length parameters in trait method signatures
      if (lengthParams.find(param->getName()) != lengthParams.end()) {
        continue;
      }

      asyncParams << ", " << escapeKeyword(toSnakeCase(param->getName()))
                  << ": ";
      clientParams << ", " << escapeKeyword(toSnakeCase(param->getName()))
                   << ": ";

      if (isStringType(param->getDataType())) {
        asyncParams << "&str";
        clientParams << "&str";
      } else {
        asyncParams << getTypeString(param->getDataType());
        clientParams << getTypeString(param->getDataType());
      }
    }

    if (direction == param_direction_t::kOutDirection ||
        direction == param_direction_t::kInoutDirection) {
      hasOutParams = true;
      outParamNames.push_back(escapeKeyword(toSnakeCase(param->getName())));
      outParamTypes.push_back(param->getDataType());
    }
  }

  return hasOutParams;
}

std::string RustGenerator::generateParameterCallList(Function *fn) {
  std::stringstream paramCallList;
  bool firstParam = true;
  for (auto param : fn->getParameters().getMembers()) {
    param_direction_t direction = getDirection(param);
    if (direction == param_direction_t::kInDirection ||
        direction == param_direction_t::kInoutDirection) {
      if (!firstParam)
        paramCallList << ", ";
      firstParam = false;
      paramCallList << escapeKeyword(toSnakeCase(param->getName()));
    }
  }
  return paramCallList.str();
}

void RustGenerator::determineReturnTypes(
    Function *fn, bool hasOutParams,
    const std::vector<DataType *> &outParamTypes, std::string &rustReturnType,
    std::string &clientReturnType) {
  DataType *returnType = fn->getReturnType();

  if (hasOutParams && returnType &&
      returnType->getDataType() != DataType::data_type_t::kVoidType) {
    // Function has both return value and out parameters
    rustReturnType = " -> Result<(" + getTypeString(returnType);
    clientReturnType = " -> Result<(" + getTypeString(returnType);
    for (size_t i = 0; i < outParamTypes.size(); ++i) {
      rustReturnType += ", " + getTypeString(outParamTypes[i]);
      clientReturnType += ", " + getTypeString(outParamTypes[i]);
    }
    rustReturnType += "), Box<dyn std::error::Error + Send + Sync>>";
    clientReturnType += "), Box<dyn std::error::Error + Send + Sync>>";
  } else if (hasOutParams) {
    // Function has only out parameters
    if (outParamTypes.size() == 1) {
      rustReturnType = " -> Result<" + getTypeString(outParamTypes[0]) +
                       ", Box<dyn std::error::Error + Send + Sync>>";
      clientReturnType = rustReturnType;
    } else {
      rustReturnType = " -> Result<(";
      clientReturnType = " -> Result<(";
      for (size_t i = 0; i < outParamTypes.size(); ++i) {
        if (i > 0) {
          rustReturnType += ", ";
          clientReturnType += ", ";
        }
        rustReturnType += getTypeString(outParamTypes[i]);
        clientReturnType += getTypeString(outParamTypes[i]);
      }
      rustReturnType += "), Box<dyn std::error::Error + Send + Sync>>";
      clientReturnType += "), Box<dyn std::error::Error + Send + Sync>>";
    }
  } else if (returnType &&
             returnType->getDataType() != DataType::data_type_t::kVoidType) {
    // Function has only return value
    rustReturnType = " -> Result<" + getTypeString(returnType) +
                     ", Box<dyn std::error::Error + Send + Sync>>";
    clientReturnType = rustReturnType;
  } else {
    // Function has no return value or out parameters
    // Always use Result type for consistency and better error handling
    rustReturnType =
        " -> Result<(), Box<dyn std::error::Error + Send + Sync>>";
    clientReturnType = rustReturnType;
  }
}

std::string RustGenerator::generateMethodCallParameters(
    Function *fn, const std::set<std::string> &lengthParams) {
  std::stringstream paramList;
  bool first = true;

  for (auto param : fn->getParameters().getMembers()) {
    param_direction_t direction = getDirection(param);
    if (direction == param_direction_t::kInDirection ||
        direction == param_direction_t::kInoutDirection) {
      // Skip @length parameters
      if (lengthParams.find(param->getName()) != lengthParams.end()) {
        continue;
      }

      if (!first)
        paramList << ", ";
      first = false;

      std::string paramName = escapeKeyword(toSnakeCase(param->getName()));
      DataType *paramType = param->getDataType();

      // Handle parameter passing based on trait signature expectations
      if (paramType->getDataType() == DataType::data_type_t::kAliasType) {
        AliasType *aliasType = dynamic_cast<AliasType *>(paramType);
        if (isStringType(aliasType->getElementType())) {
          // String alias types expect owned values in trait
          paramList << paramName;
        } else {
          // Non-string alias types expect owned values in trait
          paramList << paramName;
        }
      } else if (isStringType(paramType)) {
        // Regular strings expect references in trait
        paramList << "&" << paramName;
      } else {
        // All other types (primitives, structs, enums) expect owned values in
        // trait
        paramList << paramName;
      }
    }
  }

  return paramList.str();
}

std::string
RustGenerator::generateOnewayServerHandler(Function *fn,
                                           const std::string &codecParamName) {
  std::stringstream serverHandlerCode;
  std::set<std::string> lengthParams = collectLengthParams(fn);

  serverHandlerCode << "                    \n";

  // Generate parameter deserialization
  serverHandlerCode << generateServerParameterDeserialization(fn,
                                                              codecParamName);

  // Generate method call with parameters
  serverHandlerCode << "                    let _ = self.service."
                    << toSnakeCase(fn->getName()) << "(";
  serverHandlerCode << generateMethodCallParameters(fn, lengthParams);
  serverHandlerCode << ").await;\n";
  serverHandlerCode << "                    Ok(())";

  return serverHandlerCode.str();
}

std::string RustGenerator::generateRegularServerHandler(
    Function *fn, const std::string &codecParamName,
    const std::string &sequenceParamName) {
  std::stringstream serverHandlerCode;
  std::set<std::string> lengthParams = collectLengthParams(fn);

  // Get the interface name to construct proper constant names
  Interface *interface = fn->getInterface();
  std::string interfaceServiceId = "ServiceId::" + interface->getName();
  std::string interfaceMethodId =
      interface->getName() + "Method::" + toPascalCase(fn->getName());

  serverHandlerCode << "                    \n";

  // Generate parameter deserialization
  serverHandlerCode << generateServerParameterDeserialization(fn,
                                                              codecParamName);

  // Generate method call with parameters
  serverHandlerCode << "                    let result = self.service."
                    << toSnakeCase(fn->getName()) << "(";
  serverHandlerCode << generateMethodCallParameters(fn, lengthParams);
  serverHandlerCode << ").await;\n";

  // Check if there are any response parameters to serialize
  DataType *returnType = fn->getReturnType();
  bool hasReturnValue = (returnType && returnType->getDataType() !=
                                           DataType::data_type_t::kVoidType);

  bool hasOutParams = false;
  for (auto param : fn->getParameters().getMembers()) {
    param_direction_t direction = getDirection(param);
    if (direction == param_direction_t::kOutDirection ||
        direction == param_direction_t::kInoutDirection) {
      hasOutParams = true;
      break;
    }
  }

  bool hasResponseParams = hasReturnValue || hasOutParams;

  serverHandlerCode << "                    match result {\n";
  if (hasResponseParams) {
    serverHandlerCode << "                        Ok(response) => {\n";
  } else {
    serverHandlerCode << "                        Ok(_) => {\n";
  }
  serverHandlerCode << "                            let reply_info = "
                       "MessageInfo::new(MessageType::Reply, "
                    << interfaceServiceId << ".as_u8(), " << interfaceMethodId
                    << ".as_u8(), " << sequenceParamName << ");\n";
  serverHandlerCode << "                            " << codecParamName
                    << ".start_write_message(&reply_info)?;\n";

  // Generate response serialization
  serverHandlerCode << generateServerResponseSerialization(fn, codecParamName);

  serverHandlerCode << "                            Ok(())\n";
  serverHandlerCode << "                        },\n";
  serverHandlerCode << "                        Err(_e) => {\n";
  serverHandlerCode << "                            let reply_info = "
                       "MessageInfo::new(MessageType::Reply, "
                    << interfaceServiceId << ".as_u8(), " << interfaceMethodId
                    << ".as_u8(), " << sequenceParamName << ");\n";
  serverHandlerCode << "                            " << codecParamName
                    << ".start_write_message(&reply_info)?;\n";

  // Generate error response with default out parameters
  serverHandlerCode << generateServerErrorResponseSerialization(fn,
                                                                codecParamName);

  serverHandlerCode << "                            Ok(())\n";
  serverHandlerCode << "                        }\n";
  serverHandlerCode << "                    }";

  return serverHandlerCode.str();
}

std::string RustGenerator::generateClientMethodCode(
    Function *fn, std::vector<DataType *> &outParamTypes, bool &hasOutParams) {
  std::stringstream clientMethodCode;

  // Clear and reset collections
  outParamTypes.clear();
  hasOutParams = false;

  // Collect input parameters and output parameter types with names
  std::vector<StructMember *> inParams;
  std::vector<std::string> clientOutParamNames;
  // Collect @length annotated parameters to skip in client serialization
  std::set<std::string> clientLengthParams = collectLengthParams(fn);

  for (auto param : fn->getParameters().getMembers()) {
    param_direction_t direction = getDirection(param);
    if (direction == param_direction_t::kInDirection ||
        direction == param_direction_t::kInoutDirection) {
      // Skip @length parameters in client serialization
      if (clientLengthParams.find(param->getName()) ==
          clientLengthParams.end()) {
        inParams.push_back(param);
      }
    }
    if (direction == param_direction_t::kOutDirection ||
        direction == param_direction_t::kInoutDirection) {
      outParamTypes.push_back(param->getDataType());
      clientOutParamNames.push_back(
          escapeKeyword(toSnakeCase(param->getName())));
      hasOutParams = true;
    }
  }

  // Generate request serialization
  clientMethodCode << generateClientRequestSerialization(fn, inParams);

  if (fn->isOneway()) {
    clientMethodCode << "            Ok(())";
  } else {
    // Generate response deserialization
    clientMethodCode << generateClientResponseDeserialization(
        fn, outParamTypes, clientOutParamNames, hasOutParams);
  }

  return clientMethodCode.str();
}

cpptempl::data_list RustGenerator::processGroupConstants(Group *group) {
  cpptempl::data_list constants;
  for (Symbol *symbol : group->getSymbols()) {
    if (symbol->getSymbolType() == Symbol::symbol_type_t::kConstSymbol) {
      ConstType *constType = dynamic_cast<ConstType *>(symbol);
      if (constType) {
        cpptempl::data_map constInfo;
        constInfo["name"] = escapeKeyword(constType->getName());
        constInfo["type"] = getTypeString(constType->getDataType());
        constInfo["value"] = constType->getValue()->toString();
        setTemplateComments(constType, constInfo);
        constants.push_back(constInfo);
      }
    }
  }
  return constants;
}

cpptempl::data_list RustGenerator::processGroupTypeAliases(Group *group) {
  cpptempl::data_list typeAliases;
  for (Symbol *symbol : group->getSymbols()) {
    if (symbol->getSymbolType() == Symbol::symbol_type_t::kTypenameSymbol) {
      DataType *dataType = dynamic_cast<DataType *>(symbol);
      if (dataType &&
          dataType->getDataType() == DataType::data_type_t::kAliasType) {
        AliasType *aliasType = dynamic_cast<AliasType *>(dataType);
        if (aliasType) {
          cpptempl::data_map aliasInfo;
          aliasInfo["name"] = escapeKeyword(aliasType->getName());
          aliasInfo["type"] = getTypeString(aliasType->getElementType());
          setTemplateComments(aliasType, aliasInfo);
          typeAliases.push_back(aliasInfo);
        }
      }
    }
  }
  return typeAliases;
}

cpptempl::data_list RustGenerator::processGroupInterfaces(Group *group) {
  cpptempl::data_list interfaces;
  for (auto interface : group->getInterfaces()) {
    cpptempl::data_map interfaceInfo;
    interfaceInfo["name"] = escapeKeyword(interface->getName());
    interfaceInfo["id"] = interface->getUniqueId();
    interfaceInfo["moduleName"] = toSnakeCase(interface->getName()) + "_server";
    interfaceInfo["description"] = interface->getDescription();
    interfaceInfo["serviceIdName"] =
        toUpperSnakeCase(interface->getName()) + "_SERVICE_ID";
    interfaceInfo["methodConstName"] =
        toUpperSnakeCase(interface->getName()) + "_METHOD";
    interfaceInfo["generateServer"] = true;
    interfaceInfo["generateClient"] = true;

    setTemplateComments(interface, interfaceInfo);

    // Add functions
    cpptempl::data_list functions;
    for (auto function : interface->getFunctions()) {
      functions.push_back(getFunctionTemplateData(group, function));
    }
    interfaceInfo["functions"] = functions;

    interfaces.push_back(interfaceInfo);
  }
  return interfaces;
}

cpptempl::data_list RustGenerator::processGroupEnums(Group *group) {
  cpptempl::data_list enums;
  for (Symbol *symbol : group->getSymbols()) {
    if (symbol->getSymbolType() == Symbol::symbol_type_t::kTypenameSymbol) {
      DataType *dataType = dynamic_cast<DataType *>(symbol);
      if (dataType &&
          dataType->getDataType() == DataType::data_type_t::kEnumType) {
        EnumType *enumType = dynamic_cast<EnumType *>(dataType);
        if (enumType) {
          cpptempl::data_map enumInfo;
          enumInfo["name"] = escapeKeyword(enumType->getName());

          setTemplateComments(enumType, enumInfo);

          cpptempl::data_list members;
          string firstMemberName = "";
          for (auto member : enumType->getMembers()) {
            cpptempl::data_map memberInfo;
            string enumMemberName = member->getName();
            memberInfo["name"] = escapeKeyword(enumMemberName);
            memberInfo["value"] = member->getValue();
            memberInfo["originalName"] = member->getName();
            setTemplateComments(member, memberInfo);
            members.push_back(memberInfo);

            // Store first member name for default implementation
            if (firstMemberName.empty()) {
              firstMemberName = escapeKeyword(enumMemberName);
            }
          }
          enumInfo["members"] = members;
          enumInfo["firstMember"] = firstMemberName;

          // Determine optimal representation type based on enum values from
          // lexical analyzer input
          string reprType = determineEnumReprType(enumType);
          string codecMethod = getCodecMethodForReprType(reprType);

          enumInfo["reprType"] = reprType;
          enumInfo["writeCode"] =
              "codec.write_" + codecMethod + "(*self as " + reprType + ")?;";
          enumInfo["readCode"] = "codec.read_" + codecMethod + "()";

          enums.push_back(enumInfo);
        }
      }
    }
  }
  return enums;
}

cpptempl::data_list RustGenerator::processGroupStructs(Group *group) {
  cpptempl::data_list structs;
  for (Symbol *symbol : group->getSymbols()) {
    if (symbol->getSymbolType() == Symbol::symbol_type_t::kTypenameSymbol) {
      DataType *dataType = dynamic_cast<DataType *>(symbol);
      if (dataType &&
          dataType->getDataType() == DataType::data_type_t::kStructType) {
        StructType *structType = dynamic_cast<StructType *>(dataType);
        if (structType) {
          cpptempl::data_map structInfo;
          structInfo["name"] = escapeKeyword(structType->getName());

          setTemplateComments(structType, structInfo);

          cpptempl::data_list members;
          for (auto member : structType->getMembers()) {
            cpptempl::data_map memberInfo;
            string memberName = escapeKeyword(toSnakeCase(member->getName()));
            memberInfo["name"] = memberName;
            memberInfo["type"] = getTypeString(member->getDataType());
            memberInfo["originalType"] = member->getDataType()->getName();
            memberInfo["originalName"] = member->getName();
            memberInfo["defaultValue"] = getDefaultValue(member->getDataType());
            memberInfo["writeCode"] =
                generateMemberWrite(member, "self." + memberName);
            memberInfo["readCode"] =
                generateTypeRead(member->getDataType(), "codec");
            setTemplateComments(member, memberInfo);
            members.push_back(memberInfo);
          }
          structInfo["members"] = members;
          structs.push_back(structInfo);
        }
      }
    }
  }
  return structs;
}

std::string RustGenerator::generateBuiltinTypeWrite(
    BuiltinType *builtinType, const std::string &codecName,
    const std::string &variableName, bool addErrorHandling) {
  std::string operation;
  std::string errorSuffix = addErrorHandling ? "?" : "";

  switch (builtinType->getBuiltinType()) {
  case BuiltinType::builtin_type_t::kBoolType:
    operation = codecName + ".write_bool(" + variableName + ")" + errorSuffix;
    break;
  case BuiltinType::builtin_type_t::kInt8Type:
    operation = codecName + ".write_int8(" + variableName + ")" + errorSuffix;
    break;
  case BuiltinType::builtin_type_t::kInt16Type:
    operation = codecName + ".write_int16(" + variableName + ")" + errorSuffix;
    break;
  case BuiltinType::builtin_type_t::kInt32Type:
    operation = codecName + ".write_int32(" + variableName + ")" + errorSuffix;
    break;
  case BuiltinType::builtin_type_t::kInt64Type:
    operation = codecName + ".write_int64(" + variableName + ")" + errorSuffix;
    break;
  case BuiltinType::builtin_type_t::kUInt8Type:
    operation = codecName + ".write_uint8(" + variableName + ")" + errorSuffix;
    break;
  case BuiltinType::builtin_type_t::kUInt16Type:
    operation = codecName + ".write_uint16(" + variableName + ")" + errorSuffix;
    break;
  case BuiltinType::builtin_type_t::kUInt32Type:
    operation = codecName + ".write_uint32(" + variableName + ")" + errorSuffix;
    break;
  case BuiltinType::builtin_type_t::kUInt64Type:
    operation = codecName + ".write_uint64(" + variableName + ")" + errorSuffix;
    break;
  case BuiltinType::builtin_type_t::kFloatType:
    operation = codecName + ".write_float(" + variableName + ")" + errorSuffix;
    break;
  case BuiltinType::builtin_type_t::kDoubleType:
    operation = codecName + ".write_double(" + variableName + ")" + errorSuffix;
    break;
  case BuiltinType::builtin_type_t::kStringType:
    operation =
        codecName + ".write_string(&" + variableName + ")" + errorSuffix;
    break;
  case BuiltinType::builtin_type_t::kBinaryType:
    operation =
        codecName + ".write_binary(&" + variableName + ")" + errorSuffix;
    break;
  default:
    operation = variableName + ".write(" + codecName + ")" + errorSuffix;
    break;
  }

  return operation;
}

std::string
RustGenerator::generateBuiltinTypeRead(BuiltinType *builtinType,
                                       const std::string &codecName) {
  switch (builtinType->getBuiltinType()) {
  case BuiltinType::builtin_type_t::kBoolType:
    return codecName + ".read_bool()";
  case BuiltinType::builtin_type_t::kInt8Type:
    return codecName + ".read_int8()";
  case BuiltinType::builtin_type_t::kInt16Type:
    return codecName + ".read_int16()";
  case BuiltinType::builtin_type_t::kInt32Type:
    return codecName + ".read_int32()";
  case BuiltinType::builtin_type_t::kInt64Type:
    return codecName + ".read_int64()";
  case BuiltinType::builtin_type_t::kUInt8Type:
    return codecName + ".read_uint8()";
  case BuiltinType::builtin_type_t::kUInt16Type:
    return codecName + ".read_uint16()";
  case BuiltinType::builtin_type_t::kUInt32Type:
    return codecName + ".read_uint32()";
  case BuiltinType::builtin_type_t::kUInt64Type:
    return codecName + ".read_uint64()";
  case BuiltinType::builtin_type_t::kFloatType:
    return codecName + ".read_float()";
  case BuiltinType::builtin_type_t::kDoubleType:
    return codecName + ".read_double()";
  case BuiltinType::builtin_type_t::kStringType:
    return codecName + ".read_string()";
  case BuiltinType::builtin_type_t::kBinaryType:
    return codecName + ".read_binary()";
  default:
    return "Default::default()";
  }
}

std::string
RustGenerator::generateBuiltinTypeDefaultWrite(BuiltinType *builtinType,
                                               const std::string &codecName) {
  switch (builtinType->getBuiltinType()) {
  case BuiltinType::builtin_type_t::kBoolType:
    return codecName + ".write_bool(bool::default())?";
  case BuiltinType::builtin_type_t::kInt8Type:
    return codecName + ".write_int8(i8::default())?";
  case BuiltinType::builtin_type_t::kInt16Type:
    return codecName + ".write_int16(i16::default())?";
  case BuiltinType::builtin_type_t::kInt32Type:
    return codecName + ".write_int32(i32::default())?";
  case BuiltinType::builtin_type_t::kInt64Type:
    return codecName + ".write_int64(i64::default())?";
  case BuiltinType::builtin_type_t::kUInt8Type:
    return codecName + ".write_uint8(u8::default())?";
  case BuiltinType::builtin_type_t::kUInt16Type:
    return codecName + ".write_uint16(u16::default())?";
  case BuiltinType::builtin_type_t::kUInt32Type:
    return codecName + ".write_uint32(u32::default())?";
  case BuiltinType::builtin_type_t::kUInt64Type:
    return codecName + ".write_uint64(u64::default())?";
  case BuiltinType::builtin_type_t::kFloatType:
    return codecName + ".write_float(f32::default())?";
  case BuiltinType::builtin_type_t::kDoubleType:
    return codecName + ".write_double(f64::default())?";
  case BuiltinType::builtin_type_t::kStringType:
    return codecName + ".write_string(&String::default())?";
  case BuiltinType::builtin_type_t::kBinaryType:
    return codecName + ".write_binary(&Vec::<u8>::default())?";
  default:
    return codecName +
           ".write_uint8(u8::default())?"; // Fallback for unknown types
  }
}

////////////////////////////////////////////////////////////////////////////////
// Template data - This will be populated when we create the template
////////////////////////////////////////////////////////////////////////////////

// Template strings converted from text files by txt_to_c.py.
extern const char *const kRustTemplate;
