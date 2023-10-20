/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__CGENERATOR_H_
#define _EMBEDDED_RPC__CGENERATOR_H_

#include "Generator.hpp"
#include "cpptempl.hpp"
#include "types/Group.hpp"

#include <set>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpcgen {
/*!
 * @brief Code generator for C.
 */
class CGenerator : public Generator
{
public:
    /*!
     * @brief This function is constructor of CGenerator class.
     *
     * Interface definition contains all information about parsed files and builtin types.
     *
     * @param[in] def Contains all Symbols parsed from IDL files.
     */
    explicit CGenerator(InterfaceDefinition *def);

    /*!
     * @brief This function is destructor of CGenerator class.
     *
     * This function close opened files.
     */
    virtual ~CGenerator() {}

    /*!
     * @brief This function generate output code for output files.
     *
     * This code call all necessary functions for prepare output code and parse it into output files.
     */
    virtual void generate() override;

private:
    enum class direction_t
    {
        kIn,
        kOut,
        kInOut,
        kNone
    };

    cpptempl::data_list m_symbolsTemplate; /*!< List of all symbol templates */

    std::vector<ListType *>
        m_listBinaryTypes; /*!<
                            * Contains binary types transformed to list<uint8>.
                            * More ListType are present when @length annotation is used for binary type.
                            * If binary without @length is used then it is placed on first place in this vector.
                            */

    std::vector<StructType *> m_structListTypes; /*!<
                                                  * Contains list types transformed to struct{list<>}.
                                                  * To distinguish between user defined struct{list<>} and transformed
                                                  * list<> to struct{list<>}.
                                                  */

    /*!
     * @brief This function prepare helpful functions located in template files.
     *
     * These function may be used while parsing templates
     */
    void parseSubtemplates();

    /*!
     * @brief This function generate output files.
     *
     * This function call functions for generating client/server header/source files.
     *
     * @param[in] fileNameExtension Extension for file name (for example for case that each interface will be generated
     * in its set of output files).
     */
    void generateOutputFiles(const std::string &fileNameExtension) override;

    /*!
     * @brief This function generate header file output with common eRPC code.
     *
     * @param[in] fileName Name for common eRPC header file output.
     */
    void generateCommonCHeaderFiles(std::string fileName);

    /*!
     * @brief This function generate header file output with common eRPC code.
     *
     * @param[in] fileName Name for common eRPC header file output.
     */
    void generateCommonCppHeaderFiles(std::string fileName);

    /*!
     * @brief This function generate output interface header file.
     *
     * @param[in] fileName Name for output interface header file.
     */
    void generateInterfaceCppHeaderFile(std::string fileName);

    /*!
     * @brief This function generate output interface source file.
     *
     * @param[in] fileName Name for output interface source file.
     */
    void generateInterfaceCppSourceFile(std::string fileName);

    /*!
     * @brief This function generate output client header file for cpp.
     *
     * @param[in] fileName Name for output client header file.
     */
    void generateClientCppHeaderFile(std::string fileName);

    /*!
     * @brief This function generate output client source file for cpp.
     *
     * @param[in] fileName Name for output client source file.
     */
    void generateClientCppSourceFile(std::string fileName);

    /*!
     * @brief This function generate output server header file for cpp.
     *
     * @param[in] fileName Name for output server header file.
     */
    void generateServerCppHeaderFile(std::string fileName);

    /*!
     * @brief This function generate output server source file for cpp.
     *
     * @param[in] fileName Name for output server source file.
     */
    void generateServerCppSourceFile(std::string fileName);

    /*!
     * @brief This function generate output client header file for C.
     *
     * @param[in] fileName Name for output client header file.
     */
    void generateClientCHeaderFile(std::string fileName);

    /*!
     * @brief This function generate output client source file for C.
     *
     * @param[in] fileName Name for output client source file.
     */
    void generateClientCSourceFile(std::string fileName);

    /*!
     * @brief This function generate output server header file for C.
     *
     * @param[in] fileName Name for output server header file.
     */
    void generateServerCHeaderFile(std::string fileName);

    /*!
     * @brief This function generate output server source file for C.
     *
     * @param[in] fileName Name for output server source file.
     */
    void generateServerCSourceFile(std::string fileName);

    /*!
     * @brief This function generate output crc16 source file.
     */
    virtual void generateCrcFile();

    /*!
     * @brief This function transforms alias data type to list or structure.
     */
    void transformAliases();

    /*!
     * @brief This function gets template for symbol specified by name.
     *
     * @param[in] name Symbol name.
     *
     * @return Template for symbol defined by name.
     */
    cpptempl::data_map getSymbolTemplateByName(const std::string &name);

    /*!
     * @brief This function change list and binary data type to structure.
     *
     * This function return given data type or new structure data type.
     * If given data type contains list or binary data type, then this data type will be changed to the structure.
     * New created structure is placed before top data type.
     *
     * @param[inout] dataTypes Vector of transformed data types.
     * @param[in] dataType Top data type.
     *
     * @return Pointer to given or new DataType.
     */
    DataType *findChildDataType(std::set<DataType *> &dataTypes, DataType *dataType) override;

    /*!
     * @brief This function transform binary data type to list and set annotation to it.
     *
     * @param[in] structMember Structure member, Function parameter or Union member.
     */
    void setBinaryList(StructMember *structMember) override;

    /*!
     * @brief This function returns function base template data.
     *
     * This function returns function base template data with all data, which
     * are necessary for generating output code for output files.
     *
     * @param[in] group Group to which function belongs.
     * @param[in] fn From this are set function base template data.
     *
     * @return Contains function base data.
     */
    cpptempl::data_map getFunctionBaseTemplateData(Group *group, FunctionBase *fn);

    /*!
     * @brief This function returns interface function template data.
     *
     * This function returns interface function template data with all data, which
     * are necessary for generating output code for output files.
     *
     * @param[in] group Group to which function belongs.
     * @param[in] fn From this are set interface function template data.
     *
     * @return Contains interface function data.
     */
    cpptempl::data_map getFunctionTemplateData(Group *group, Function *fn) override;

    /*!
     * @brief This function returns function type (callbacks type) template data.
     *
     * This function returns function type (callbacks type) template data with all data, which
     * are necessary for generating output code for output files. Shim code is generating
     * common function for serialization/deserialization of data.
     *
     * @param[in] group Group to which function belongs.
     * @param[in] fn From this are set function type template data.
     *
     * @return Contains interface function data.
     */
    cpptempl::data_map getFunctionTypeTemplateData(Group *group, FunctionType *fn) override;

    /*!
     * @brief This function will get symbol comments and convert to language specific ones
     *
     * @param[in] symbol Pointer to symbol.
     * @param[inout] symbolInfo Data map, which contains information about symbol.
     */
    void setTemplateComments(Symbol *symbol, cpptempl::data_map &symbolInfo) override;

    /*!
     * @brief This function sets const template data.
     *
     * This function sets const template data with all data, which
     * are necessary for generating output code for output files.
     */
    void makeConstTemplateData();

    // Functions that populate type-specific template data

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
    cpptempl::data_map makeGroupSymbolsTemplateData(Group *group) override;

    /*!
     * @brief This function sets group callbacks template data.
     *
     * This function sets group callbacks template data with all data, which
     * are necessary for generating output code for output files.
     *
     * @param[in] group Pointer to a group.
     *
     * @return Data list of group function types (callback types).
     */
    cpptempl::data_list makeGroupCallbacksTemplateData(Group *group);

    /*!
     * @brief This function returns template data for given structure.
     *
     * This function return structure's template data with necessary data
     * for generating output code for structure declaration.
     *
     * @param[in] structType Given structure type.
     *
     * @return Data map with structure template data.
     */
    cpptempl::data_map getStructDeclarationTemplateData(StructType *structType);

    /*!
     * @brief This function return template data for given structure.
     *
     * This function return structure's template data with all data, which
     * are necessary for generating output code for output files.
     *
     * It needs to have struct declaration as an input.
     *
     * @param[in] group Pointer to a group.
     * @param[in] structType Given structure type.
     * @param[in] structInfo Struct declaration template.
     *
     * @return Data map with structure template data.
     */
    cpptempl::data_map getStructDefinitionTemplateData(Group *group, StructType *structType,
                                                       cpptempl::data_map structInfo);

    /*!
     * @brief This function returns template data for given union.
     *
     * This function return union template data with necessary data
     * for generating output code for union declaration.
     *
     * @param[in] unionType Given union type.
     *
     * @return Data map with union template data.
     */
    cpptempl::data_map getUnionDeclarationTemplateData(UnionType *unionType);

    /*!
     * @brief This function return template data for given union.
     *
     * This function return union template data with all data, which
     * are necessary for generating output code for output files.
     *
     * It needs to have union declaration as an input.
     *
     * @param[in] group Pointer to a group.
     * @param[in] unionType Given union type.
     * @param[in] unionInfo Union declaration template.
     * @param[inout] needUnionsServerFree Bool indication if server code needs to have deallocation of the union.
     *
     * @return Data map with union template data.
     */
    cpptempl::data_map getUnionDefinitionTemplateData(Group *group, UnionType *unionType, cpptempl::data_map &unionInfo,
                                                      bool &needUnionsServerFree);
    /*!
     * @brief This function sets union cases template data.
     *
     * This function sets unions cases template data for given union, which
     * is necessary for generating output code for output files.
     *
     * @param[in] unionType Union, which contains union cases.
     * @param[in] unionInfo Data map for which data should be set.
     */
    void setUnionMembersTemplateData(UnionType *unionType, cpptempl::data_map &unionInfo);

    /*!
     * @brief This function returns union cases declaration description to union.
     *
     * This function returns unions cases declaration description to union, which
     * is necessary for generating union encapsulated data type.
     *
     * @param[in] unionType Union, which contains union cases.
     * @param[in] indent Additional indent used for member data.
     *
     * @return Union cases declaration description.
     */
    std::string getUnionMembersData(UnionType *unionType, std::string indent);

    /*!
     * @brief This function sets enum template data.
     *
     * This function sets enum template data with all data, which
     * are necessary for generating output code for output files.
     */
    void makeEnumsTemplateData();

    /*!
     * @brief This function sets enum template data.
     *
     * This function sets enum template data with all data, which
     * are necessary for generating output code for output files.
     */
    void makeSymbolsDeclarationTemplateData();

    /*!
     * @brief This function return template data for given enum.
     *
     * This function return enum's template data with all data, which
     * are necessary for generating output code for output files.
     *
     * @param[in] enumType Given enum type.
     *
     * @return Contain enum template data.
     */
    cpptempl::data_map getEnumTemplateData(EnumType *enumType);

    /*!
     * @brief This function return enum members template data.
     *
     * This function return enum members template data with all data, which
     * are necessary for generating output code for output files.
     *
     * @param[in] enumType Pointer to enum.
     *
     * @return Contains enum members data.
     */
    cpptempl::data_list getEnumMembersTemplateData(EnumType *enumType);

    /*!
     * @brief This function sets alias (type definition) template data.
     *
     * This function sets alias template data with all data, which
     * are necessary for generating output code for output files.
     */
    void makeAliasesTemplateData();

    /*!
     * @brief This function returns alias type of given DataType.
     *
     * @return AliasType or nullptr, when alias was not find.
     */
    AliasType *getAliasType(DataType *dataType);

    /*!
     * @brief This function returns alias name of given DataType.
     *
     * @return Alias name or empty string, when alias was not find.
     */
    std::string getAliasName(DataType *dataType);

    /*!
     * @brief This function return necessary template data for data type.
     *
     * This function return data type template data with all necessary data, which
     * are necessary for generating output code for output files.
     *
     * @param[in] t From this are set data type template data.
     * @param[in] isFunction If set to true return parameters will have
     *          comma at start of sequence.
     *
     * @return Contains data type template data.
     */
    cpptempl::data_map getTypeInfo(DataType *t, bool isFunction);

    /*!
     * @brief This function returns error return value for given function.
     *
     * @param[in] fn Function from which is returned error return value.
     *
     * @return Return string representation of error return value.
     */
    std::string getErrorReturnValue(FunctionBase *fn);

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
    std::string getFunctionPrototype(Group *group, FunctionBase *fn, const std::string &interfaceName = "",
                                     const std::string &name = "", bool insideInterfaceCall = false) override;

    /*!
     * @brief This function return interface function representation called by server side.
     *
     * @param[in] fn Function for interface function representation.
     * @param[in] isCCall C and C++ code is similar, but not same.
     *
     * @return String representation for given function.
     */
    std::string getFunctionServerCall(Function *fn, bool isCCall = false);

    /*!
     * @brief This function return name with guard.
     *
     * @param[in] filename File name for guard name.
     *
     * @return Guard name.
     */
    std::string generateIncludeGuardName(const std::string &filename);

    /*!
     * @brief This function return string representation for given BuiltinType.
     *
     * @param[in] t Builtin type.
     *
     * return std::string String representation for given builtin type.
     */
    std::string getBuiltinTypename(const BuiltinType *t);

    /*!
     * @brief This function set to variable returnName type or type with variable name.
     *
     * This function set to variable returnName "type" if given returnName is "",
     * or return "type with variable name", when given returnName is set to
     * variable name.
     *
     * @param[in] t data type.
     * @param[in] name Return type name.
     *
     * @return Return data type name with given name.
     *
     * @exception internal_error Thrown, when unknown data type is called.
     */
    std::string getTypenameName(DataType *t, const std::string &name);

    /*!
     * @brief This function return necessary template data for decode and encode data type.
     *
     * This function prepare data for decode or encode functions in c_coders template file.
     *
     * @param[in] name Variable name.
     * @param[in] t Variable data type.
     * @param[in] structType Structure holdings structure members.
     * @param[in] inDataContainer Is inside data container (struct, list, array).
     * @param[in] structMember Null for return.
     * @param[out] needTempVariableI32 Return true, when data type contains enum, function, union type.
     * @param[in] isFunctionParam True for function param else false (structure member).
     *
     * @return Template data for decode or encode data type.
     */
    cpptempl::data_map getEncodeDecodeCall(const std::string &name, Group *group, DataType *t, StructType *structType,
                                           bool inDataContainer, StructMember *structMember, bool &needTempVariableI32,
                                           bool isFunctionParam);

    /*!
     * @brief This function add to template data, which kind of BuiltinType is data type.
     *
     * @param[in] t Data type.
     * @param[out] templateData Template data.
     * @param[in] structType Structure holdings structure members.
     * @param[in] structMember Null for return.
     * @param[in] isFunctionParam True for function param else false (structure member).
     */
    void getEncodeDecodeBuiltin(Group *group, BuiltinType *t, cpptempl::data_map &templateData, StructType *structType,
                                StructMember *structMember, bool isFunctionParam);

    /*!
     * @brief This function encapsulate gave name, if it start with pointer.
     *
     * It is necessary for out and inout parameters, because int *a[] is not same as int (*a)[].
     *
     * @param[inout] name Variable name.
     */
    void giveBracesToArrays(std::string &name);

    /*!
     * @brief This function return "*", if it is need.
     *
     * It is need by rules of passing data types for each direction type.
     *
     * @param[in] structMember Contains direction type and data type.
     *
     * @return Pointer or empty.
     */
    std::string getExtraDirectionPointer(StructMember *structMember);

    /*!
     * @brief This function return "*", if it is need.
     *
     * It is need by rules of taking data types from return type.
     *
     * @param[in] dataType Data type.
     *
     * @return Pointer or empty.
     */
    std::string getExtraPointerInReturn(DataType *dataType);

    /*!
     * @brief This function call first erpc_alloc on server side for parameters if it is need (except out param).
     *
     * It is need by rules of passing data types for each direction type.
     *
     * @param[in] name Parameter name.
     * @param[in] structMember Contains direction type and data type.
     *
     * @return Erpc_alloc function or empty.
     */
    cpptempl::data_map firstAllocOnServerWhenIsNeed(const std::string &name, StructMember *structMember);

    /*!
     * @brief This function call first erpc_alloc on client side return statement if it is need.
     *
     * It is need by rules of taking data types from return type.
     *
     * @param[in] name Parameter name.
     * @param[in] dataType Contains data type information.
     *
     * @return Erpc_alloc function or empty.
     */
    cpptempl::data_map firstAllocOnReturnWhenIsNeed(const std::string &name, DataType *dataType);

    /*!
     * @brief This function return call for alloc space based on given data type.
     *
     * @param[in] name Name, for which is allocating space.
     * @param[in] symbol Given symbol type contains annotations and data type.
     *
     * @return Return erpc_alloc call or empty string.
     */
    cpptempl::data_map allocateCall(const std::string &name, Symbol *symbol);

    /*!
     * @brief This function will add to given lists given map based on given by symbol direction.
     *
     * Based on directions of structures or functions parameters, given map will be added to given lists.
     *
     * @param[in] symbolType Contains structure or function parameter.
     * @param[in] directions Set of directions
     * @param[in,out] toClient List of data types designed for client direction.
     * @param[in,out] toServer List of data types designed for server direction.
     * @param[in] dataMap Map with information about structure or function parameter.
     */
    void setSymbolDataToSide(const Symbol *symbolType, const std::set<param_direction_t> &directions,
                             cpptempl::data_list &toClient, cpptempl::data_list &toServer, cpptempl::data_map &dataMap);

    /*!
     * @brief This function returns true, when given data type need be freed.
     *
     * @param[in] dataType Given data type.
     *
     * @retval true When given data type need be freed.
     * @retval false When given data type don't need be freed (like int, enum).
     */
    bool isNeedCallFree(DataType *dataType);

    /*!
     * @ This function set variables for calling first freeing function on server side for parameters, when it is need.
     *
     * It free what was allocated by this function: std::string firstAllocOnServerWhenIsNeed(std::string name,
     * StructMember *structMember);
     *
     * @param[in] symbol StructMember when function parameter or DataType when return type.
     * @param[in] info DataMap which contains information about data type for output.
     * @param[in] returnType To recognize param type and return type.
     */
    void setCallingFreeFunctions(Symbol *symbol, cpptempl::data_map &info, bool returnType);

    /*!
     * @brief This function return space if given string is not empty.
     *
     * @param[in] param Given string.
     *
     * @return Space or empty.
     */
    std::string returnSpaceWhenNotEmpty(const std::string &param);

    /*!
     * @brief This function check, if data type contains string data type.
     *
     * @param[in] dataType Given data type.
     *
     * @retval True if data type contains string, else false.
     *
     * @see bool CGenerator::containsList(DataType * dataType)
     * @see bool CGenerator::containsBinary(DataType * dataType)
     */
    bool containsString(DataType *dataType);

    /*!
     * @brief This function check, if data type contains list data type.
     *
     * @param[in] dataType Given data type.
     *
     * @retval True if data type contains list, else false.
     *
     * @see bool CGenerator::containsBinary(DataType * dataType)
     * @see bool CGenerator::containsString(DataType * dataType)
     */
    bool containsList(DataType *dataType);

    /*!
     * @brief This function check, if data type is struct which contains byref parameter and not contains shared
     * annotation.
     *
     * @param[in] dataType Given data type.
     * @param[in] dataTypes For loops from forward declaration detection.
     *
     * @retval True if data type is structure and contains byref parameter and not contains shared annotation, else
     * false.
     *
     */
    bool containsByrefParamToFree(DataType *dataType, std::set<DataType *> &dataTypes);

    /*!
     * @brief This function returns true when structure is used as a wrapper for binary type.
     *
     * Binary type which is not using length annotation is in CGenerator presented as struct{ list<uint8> }.
     *
     * @param[in] structType Potential structure wrapper.
     *
     * @retval true When structure is used as a wrapper for binary type.
     * @retval false When structure is not used as a wrapper for binary type.
     */
    bool isBinaryStruct(StructType *structType);

    /*!
     * @brief This function returns true when list was created for replacing binary type.
     *
     * Binary type which is using length annotation is in CGenerator presented as list<uint8>.
     *
     * @param[in] structType Potential structure wrapper.
     *
     * @retval true When structure is used as a wrapper for binary type.
     * @retval false When structure is not used as a wrapper for binary type.
     */
    bool isBinaryList(const ListType *listType);

    /*!
     * @brief This function returns true when structure is used as a wrapper for list type.
     *
     * @param[in] structType Potential structure wrapper.
     *
     * @retval true When structure is used as a wrapper for list type.
     * @retval false When structure is not used as a wrapper for list type.
     */
    bool isListStruct(const StructType *structType);

    /*!
     * @brief This function returns true when "retain" annotation wasn't set.
     *
     * This annotation have effect for server side of generated files. Allocated space will be not freed by server shim
     * code.
     *
     * @param[in] structMember Function parameter.
     *
     * @retval true When "retain" annotation wasn't set.
     * @retval false When "retain" annotation was set.
     */
    bool generateServerFreeFunctions(StructMember *structMember);

    /*!
     * @brief Set no_shared annotation to struct/union type.
     *
     * This annotation will be set to these data types if one of theirs members will contain mentioned annotation.
     *
     * @param[in] parentSymbol Struct/union type.
     * @param[in] childSymbol It's member type.
     */
    void setNoSharedAnn(Symbol *parentSymbol, Symbol *childSymbol);

    bool setDiscriminatorTemp(UnionType *unionType, StructType *structType, StructMember *structMember,
                              bool isFunctionParam, cpptempl::data_map &templateData);

    /*!
     * @brief This function returns data type name for scalar data type.
     *
     * @param[in] dataType Data type to inspect.
     *
     * @retval Empty string when data type is not scalar.
     * @retval Alias data name when dataType is AliasType.
     * @retval Otherwise output from getBuiltinTypename.
     */
    std::string getScalarTypename(DataType *dataType);

    /*!
     * @brief This function returns string representations of function parameter direction.
     *
     * @param[in] direction Enum direction.
     *
     * @return String representation for given direction.
     */
    std::string getDirection(param_direction_t direction);

    /*!
     * @brief This function returns information if function parameter on server side need be initialized to NULL.
     *
     * @param[in] structMember Function parameter.
     *
     * @retval true When Function parameter need to be initialized to NULL.
     * @retval false When Function parameter don't need to be initialized to NULL.
     */
    bool isServerNullParam(StructMember *structMember);

    /*!
     * @brief This function returns information if function parameter is passed by pointer.
     *
     * @param[in] structMember Function parameter.
     *
     * @retval true When Function parameter is passed by pointer.
     * @retval false When Function parameter isn't passed by pointer.
     */
    bool isPointerParam(StructMember *structMember);

    /*!
     * @brief This function returns information if function parameter is null-able.
     *
     * @param[in] structMember Function parameter.
     *
     * @retval true When Function parameter is null-able.
     * @retval false When Function parameter isn't null-able.
     */
    bool isNullableParam(StructMember *structMember);

    /*!
     * Stores reserved words for C/C++ program language.
     */
    void initCReservedWords();

    /*!
     * @brief Controlling annotations used on structure members.
     *
     * Struct members are examined for @length and @max_length annotations, and the length member is denoted.
     * This function is also used on function parameters, since they are covered by structs.
     *
     * @param[in] currentStructType StrucType to check.
     * @param[in] isFunction To distinguish if given structure is used for function parameters.
     */
    void scanStructForAnnotations(StructType *currentStructType, bool isFunction);

    /*!
     * @brief Check if annotation is integer number or integer type variable.
     *
     * Annotation can contain reference to integer data type or it can be integer number.
     * Referenced integer data type can be presented in global scope or in same structure scope.
     *
     * @param[in] ann Annotation to check.
     * @param[in] currentStructType StrucType to check.
     */
    void checkIfAnnValueIsIntNumberOrIntType(Annotation *ann, StructType *currentStructType);
};
} // namespace erpcgen

#endif // _EMBEDDED_RPC__CGENERATOR_H_
