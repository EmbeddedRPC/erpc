/*
 * The Clear BSD License
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted (subject to the limitations in the disclaimer below) provided
 *  that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _EMBEDDED_RPC__SYMBOLSCANNER_H_
#define _EMBEDDED_RPC__SYMBOLSCANNER_H_

#include "AstWalker.h"
#include "types/AliasType.h"
#include "types/EnumMember.h"
#include "types/EnumType.h"
#include "types/Interface.h"
#include "types/Program.h"
#include "types/StructType.h"
#include "types/SymbolScope.h"
#include "types/UnionType.h"

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpcgen {

/*!
 * @brief Scans for symbol names.
 */
class SymbolScanner : public AstWalker
{
public:
    /*!
     * @brief This function is constructor of symbol scanner class.
     *
     * This function set their variables to default values, when object is created.
     *
     * @param[in] globals Global symbol scope variable.
     */
    SymbolScanner(SymbolScope *globals)
    : m_globals(globals)
    , m_currentInterface(nullptr)
    , m_currentStruct(nullptr)
    , m_currentProgram(nullptr)
    , m_currentEnum(nullptr)
    , m_currentAlias(nullptr)
    , m_currentUnion(nullptr)
    , m_isNewInterface(false)
    {
    }

    /*!
     * @brief This function is constructor of symbol scanner class.
     *
     * This function set their variables to default values, when object is created.
     *
     * @param[in] globals Global symbol scope variable.
     * @param[in] fileName name of IDL from which AST was built.
     */
    SymbolScanner(SymbolScope *globals, std::string fileName)
    : AstWalker(fileName)
    , m_globals(globals)
    , m_currentInterface(nullptr)
    , m_currentStruct(nullptr)
    , m_currentProgram(nullptr)
    , m_currentEnum(nullptr)
    , m_currentAlias(nullptr)
    , m_currentUnion(nullptr)
    , m_isNewInterface(false)
    {
    }

    /*!
     * @brief Destructor.
     */
    virtual ~SymbolScanner() {}

protected:
    SymbolScope
        *m_globals;                                          /*!< SymbolScope contains information about generating data types, functions, and interfaces.*/
    Interface *m_currentInterface;                           /*!< Contains pointer to interface when interface is handled. */
    StructType *m_currentStruct;                             /*!< Contains pointer to structure when structure is handled. */
    Program *m_currentProgram;                               /*!< Contains pointer to program when program was set in parsed file. */
    EnumType *m_currentEnum;                                 /*!< Contains pointer to enumeration when enumeration is handled. */
    AliasType *m_currentAlias;                               /*!< Contains pointer to alias when alias is handled. */
    UnionType *m_currentUnion;                               /*!< Contains pointer to union when union is handled. */
    std::vector<UnionCase *> m_emptyUnionCases;              /*!< Vector of union cases. */
    bool m_isNewInterface;                                   /*!< When next interface is handled. */
    std::map<std::string, DataType *> m_forwardDeclarations; /*!< To keep forward declarations. */

    /*!
     * @brief This function is called at the end of scanning.
     *
     * @param[in] node Root node.
     *
     * @see rest of AstNode handle functions
     */
    virtual void handleRoot(AstNode *node, bottom_up);

    /*!
     * @brief This function start handle program.
     *
     * @param[in] node AST node program (type definition).
     *
     * @retval nullptr Always return null pointer.
     *
     * @see rest of AstNode handle functions
     */
    virtual AstNode *handleProgram(AstNode *node, top_down);

    /*!
     * @brief This function start handle program.
     *
     * @param[in] node AST node program (type definition).
     *
     * @retval nullptr Always return null pointer.
     *
     * @see rest of AstNode handle functions
     */
    virtual AstNode *handleProgram(AstNode *node, bottom_up);

    /*!
     * @brief This function handles a constant definition
     *
     * @param[in] node AST node const type definition).
     *
     * @retval nullptr Always return null pointer.
     *
     * @see rest of AstNode handle functions
     */
    virtual AstNode *handleConst(AstNode *node, bottom_up);

    /*!
     * @brief This function start handle type.
     *
     * The function set alias (type definition) to symbol scope variable m_globals.
     * Also set name and data type to alias. Bool variable m_isInTypedef is set to true.
     *
     * @param[in] node AST node type alias (type definition).
     *
     * @retval nullptr Always return null pointer.
     *
     * @see rest of AstNode handle functions
     */
    virtual AstNode *handleType(AstNode *node, top_down);

    /*!
     * @brief This function end handle type.
     *
     * The function set bool variable m_isInTypedef to false.
     *
     * @param[in] node AST node type alias (type definition).
     *
     * @retval nullptr Always return null pointer.
     *
     * @see rest of AstNode handle functions
     */
    virtual AstNode *handleType(AstNode *node, bottom_up);

    /*!
     * @brief This function end handle enum.
     *
     * The function set enum variable m_currentEnum to null.
     *
     * @param[in] node AST node type enum.
     *
     * @retval nullptr Always return null pointer.
     *
     * @see rest of AstNode handle functions
     */
    virtual AstNode *handleEnum(AstNode *node, top_down);

    /*!
     * @brief This function end handle enum.
     *
     * The function set enum variable m_currentEnum to null.
     *
     * @param[in] node AST node type enum.
     *
     * @retval nullptr Always return null pointer.
     *
     * @see rest of AstNode handle functions
     */
    virtual AstNode *handleEnum(AstNode *node, bottom_up);

    /*!
     * @brief This function end handle enum member.
     *
     * The function set to enum member variable m_currentEnumMember to null.
     *
     * @param[in] node AST node type enum member.
     *
     * @retval nullptr Always return null pointer.
     *
     * @see rest of AstNode handle functions
     */
    virtual AstNode *handleEnumMember(AstNode *node, bottom_up);

    /*!
     * @brief Check to see if enum member is assigned a value by the user
     *
     * @param[in] enumMember AST node type enum member.
     *
     * @retval Returns whether or not the enum member has user-defined value.
     */
    bool enumMemberHasValue(AstNode *enumMember);

    /*!
     * @brief This function start handle expression.
     *
     * @param[in] node AST node type expression.
     *
     * @retval nullptr Always return null pointer.
     *
     * @see rest of AstNode handle functions
     */
    virtual AstNode *handleExpr(AstNode *node, bottom_up);

    /*!
     * @brief This function end handle binary operator.
     *
     * This function handle binary operator for integer and float value.
     *
     * @param[in] node AST node.
     *
     * @return New AstNode with binary result.
     * @retval nullptr Return null pointer, when left or right token value of given AstNode
     *          can't be cast to IntegerValue or FloatValue.
     *
     * @exception internal_error Thrown if is used unknown binary operator for integer or float value.
     * @exception semantic_error Thrown if is used invalid binary operators on float value.
     *
     * @see AstNode * SymbolScanner::handleExpr()
     * @see AstNode * SymbolScanner::handleUnaryOp()
    */
    virtual AstNode *handleBinaryOp(AstNode *node, bottom_up);

    /*!
     * @brief This function end handle unary operator.
     *
     * This function handle unary operator for integer and float value.
     *
     * @param[in] node AST node.
     *
     * @return New AstNode with integer or float result.
     * @retval nullptr Return null pointer, when value of AstNode token can't be cast
     *          to FloatValue or IntegerValue.
     *
     * @exception internal_error Thrown if is used unknown unary operator for integer or float value.
     * @exception semantic_error Thrown if is used unary operator '~' on float value.
     *
     * @see AstNode * SymbolScanner::handleExpr()
     * @see AstNode * SymbolScanner::handleBinaryOp()
     */
    virtual AstNode *handleUnaryOp(AstNode *node, bottom_up);

    /*!
     * @brief This function start handle struct.
     *
     * The function add struct to symbol scope variable m_globals.
     * For struct is also set name and this struct is set to struct variable m_currentStruct.
     *
     * @param[in] node AST node type struct.
     *
     * @retval nullptr Always return null pointer.
     *
     * @see rest of AstNode handle functions
     *
     * @exception semantic_error Thrown if an given node has not children and is not alias (type definition).
     */
    virtual AstNode *handleStruct(AstNode *node, top_down);

    /*!
     * @brief This function end handle struct.
     *
     * The function set struct variable m_currentStruct to null.
     *
     * @param[in] node AST node type struct.
     *
     * @retval nullptr Always return null pointer.
     *
     * @see rest of AstNode handle functions
     */
    virtual AstNode *handleStruct(AstNode *node, bottom_up);

    /*!
     * @brief This function end handle struct member.
     *
     * The function add struct member to struct m_currentStruct variable.
     * For struct member is also set name and data type.
     *
     * @param[in] node AST node type struct member.
     *
     * @retval nullptr Always return null pointer.
     *
     * @see rest of AstNode handle functions
     */
    virtual AstNode *handleStructMember(AstNode *node, bottom_up);

    // TODO: Update doxygen for union functions
    /*!
     * @brief Handles Union data type while traversing down the AST
     *
     * The function sets the variable m_currentUnion.
     *
     * @param[in] node AST node type struct.
     *
     * @retval nullptr Always return null pointer.
     *
     * @see rest of AstNode handle functions
     */
    virtual AstNode *handleUnion(AstNode *node, top_down);

    /*!
     * @brief Handles Union data type while traversing down the AST
     *
     * The function add m_currentUnion to m_currentStruct.
     *
     * @param[in] node AST node type struct.
     *
     * @retval nullptr Always return null pointer.
     *
     * @see rest of AstNode handle functions
     */
    virtual AstNode *handleUnion(AstNode *node, bottom_up);

    /*!
     * @brief
     *
     * @param[in] node AST node type struct member.
     *
     * @retval nullptr Always return null pointer.
     *
     * @see rest of AstNode handle functions
     */
    virtual AstNode *handleUnionCase(AstNode *node, top_down);

    /*!
     * @brief This function end handle struct member.
     *
     * The function add union cases to union m_currentUnion variable.
     *
     * @param[in] node AST node type struct member.
     *
     * @retval nullptr Always return null pointer.
     *
     * @see rest of AstNode handle functions
     */
    virtual AstNode *handleUnionCase(AstNode *node, bottom_up);

    /*!
     * @brief This function start handle interface.
     *
     * The function add interface to symbol m_globals variable. Also initialize interface functions vector.
     * For interface is also set name.
     *
     * @param[in] node AST node type interface.
     *
     * @retval nullptr Always return null pointer.
     *
     * @throw Throw semantic error, when struct member data type name is same as struct name (nested structs).
     *
     * @see rest of AstNode handle functions
     */
    virtual AstNode *handleInterface(AstNode *node, top_down);

    /*!
     * @brief This function end handle interface.
     *
     * The function set interface functions vector variable to null.
     *
     * @param[in] node AST node type interface.
     *
     * @retval nullptr Always return null pointer.
     *
     * @see rest of AstNode handle functions
     */
    virtual AstNode *handleInterface(AstNode *node, bottom_up);

    /*!
     * @brief This function start handle function.
     *
     * The function add function to interface functions vector. Also initialize function parameters vector.
     * For function is also set name and return type.
     *
     * @param[in] node AST node type function.
     *
     * @retval nullptr Always return null pointer.
     *
     * @see rest of AstNode handle functions
     */
    virtual AstNode *handleFunction(AstNode *node, top_down);

    /*!
     * @brief This function end handle function.
     *
     * The function set function parameters vector variable to null.
     *
     * @param[in] node AST node type function.
     *
     * @retval nullptr Always return null pointer.
     *
     * @see rest of AstNode handle functions
     */
    virtual AstNode *handleFunction(AstNode *node, bottom_up);

    /*!
     * @brief This function start handle function parameter.
     *
     * The function add function parameter to function parameters vector.
     * The function also set name and data type to parameter.
     *
     * @param[in] node AST node type parameter.
     *
     * @retval nullptr Always return null pointer.
     *
     * @see rest of AstNode handle functions
     *
     * @exception semantic_error Thrown if given ast node has not child with token type TOK_IN, TOK_OUT, TOK_INOUT.
     */
    virtual AstNode *handleParam(AstNode *node, top_down);

    /*!
     * @brief This function end handle function parameter.
     *
     * @param[in] node AST node type parameter.
     *
     * @retval nullptr Always return null pointer.
     *
     * @see rest of AstNode handle functions
     *
     * @exception semantic_error Thrown if given ast node has not child with token type TOK_IN, TOK_OUT, TOK_INOUT.
     */
    virtual AstNode *handleParam(AstNode *node, bottom_up);

    /*!
     * @brief set parameter direction: in, out, inout
     *
     * @param[in] param Current parameter
     * @param[in] directionNode Direction node in AST
     *
     */
    void setParameterDirection(StructMember *param, AstNode *directionNode);

    /*!
     * @brief Get value from symbol table for name defined by given token
     *
     * @param[in] tok Token with symbol name defined
     *
     * @retval Value object for name defined by given token
     *
     * @exception syntax_error Thrown, when token value is null or when symbol is not defined in global symbol scope.
     * @exception semantic_error Thrown, when symbol type is not constant or enum member.
     */
    Value *getValueFromSymbol(Token &tok);

    /*!
     * @brief This function returns the data type for a constant variable
     *
     * @param[in] typeNode Node referring to the data type
     *
     * @return DataType of the constant variable
     *
     */
    DataType *getDataTypeForConst(AstNode *typeNode);

    /*!
     * @brief Returns true when given Astnode is enum or structure declaration.
     *
     * @param[in] typeNode Node, which data type is searched.
     *
     * @retval true when AstNode is enum/struct declaration.
     * @retval false when AstNode is not enum/struct declaration.
     */
    bool containsStructEnumDeclaration(const AstNode *typeNode);

    /*!
     * @brief This function returns pointer to new type object.
     *
     * The function returns pointer to new data type object (array, list, alias, ...).
     * Created data type depends on given node token.
     *
     * @param[in] typeNode Node, which data type is searched.
     *
     * @return Return new data type.
     *
     * @see DataType * SymbolScanner::createListType(const AstNode * typeNode, const std::string &name)
     * @see DataType * SymbolScanner::createArrayType(const AstNode * typeNode, const std::string &name)
     * @see DataType * SymbolScanner::lookupDataTypeByName(const Token * tok, const std::string &name)
     *
     * @exception internal_error Thrown if given ast node has not token type: TOK_IDENT, TOK_LIST, TOK_ARRAY.
     */
    DataType *lookupDataType(const AstNode *typeNode);

    /*!
     * @brief This function returns pointer to new list object.
     *
     * The function returns pointer to new list object with set list variable name for list size and nested element
     * type.
     *
     * @param[in] typeNode Node, which has list token type.
     *
     * @return Return new list data type.
     *
     * @see DataType * SymbolScanner::createArrayType(const AstNode * typeNode, const std::string &name)
     * @see DataType * SymbolScanner::lookupDataTypeByName(const Token * tok, const std::string &name)
     */
    DataType *createListType(const AstNode *typeNode);

    /*!
     * @brief This function returns pointer to new array object.
     *
     * The function returns pointer to new array object with set array size and nested element type.
     *
     * @param[in] typeNode Node, which has array token type.
     *
     * @return Return new array data type.
     *
     * @see DataType * SymbolScanner::createListType(const AstNode * typeNode, const std::string &name)
     * @see DataType * SymbolScanner::lookupDataTypeByName(const Token * tok, const std::string &name)
     */
    DataType *createArrayType(const AstNode *typeNode);

    /*!
     * @brief This function returns pointer to new type object.
     *
     * The function returns pointer to new type object, which can be simple data type or alias.
     *
     * @param[in] tok Token with data type name.
     * @param[in] scope Scope determines searching area.
     * @param[in] recursive Recursive searching given scope.
     *
     * @return Return data type, which is not array or list.
     *
     * @see DataType * SymbolScanner::createListType(const AstNode * typeNode, const std::string &name)
     * @see DataType * SymbolScanner::createArrayType(const AstNode * typeNode, const std::string &name)
     *
     * @exception semantic_error Thrown if given token has not name located in symbol scope variable m_globals.
     * @exception semantic_error Thrown if given token can not be transformed to data type.
     */
    DataType *lookupDataTypeByName(const Token &tok, SymbolScope *scope, bool recursive = true);

    /*!
     * @brief This function returns token integer value.
     *
     * The function returns token integer value, when when given AstNode is expression and his child token is equal
     * TOK_INT_LITERAL. Otherwise throw error.
     *
     * @param[in] exprNode Expression node.
     *
     * @return Integer value from expression node.
     *
     * @exception semantic_error Thrown if given ast node has not child with token type TOK_INT_LITERAL.
     */
    uint32_t getIntExprValue(const AstNode *exprNode);

    /*!
     * @brief Determines if the right hand side of a constant declaration
     * is a string literal.
     *
     * @param[in] rhs Node referring to the rhs expression.
     *
     * @retval True when given ast node is string literal, else false.
     */
    bool rhsIsAStringLiteral(AstNode *rhs);

    /*!
     * @brief Determines if the right hand side of a constant declaration
     * is an expression
     *
     * @param[in] rhs Node referring to the rhs expression.
     *
     * @retval True when given ast node is expression, else false.
     */
    bool rhsIsAnExpression(AstNode *rhs);

    /*!
     * @brief Gets the value object for the const variable
     *
     * @param[in] node Parent const node
     * @param[in] constDataType const variable's data type
     *
     * @return Literal value assigned to const variable
     */
    Value *getValueForConst(AstNode *const node, DataType *const constDataType);

    /*!
     * @brief This function add annotations to vector of symbol annotations.
     *
     * @param[in] childTok AstNode contains annotations information.
     * @param[in] symbol Symbol containing vector of annotations belongs to him.
     */
    void addAnnotations(AstNode *childTok, Symbol *symbol);

    /*!
     * @brief This function check annotation just before it will be added to symbol.
     *
     * @param[in] annotation Node containing information about annotation.
     * @param[in] symbol Symbol containing vector of annotations belongs to him.
     */
    void checkAnnotationBeforeAdding(AstNode *annotation, Symbol *symbol);

    /*!
     * @brief Helper function to get Value from annotation AstNode
     *
     * @param[in] annotationNode AstNode pointing to the annotation
     *
     * @return Value pointer for annotation
     */
    Value *getAnnotationValue(AstNode *annotationNode);

    /*!
     * @brief Controlling annotations used on structure members.
     *
     * Struct members are examined for @length and @max_length annotations, and the length member is denoted.
     * This function is also used on function parameters, since they are represented as structs.
     */
    void scanStructForAnnotations();

    /*!
     * @brief Check if annotation is integer number or integer type variable.
     *
     * Annotation can contain reference to integer data type or it can be integer number.
     * Referenced integer data type can be presented in global scope or in same structure scope.
     *
     * @param[in] ann Annotation to check.
     */
    void checkIfAnnValueIsIntNumberOrIntType(Annotation *ann);

    /*!
     * @brief This function sets to given symbol given doxygen comments.
     *
     * Comments can be placed above declaration or as trailing comments.
     *
     * @param[in] symbol Symbol where doxygen comments will be added.
     * @param[in] above Doxygen comments placed above.
     * @param[in] trailing Trailing doxygen comments.
     */
    void addDoxygenComments(Symbol *symbol, AstNode *above, AstNode *trailing);

    /*!
     * @brief This function creates new function parameter.
     *
     * Function parameter information are set based on given structure member, which is
     * param member of function type.
     *
     * @param[in] structMember Function type param member.
     * @param[in] name Param name.
     *
     * @return new function (callback) parameter.
     */
    StructMember *createCallbackParam(StructMember *structMember, const std::string &name);

    /*!
     * @brief This function registratate forward union/structure declarations.
     *
     * @param[in] dataType Union/structure data type.
     */
    void addForwardDeclaration(DataType *dataType);

    /*!
     * @brief This function unregister union/structure declarations.
     *
     * @param[in] dataType Union/structure data type.
     */
    void removeForwardDeclaration(DataType *dataType);

    /*!
     * @brief This function add symbol into global symbol scope.
     *
     * @param[in] dataType Union/structure data type.
     */
    void addGlobalSymbol(Symbol *symbol);
};

} // namespace erpcgen

#endif // _EMBEDDED_RPC__SYMBOLSCANNER_H_
