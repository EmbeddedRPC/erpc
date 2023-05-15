/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * Copyright (c) 2020 Texas Instruments Incorporated
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "SymbolScanner.hpp"

#include "ErpcLexer.hpp"
#include "Logging.hpp"
#include "annotations.h"
#include "smart_ptr.hpp"
#include "types/ArrayType.hpp"
#include "types/BuiltinType.hpp"
#include "types/ConstType.hpp"
#include "types/FunctionType.hpp"
#include "types/ListType.hpp"
#include "types/VoidType.hpp"

#include <algorithm>
#include <cstring>

using namespace erpcgen;
using namespace std;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////
void SymbolScanner::handleRoot(AstNode *node, bottom_up)
{
    (void)node;
    if (m_forwardDeclarations.size() != 0)
    {
        string forwardTypes;
        for (auto it = m_forwardDeclarations.begin(); it != m_forwardDeclarations.end(); ++it)
        {
            if (it != m_forwardDeclarations.begin())
            {
                forwardTypes += ", ";
            }
            forwardTypes += format_string("type name %s: line %d", it->first.c_str(), it->second->getFirstLine());
        }
        throw syntax_error(format_string("Missing type definitions for one or more forward type declarations: %s",
                                         forwardTypes.c_str())
                               .c_str());
    }
}

AstNode *SymbolScanner::handleConst(AstNode *node, bottom_up)
{
    DataType *constDataType = getDataTypeForConst(node->getChild(0));
    Value *constVal = getValueForConst(node, constDataType);
    ConstType *constType = new ConstType(node->getChild(1)->getToken(), constDataType, constVal);
    addAnnotations(node->getChild(3), constType);

    // doxygen comment
    addDoxygenComments(constType, node->getChild(4), node->getChild(5));

    addGlobalSymbol(constType);
    return nullptr;
}

Value *SymbolScanner::getValueForConst(AstNode *const node, DataType *const constDataType)
{
    Value *constVal = nullptr;
    bool isStringConst = constDataType->getTrueDataType()->isString();
    if (rhsIsAStringLiteral(node))
    {
        if (!isStringConst)
        {
            throw syntax_error2("Attempt to assign a string to a non-string data type",
                                node->getChild(0)->getToken().getLocation(), m_fileName);
        }
        else
        {
            constVal = node->getChild(2)->getTokenValue();
        }
    }
    else if (rhsIsAnExpression(node))
    {
        // const node -> virtual expression node -> actual expression (id or number)
        Token &rhsExpressionChildTok = node->getChild(2)->getChild(0)->getToken();
        if (rhsExpressionChildTok.isNumberTok())
        {
            if (isStringConst)
            {
                throw syntax_error2("Attempt to assign a number to a string data type",
                                    node->getChild(0)->getToken().getLocation(), m_fileName);
            }
            else
            {
                constVal = rhsExpressionChildTok.getValue();
            }
        }
        else if (rhsExpressionChildTok.isIdentifierTok())
        {
            constVal = getValueFromSymbol(rhsExpressionChildTok);
            if (kStringValue == constVal->getType())
            {
                if (!isStringConst)
                {
                    delete constVal;
                    throw syntax_error2("Attempt to assign a string to a non-string data type",
                                        node->getChild(0)->getToken().getLocation(), m_fileName);
                }
            }
            else // kStringValue != constVal->getType()
            {
                if (isStringConst)
                {
                    delete constVal;
                    throw syntax_error2("Attempt to assign a number to a string data type",
                                        node->getChild(0)->getToken().getLocation(), m_fileName);
                }
            }
        } // else do nothing
    }
    assert(nullptr != constVal);
    return constVal;
}

bool SymbolScanner::rhsIsAnExpression(AstNode *rhs)
{
    return TOK_EXPR == rhs->getChild(2)->getToken().getToken();
}
bool SymbolScanner::rhsIsAStringLiteral(AstNode *rhs)
{
    return TOK_STRING_LITERAL == rhs->getChild(2)->getToken().getToken();
}

DataType *SymbolScanner::getDataTypeForConst(AstNode *typeNode)
{
    const string &nameOfType = typeNode->getToken().getStringValue();
    if (!m_globals->hasSymbol(nameOfType))
    { // throw typeNotFound exception
        throw syntax_error2("Type '" + nameOfType + "' not a defined type", typeNode->getToken().getLocation(),
                            m_fileName);
    }

    assert(nullptr != dynamic_cast<DataType *>(m_globals->getSymbol(nameOfType)));
    return dynamic_cast<DataType *>(m_globals->getSymbol(nameOfType));
}

AstNode *SymbolScanner::handleType(AstNode *node, top_down)
{
    // Extract new type name.
    AstNode *ident = (*node)[0];
    const Token &tok = ident->getToken();
    const string &name = tok.getStringValue();
    Log::debug("type: %s\n", name.c_str());

    // Find existing type.
    AstNode *typeNode = (*node)[1];
    DataType *dataType = nullptr;
    if (!containsStructEnumDeclaration(typeNode))
    {
        dataType = lookupDataType(typeNode);
    }

    AliasType *type = new AliasType(tok, dataType);

    // Get comment if exist.
    addDoxygenComments(type, node->getChild(3), node->getChild(4));

    m_currentAlias = type;

    return nullptr;
}

AstNode *SymbolScanner::handleType(AstNode *node, bottom_up)
{
    if (m_currentAlias)
    {
        addAnnotations(node->getChild(2), m_currentAlias);
        addGlobalSymbol(m_currentAlias);
        m_currentAlias = nullptr;
    }
    return nullptr;
}

AstNode *SymbolScanner::handleEnum(AstNode *node, top_down)
{
    AstNode *ident = (*node)[0];
    const string *name = nullptr;
    EnumType *newEnum;

    if (ident)
    {
        const Token &tok = ident->getToken();
        name = &(tok.getStringValue());
        Log::debug("enum: %s\n", name->c_str());
        newEnum = new EnumType(tok);
    }
    else
    {
        // TODO: support enum{...} ?		//Log::debug("typedef enum: %s\n",
        // check_null(check_null(check_null(node->getParent())->getChild(0))->getTokenValue())->toString().c_str());
        newEnum = new EnumType();
    }

    Log::debug("Enum: %s\n", newEnum->getName().c_str());

    // Get doxygen comment if exist.
    addDoxygenComments(newEnum, node->getChild(3), node->getChild(4));

    m_currentEnum = newEnum;
    return nullptr;
}

AstNode *SymbolScanner::handleEnum(AstNode *node, bottom_up)
{
    if (m_currentAlias != nullptr)
    {
        if (m_currentAlias->getElementType() == nullptr)
        {
            m_currentAlias->setElementType(m_currentEnum);
        }
        else
        {
            throw internal_error("Alias type should not have assigned data type at this state.");
        }

        if (m_currentEnum->getName().empty())
        {
            m_currentEnum->setName(m_currentAlias->getName());
            delete m_currentAlias;
            m_currentAlias = nullptr;
        }
    }
    else
    {
        addAnnotations(node->getChild(2), m_currentEnum);
        if (m_currentEnum->getMembers().size() == 0 &&
            m_currentEnum->findAnnotation(EXTERNAL_ANNOTATION, Annotation::program_lang_t::kAll) == nullptr)
        {
            throw semantic_error("Enum may have 0 members only when is defined with external annotation.");
        }
    }

    addGlobalSymbol(m_currentEnum);

    // Clear current enum pointer.
    m_currentEnum = nullptr;

    return nullptr;
}

AstNode *SymbolScanner::handleEnumMember(AstNode *node, bottom_up)
{
    // m_currentEnumMember = NULL;
    assert_throw_internal((m_currentEnum), "enum member not in enum");

    AstNode *ident = (*node)[0];
    const Token &tok = ident->getToken();
    const string &name = tok.getStringValue();

    if (enumMemberHasValue(node))
    {
        Value *enumValue = node->getChild(1)->getChild(0)->getTokenValue();
        if (enumValue->getType() == kStringValue)
        {
            enumValue = getValueFromSymbol(node->getChild(1)->getChild(0)->getToken());
        }

        if (enumValue->getType() == kIntegerValue)
        {
            assert(dynamic_cast<IntegerValue *>(enumValue));
            m_currentEnum->setCurrentValue((uint32_t) dynamic_cast<IntegerValue *>(enumValue)->getValue());
        }
        else
        {
            throw semantic_error(format_string("line %d: expected integer as enum's member value",
                                               node->getChild(1)->getChild(0)->getToken().getFirstLine()));
        }
    }
    EnumMember *member = new EnumMember(tok, m_currentEnum->getNextValue());

    Log::debug("enum member: %s = %d\n", name.c_str(), member->getValue());

    m_currentEnum->addMember(member);
    addGlobalSymbol(member);

    // add annotations
    addAnnotations(node->getChild(2), member);

    // doxygen comment
    addDoxygenComments(member, node->getChild(3), node->getChild(4));

    return nullptr;
}

bool SymbolScanner::enumMemberHasValue(AstNode *enumMember)
{
    auto enumValueNode = enumMember->getChild(1);
    if (enumValueNode && TOK_EXPR == enumValueNode->getToken().getToken())
    {
        AstNode *valueNode = enumValueNode->getChild(0);
        if (!valueNode)
        {
            return false;
        }
        auto val = valueNode->getTokenValue();
        if (!val)
        {
            return false;
        }

        return true;
    }
    return false;
}

AstNode *SymbolScanner::handleBinaryOp(AstNode *node, bottom_up)
{
    Log::debug("binop: %s\n", node->getDescription().c_str());

    Token &tok = node->getToken();

    // Get the left operand.
    AstNode *left = (*node)[0];
    assert(left);
    Token &leftTok = left->getToken();

    smart_ptr<Value> leftValue;
    IntegerValue *leftInt;
    FloatValue *leftFloat;

    if (leftTok.isIdentifierTok())
    {
        leftValue = getValueFromSymbol(leftTok);
        leftInt = dynamic_cast<IntegerValue *>(leftValue.get());
        leftFloat = dynamic_cast<FloatValue *>(leftValue.get());
    }
    else if (leftTok.isNumberTok())
    {
        leftInt = dynamic_cast<IntegerValue *>(leftTok.getValue());
        leftFloat = dynamic_cast<FloatValue *>(leftTok.getValue());
    }
    else
    {
        throw semantic_error(
            format_string("expression \"%s\" invalid as binary operand", leftTok.getValue()->toString().c_str()));
    }

    // Get the right operand.
    AstNode *right = (*node)[1];
    assert(right);
    Token &rightTok = right->getToken();

    smart_ptr<Value> rightValue;
    IntegerValue *rightInt;
    FloatValue *rightFloat;

    if (rightTok.isIdentifierTok())
    {
        rightValue = getValueFromSymbol(rightTok);
        rightInt = dynamic_cast<IntegerValue *>(rightValue.get());
        rightFloat = dynamic_cast<FloatValue *>(rightValue.get());
    }
    else if (rightTok.isNumberTok())
    {
        rightInt = dynamic_cast<IntegerValue *>(rightTok.getValue());
        rightFloat = dynamic_cast<FloatValue *>(rightTok.getValue());
    }
    else
    {
        throw semantic_error(
            format_string("expression \"%s\" invalid as binary operand", rightTok.getValue()->toString().c_str()));
    }

    // Perform type promotion. An int and a float in an operation together results in a float.
    bool needToDeleteLeft = false;
    bool needToDeleteRight = false;
    if (leftInt && rightFloat)
    {
        leftFloat = new FloatValue(double(leftInt->getValue()));
        leftInt = nullptr;
        needToDeleteLeft = true;
    }
    else if (leftFloat && rightInt)
    {
        rightFloat = new FloatValue(double(rightInt->getValue()));
        rightInt = nullptr;
        needToDeleteRight = true;
    }

    // Compute result of operation.
    AstNode *resultNode = nullptr;

    if (leftInt && rightInt)
    {
        uint64_t l = *leftInt;
        uint64_t r = *rightInt;
        Log::debug("    %llu %s %llu\n", l, tok.getTokenName(), r);

        uint64_t result = 0;
        switch (tok.getToken())
        {
            case '+':
                result = l + r;
                break;
            case '-':
                result = l - r;
                break;
            case '*':
                result = l * r;
                break;
            case '/':
                // Just make the result a zero if div-by-zero.
                if (r == 0)
                {
                    result = 0;
                }
                else
                {
                    result = l / r;
                }
                break;
            case '%':
                // Just make the result a zero if div-by-zero.
                if (r == 0)
                {
                    result = 0;
                }
                else
                {
                    result = l % r;
                }
                break;
            case '&':
                result = l & r;
                break;
            case '|':
                result = l | r;
                break;
            case '^':
                result = l ^ r;
                break;
            case TOK_LSHIFT:
                result = l << r;
                break;
            case TOK_RSHIFT:
                result = l >> r;
                break;
            default:
                throw internal_error("unknown binary operator");
        }

        IntegerValue::int_type_t intType;
        if (leftInt->getIntType() == IntegerValue::kUnsignedLong ||
            rightInt->getIntType() == IntegerValue::kUnsignedLong)
        {
            intType = IntegerValue::kUnsignedLong;
        }
        else if (leftInt->getIntType() == IntegerValue::kUnsigned || rightInt->getIntType() == IntegerValue::kUnsigned)
        {
            if (leftInt->getIntType() == IntegerValue::kSignedLong ||
                rightInt->getIntType() == IntegerValue::kSignedLong)
            {
                intType = IntegerValue::kUnsignedLong;
            }
            else
            {
                intType = IntegerValue::kUnsigned;
            }
        }
        else if (leftInt->getIntType() == IntegerValue::kSignedLong ||
                 rightInt->getIntType() == IntegerValue::kSignedLong)
        {
            intType = IntegerValue::kSignedLong;
        }
        else
        {
            intType = IntegerValue::kSigned;
        }

        resultNode = new AstNode(Token(TOK_INT_LITERAL, new IntegerValue(result, intType)));
    }
    else if (leftFloat && rightFloat)
    {
        double l = *leftFloat;
        double r = *rightFloat;

        if (needToDeleteLeft)
        {
            delete leftFloat; /* delete float values because they could be newly allocated during type promotion */
        }
        if (needToDeleteRight)
        {
            delete rightFloat;
        }

        Log::debug("    %g %s %g\n", l, tok.getTokenName(), r);
        double result = 0;
        switch (tok.getToken())
        {
            case '+':
                result = l + r;
                break;
            case '-':
                result = l - r;
                break;
            case '*':
                result = l * r;
                break;
            case '/':
                /* Just make the result a zero if div-by-zero. */
                if (r == 0)
                {
                    result = 0;
                }
                else
                {
                    result = l / r;
                }
                break;

            /* Throw semantic error on invalid operators for floats. */
            case '%':
            case '&':
            case '|':
            case '^':
            case TOK_LSHIFT:
            case TOK_RSHIFT:
                throw semantic_error(format_string("line %d: invalid operator on float value", tok.getFirstLine()));
            default:
                throw internal_error("unknown binary operator");
        }

        resultNode = new AstNode(Token(TOK_FLOAT_LITERAL, new FloatValue(result)));
    }

    return resultNode;
}

AstNode *SymbolScanner::handleUnaryOp(AstNode *node, bottom_up)
{
    Log::debug("unop: %s\n", node->getDescription().c_str());

    Token &tok = node->getToken();

    AstNode *valueNode = (*node)[0];
    assert(valueNode);
    Token &valueTok = valueNode->getToken();

    IntegerValue *valueInt;
    FloatValue *valueFloat;

    if (valueTok.isIdentifierTok())
    {
        smart_ptr<Value> unaryValue = getValueFromSymbol(valueTok);
        valueInt = dynamic_cast<IntegerValue *>(unaryValue.get());
        valueFloat = dynamic_cast<FloatValue *>(unaryValue.get());
    }
    else if (valueTok.isNumberTok())
    {
        valueInt = dynamic_cast<IntegerValue *>(valueTok.getValue());
        valueFloat = dynamic_cast<FloatValue *>(valueTok.getValue());
    }
    else
    {
        throw semantic_error(format_string("expression \"%s\"  invalid as binary operand on line %d",
                                           valueTok.getValue()->toString().c_str(),
                                           valueTok.getLocation().m_firstLine));
    }

    if (valueInt)
    {
        uint64_t value = *valueInt;
        switch (tok.getToken())
        {
            case TOK_UNARY_NEGATE:
                value = (-(int64_t)(value));
                break;
            case '~':
                value = ~value;
                break;
            default:
                throw internal_error("unknown unary operator");
        }

        return new AstNode(Token(TOK_INT_LITERAL, new IntegerValue(value, valueInt->getIntType())));
    }
    else if (valueFloat)
    {
        double value = *valueFloat;
        switch (tok.getToken())
        {
            case TOK_UNARY_NEGATE:
                value = -value;
                return new AstNode(Token(TOK_FLOAT_LITERAL, new FloatValue(value)));

            // It is a semantic error to use the binary invert operator on a float.
            case '~':
                throw semantic_error(format_string("line %d: invalid operator on float value", tok.getFirstLine()));

            default:
                throw internal_error("unknown unary operator");
        }
    }

    return nullptr;
}

AstNode *SymbolScanner::handleProgram(AstNode *node, top_down)
{
    static int numberOfPrograms = 0;

    numberOfPrograms++;
    if (1 < numberOfPrograms)
    {
        throw semantic_error("Cannot define more than one program per file.");
    }
    const Token &tok = node->getChild(0)->getToken();
    const string &name = tok.getStringValue();
    Log::debug("Program: %s\n", name.c_str());

    Program *prog = new Program(tok);
    m_currentProgram = prog;

    return nullptr;
}

AstNode *SymbolScanner::handleProgram(AstNode *node, bottom_up)
{
    addAnnotations(node->getChild(1), m_currentProgram);
    addDoxygenComments(m_currentProgram, node->getChild(2), nullptr);
    return nullptr;
}

AstNode *SymbolScanner::handleStruct(AstNode *node, top_down)
{
    AstNode *structNameNode = (*node)[0];
    if (!structNameNode && m_currentAlias == nullptr)
    {
        throw semantic_error(
            format_string("line %d: illegal anonymous struct definition at file level", node->getToken().getFirstLine())
                .c_str());
    }

    // Create the struct symbol.
    StructType *newStruct;
    if (structNameNode)
    {
        const Token &tok = structNameNode->getToken();
        string name = tok.getStringValue();
        Log::debug("struct: %s\n", name.c_str());

        /* match forward declaration with definition */
        auto forwardDecl = m_forwardDeclarations.find(name);
        if (forwardDecl != m_forwardDeclarations.end())
        {
            newStruct = dynamic_cast<StructType *>(forwardDecl->second);
            if (newStruct != nullptr)
            {
                new (newStruct) StructType(tok);
            }
            else
            {
                throw syntax_error(format_string("line %d: Structure definition type name didn't match data type of "
                                                 "forward declaration from line %d.",
                                                 tok.getFirstLine(), forwardDecl->second->getFirstLine())
                                       .c_str());
            }
        }
        else
        {
            newStruct = new StructType(tok);
        }
    }
    else
    {
        Log::debug(
            "typedef struct: %s\n",
            check_null(check_null(check_null(node->getParent())->getChild(0))->getTokenValue())->toString().c_str());
        newStruct = new StructType("");
    }

    m_currentStruct = newStruct;

    return nullptr;
}

AstNode *SymbolScanner::handleStruct(AstNode *node, bottom_up)
{
    m_currentStruct->getScope().setParent(m_globals);

    if (m_currentAlias != nullptr)
    {
        if (m_currentAlias->getElementType() == nullptr)
        {
            m_currentAlias->setElementType(m_currentStruct);
        }
        else
        {
            throw internal_error("Alias type should not have assigned data type at this state.");
        }
    }
    else
    {
        // Forward declaration have 4 nodes.
        if (node->childCount() > 4)
        {
            // Get comment if exist.
            addDoxygenComments(m_currentStruct, node->getChild(3), node->getChild(4));

            addAnnotations(node->getChild(2), m_currentStruct);
        }
    }

    // Forward declaration have 4 nodes.
    if (node->childCount() == 4)
    {
        addForwardDeclaration(m_currentStruct);
    }
    else
    {
        removeForwardDeclaration(m_currentStruct);

        addGlobalSymbol(m_currentStruct);

        // Handle annotations for function params
        scanStructForAnnotations();
    }

    /* Clear current struct pointer. */
    m_currentStruct = nullptr;

    return nullptr;
}

AstNode *SymbolScanner::handleStructMember(AstNode *node, bottom_up)
{
    assert_throw_internal((m_currentStruct), "struct member not in struct");

    AstNode *ident = (*node)[0];
    const Token &tok = ident->getToken();
    const string &name = tok.getStringValue();
    Log::debug("struct member: %s\n", name.c_str());

    /* Extract member type. */
    AstNode *typeNode = (*node)[1];
    DataType *dataType = lookupDataType(typeNode);
    if (dataType->getName() == m_currentStruct->getName())
    {
        throw semantic_error(
            format_string("line %d: invalid data type. Structure member has same data type name as structure name.",
                          tok.getFirstLine()));
    }

    /* Create struct member object. */
    StructMember *param = new StructMember(tok, dataType);

    // Set byref flag for structure member.
    AstNode *memberType = node->getChild(2);
    param->setByref(memberType->getTokenString().compare("2") == 0);

    addAnnotations(node->getChild(3), param);

    /* doxygen comment */
    addDoxygenComments(param, node->getChild(4), node->getChild(5));

    m_currentStruct->addMember(param);

    return nullptr;
}

AstNode *SymbolScanner::handleUnion(AstNode *node, top_down)
{
    Token *tok;
    AstNode *astUnionName = node->getChild(0);
    AstNode *astUnionDiscriminator = node->getChild(1);

    /* get union type name*/
    if (!astUnionName)
    {
        /* Get the name of the union variable so we can manipulate it and add it to the AST
           under the union token */
        string unionVariableName = node->getParent()->getChild(0)->getToken().getStringValue();

        /* Create a new node in the AST for the union's name, and assign it */
        node->appendChild(new AstNode(Token(TOK_IDENT, new StringValue(unionVariableName + "_union"))));

        /* union token. */
        tok = &node->getChild(3)->getToken();
    }
    else
    {
        /* union token for non-encapsulated discriminated unions. */
        tok = &astUnionName->getToken();
    };

    Log::debug("union: %s\n", tok->getStringValue().c_str());

    UnionType *newUnion;
    /* get union type object */
    if (astUnionDiscriminator)
    {
        /* discriminated unions. */
        const string &discriminatorName = astUnionDiscriminator->getToken().getStringValue();
        newUnion = new UnionType(*tok, discriminatorName);
    }
    else
    {
        /* match forward declaration with definition */
        auto forwardDecl = m_forwardDeclarations.find(tok->getStringValue());
        if (forwardDecl != m_forwardDeclarations.end())
        {
            newUnion = dynamic_cast<UnionType *>(forwardDecl->second);
            if (newUnion != nullptr)
            {
                new (newUnion) UnionType(*tok, "");
            }
            else
            {
                throw syntax_error(format_string("line %d: Union definition type name didn't match data type of "
                                                 "forward declaration from line %d.",
                                                 tok->getFirstLine(), forwardDecl->second->getFirstLine())
                                       .c_str());
            }
        }
        else
        {
            /* non-encapsulated discriminated unions. */
            newUnion = new UnionType(*tok, "");
        }
    }

    m_currentUnion = newUnion;

    return nullptr;
}

AstNode *SymbolScanner::handleUnion(AstNode *node, bottom_up)
{
    /* add union to global symbol list */
    if (m_currentUnion->isNonEncapsulatedUnion())
    {
        // Forward declaration have 5 nodes.
        if (node->childCount() == 5)
        {
            addForwardDeclaration(m_currentUnion);
        }
        else
        {
            addAnnotations(node->getChild(3), m_currentUnion);

            /* doxygen comment */
            addDoxygenComments(m_currentUnion, node->getChild(4), node->getChild(5));

            removeForwardDeclaration(m_currentUnion);

            addGlobalSymbol(m_currentUnion);
        }
    }
    else /* add union to structure */
    {
        assert(nullptr != m_currentStruct);
        m_currentStruct->getScope().addSymbol(m_currentUnion);
        m_currentUnion->setParentStruct(m_currentStruct);
    }
    m_currentUnion = nullptr;
    return nullptr;
}

AstNode *SymbolScanner::handleUnionCase(AstNode *node, top_down)
{
    (void)node;
    return nullptr;
}

/* TOK_UNION_CASE -> ( (TOK_EXPR -> (indent | scalar)) indent(datatype) indent(varName) ) */
AstNode *SymbolScanner::handleUnionCase(AstNode *node, bottom_up)
{
    uint32_t caseIdIntValue;
    UnionCase *newCase;
    /* get case name and value */
    if (TOK_DEFAULT == node->getChild(0)->getToken().getToken())
    {
        string caseIdName = node->getChild(0)->getToken().getTokenName();

        Log::debug("union case id name: %s\n", caseIdName.c_str());

        /* remove quotation marks on caseIdName token */
        newCase = new UnionCase(caseIdName.substr(1, caseIdName.length() - 2));

        /* Now that we've created the new case, add it to the union it is inside */
        m_currentUnion->addCase(newCase);
        /* Add the new case to the empty case list. We will immediately check if there is
           a member declaration and, if so, immediately pop the new case of the empty list */
        m_emptyUnionCases.push_back(newCase);
    }
    else /* TOK_CHILDREN, handle case expression list */
    {
        AstNode *childrenNode = node->getChild(0);
        assert(TOK_CHILDREN == childrenNode->getToken().getToken());
        for (auto childCase = childrenNode->begin(); childCase != childrenNode->end(); ++childCase)
        {
            assert(TOK_EXPR == (*childCase)->getToken().getToken());
            Token &caseIdTok = (*childCase)->getChild(0)->getToken();
            /* If the case value is an identifier, record the name and find the true value */
            if (caseIdTok.isIdentifierTok())
            {
                Value *caseIdValue = getValueFromSymbol(caseIdTok);
                if (kIntegerValue != caseIdValue->getType())
                {
                    delete caseIdValue;
                    throw semantic_error(
                        format_string("line %d: Value for union case must be an int\n", caseIdTok.getFirstLine()));
                }

                caseIdIntValue = (uint32_t) dynamic_cast<IntegerValue *>(caseIdValue)->getValue();
                const string caseIdName = caseIdTok.getStringValue();
                Log::debug("union case id name: %s\n", caseIdName.c_str());
                newCase = new UnionCase(caseIdName, caseIdIntValue);
                delete caseIdValue;
            }
            /* If there is no identifier, simply record the case value */
            else if (caseIdTok.isNumberTok())
            {
                if (kIntegerValue != caseIdTok.getValue()->getType())
                {
                    throw semantic_error(
                        format_string("line %d: Value for union case must be an int\n", caseIdTok.getFirstLine()));
                }
                caseIdIntValue = (uint32_t) dynamic_cast<IntegerValue *>(caseIdTok.getValue())->getValue();
                Log::debug("union case id name: default\n");
                newCase = new UnionCase(caseIdIntValue);
            }
            else
            {
                throw semantic_error(
                    format_string("line %d: Invalid token for case value in union\n", caseIdTok.getFirstLine()));
            }
            // Now that we've created the new case, add it to the union it is inside
            m_currentUnion->addCase(newCase);
            // Add the new case to the empty case list. We will immediately check if there is
            // a member declaration and, if so, immediately pop the new case of the empty list
            m_emptyUnionCases.push_back(newCase);
        }
    }

    // Now we add the member declaration to the union
    // This entails adding the declaration to the union containing this case
    // and then add a name reference to the current UnionCase object
    if (nullptr != node->getChild(1))
    {
        // Get TOK_CHILDREN node
        AstNode *unionDeclList = node->getChild(1);
        assert(TOK_CHILDREN == unionDeclList->getToken().getToken());
        vector<string> declNames;
        if (TOK_VOID == unionDeclList->getChild(0)->getToken().getToken())
        {
            declNames.push_back("void");
            Log::debug("union case member name: void\n");
        }
        else
        {
            // If not void, handle possible list of declarations
            for (auto unionDecl : *unionDeclList)
            {
                assert(TOK_CHILDREN == unionDecl->getToken().getToken());

                string name = unionDecl->getChild(0)->getTokenValue()->toString();
                Log::debug("union case member name: %s\n", name.c_str());
                DataType *declType = lookupDataType(unionDecl->getChild(1));
                m_currentUnion->addUnionMemberDeclaration(name, declType);

                StructMember *unionMember = m_currentUnion->getUnionMembers().getMembers().back();
                addAnnotations(unionDecl->getChild(3), unionMember);
                addDoxygenComments(unionMember, unionDecl->getChild(4), unionDecl->getChild(5));

                declNames.push_back(name);
            }
        }
        // We found a case with a member declaration. Add it to this member and any members
        // in m_emptyUnionCases
        for (auto unionCase : m_emptyUnionCases)
        {
            for (auto declName : declNames)
            {
                unionCase->addCaseMemberName(declName);
            }
        }
        m_emptyUnionCases.clear();
    }
    return nullptr;
}

AstNode *SymbolScanner::handleInterface(AstNode *node, top_down)
{
    AstNode *ident = (*node)[0];
    const Token &tok = ident->getToken();
    const string &name = tok.getStringValue();
    Log::debug("interface: %s\n", name.c_str());

    Interface *iface = new Interface(tok);
    iface->getScope().setParent(m_globals);
    m_currentInterface = iface;
    addGlobalSymbol(iface);

    // Get comment if exist.
    addDoxygenComments(iface, node->getChild(3), node->getChild(4));

    return nullptr;
}

AstNode *SymbolScanner::handleInterface(AstNode *node, bottom_up)
{
    addAnnotations(node->getChild(2), m_currentInterface);

    // Interfaces cannot be nested, so we can just clear this. If they were nestable, we would
    // have to keep a stack of open interfaces.
    m_currentInterface = nullptr;

    return nullptr;
}

AstNode *SymbolScanner::handleFunction(AstNode *node, top_down)
{
    // Get function name.
    AstNode *ident = (*node)[0];
    const Token &tok = ident->getToken();
    const string &name = tok.getStringValue();
    Log::debug("function: %s\n", name.c_str());

    // Create function symbol.
    FunctionBase *func;
    if (m_currentInterface) /* function definition */
    {
        if (m_currentInterface->getFunctions().size() == 0)
        {
            func = new Function(tok, m_currentInterface, 1);
        }
        else
        {
            func = new Function(tok, m_currentInterface);
        }
        m_currentInterface->addFunction(dynamic_cast<Function *>(func));
    }
    else /* function type */
    {
        func = new FunctionType(tok);
        addGlobalSymbol(dynamic_cast<FunctionType *>(func));
    }

    m_currentStruct = &(func->getParameters());

    // Get return type.
    AstNode *returnNode = (*node)[1];
    if (returnNode) /* Function type/definition */
    {
        Token &returnTokenType = returnNode->getToken();
        if (returnTokenType.getToken() == TOK_RETURN)
        {
            AstNode *returnTypeNode = returnNode->getChild(0);
            Token &returnTypeToken = returnTypeNode->getToken();
            switch (returnTypeToken.getToken())
            {
                case TOK_ONEWAY:
                    func->setIsOneway(true);
                    func->setReturnStructMemberType(new StructMember("(return)", new VoidType));
                    break;

                case TOK_VOID:
                    func->setReturnStructMemberType(new StructMember("(return)", new VoidType));
                    break;

                default:
                    DataType *dataType = lookupDataType(returnTypeNode);
                    func->setReturnStructMemberType(new StructMember("(return)", dataType));
                    break;
            }
            if (returnTypeToken.getToken() != TOK_ONEWAY)
            {
                addAnnotations(returnNode->getChild(1), func->getReturnStructMemberType());
            }
        }
        else
        {
            throw semantic_error(
                format_string("line %d: Unexpected token type. Expected TOK_RETURN\n", returnTokenType.getFirstLine()));
        }
    }
    else
    {
        /* Function is callback */
        AstNode *callbackTypeNode = (*node)[2];
        assert(callbackTypeNode);
        DataType *callbackDataType = lookupDataType(callbackTypeNode)->getTrueDataType();

        if (!callbackDataType->isFunction())
        {
            throw semantic_error(
                format_string("line %d: Expected function (callback) type.", callbackDataType->getFirstLine()));
        }
        FunctionType *callbackFunctionType = dynamic_cast<FunctionType *>(callbackDataType);
        assert(callbackFunctionType);
        Function *funcDef = dynamic_cast<Function *>(func);
        assert(funcDef);

        /* Connect callback type with callbacks. */
        callbackFunctionType->getCallbackFuns().push_back(funcDef);

        /* Connect callback with callback type. */
        funcDef->setFunctionType(callbackFunctionType);

        /* Add function oneway information. */
        funcDef->setIsOneway(callbackFunctionType->isOneway());

        /* Add function return type*/
        funcDef->setReturnStructMemberType(callbackFunctionType->getReturnStructMemberType());

        /* Add functions annotations. */
        /*for (const Annotation &ann : callbackFunctionType->getAnnotations())
        {
            funcDef->addAnnotation(ann);
        }*/
    }

    /* Get comment if exist. */
    addDoxygenComments(dynamic_cast<Symbol *>(func), node->getChild(5), node->getChild(6));

    return nullptr;
}

AstNode *SymbolScanner::handleFunction(AstNode *node, bottom_up)
{

    if (m_currentInterface) /* function definition */
    {
        Function *func = m_currentInterface->getFunctions().back();
        func->getParameters().getScope().setParent(&m_currentInterface->getScope());

        /* Function annotations. */
        addAnnotations(node->getChild(4), func);

        /* Add missing callbacks parameters. */
        FunctionType *callbackFunctionType = func->getFunctionType();
        if (callbackFunctionType)
        {
            uint32_t paramsSize = func->getParameters().getMembers().size();
            const StructType::member_vector_t &callbackParams = callbackFunctionType->getParameters().getMembers();
            if (callbackFunctionType->getParameters().getMembers().size() > paramsSize)
            {
                for (unsigned int i = paramsSize; i < callbackParams.size(); ++i)
                {
                    if (callbackParams[i]->getName().compare("") == 0)
                    {
                        throw semantic_error(
                            format_string("Missing function param name. That has to be defined in function definition "
                                          "%d or function type definition %d.\n",
                                          func->getFirstLine(), callbackFunctionType->getFirstLine()));
                    }
                    func->getParameters().addMember(
                        createCallbackParam(callbackParams[i], callbackParams[i]->getName()));
                }
            }
        }
    }
    else /* function type */
    {
        FunctionType *func = nullptr;
        for (Symbol *funSymbol : m_globals->getSymbolsOfType(Symbol::kTypenameSymbol))
        {
            DataType *datatype = dynamic_cast<DataType *>(funSymbol);
            assert(datatype);

            if (datatype->isFunction())
            {
                func = dynamic_cast<FunctionType *>(datatype);
            }
        }
        assert(func);
        func->getParameters().getScope().setParent(m_globals);

        /* Function annotations. */
        addAnnotations(node->getChild(4), func);
    }

    // Handle annotations for function params
    scanStructForAnnotations();

    m_currentStruct = nullptr;

    return nullptr;
}

AstNode *SymbolScanner::handleParam(AstNode *node, top_down)
{
    assert_throw_internal((m_currentStruct), "parameter not in function");

    /* Is this param for callback function? */
    StructMember *callbackParam = nullptr;
    Function *fun = nullptr;
    FunctionType *funType = nullptr;
    if (m_currentInterface)
    {
        fun = m_currentInterface->getFunctions().back();
        for (Symbol *funSymbol : m_globals->getSymbolsOfType(Symbol::kTypenameSymbol))
        {
            DataType *datatype = dynamic_cast<DataType *>(funSymbol);
            assert(datatype);

            if (datatype->isFunction())
            {
                funType = dynamic_cast<FunctionType *>(datatype);
                assert(funType);
                FunctionType::c_function_list_t &callbacks = funType->getCallbackFuns();
                if (find(callbacks.begin(), callbacks.end(), fun) != callbacks.end())
                {
                    if (fun->getParameters().getMembers().size() > funType->getParameters().getMembers().size())
                    {
                        throw syntax_error(format_string("line %d: Function definition contains more parameters than "
                                                         "function type definition from %d.\n",
                                                         fun->getFirstLine(), funType->getFirstLine())
                                               .c_str());
                    }
                    else
                    {
                        callbackParam = funType->getParameters().getMembers()[fun->getParameters().getMembers().size()];
                    }
                    break;
                }
            }
        }
    }

    AstNode *ident = (*node)[0];
    StructMember *param;
    if (callbackParam) /* Callback parameters. */
    {
        string name;
        if (ident) /* Get name from callback definition. */
        {
            const Token &tok = ident->getToken();
            name = tok.getStringValue();
        }
        else /* Get name from function type definition. */
        {
            name = callbackParam->getName();
        }
        if (name.compare("") == 0)
        {
            assert(funType);
            assert(fun);
            throw semantic_error(
                format_string("Missing function param name. That has to be defined in function definition %d or "
                              "function type definition %d.\n",
                              fun->getFirstLine(), funType->getFirstLine()));
        }
        param = createCallbackParam(callbackParam, name);
    }
    else /* Function or Function type parameters. */
    {
        /* Extract param data type. */
        AstNode *typeNode = (*node)[1];
        assert(typeNode);
        DataType *dataType = lookupDataType(typeNode);

        /* Extract param name. */
        if (ident)
        {
            Token &tok = ident->getToken();
            param = new StructMember(tok, dataType);
        }
        else if (m_currentInterface && !funType) // Functions need param names. Types of functions don't.
        {
            throw syntax_error(
                format_string("line %d: Missing function param name.\n", node->getToken().getFirstLine()));
        }
        else /* Function type don't need param name. */
        {
            param = new StructMember("", dataType);
        }
        setParameterDirection(param, (*node)[2]);
    }

    Log::debug("param: %s\n", param->getName().c_str());

    /* TOK_PARAM -> name datatype annotations */
    m_currentStruct->addMember(param);

    return nullptr;
}

AstNode *SymbolScanner::handleParam(AstNode *node, bottom_up)
{
    StructMember *param = m_currentStruct->getMembers().back();
    addAnnotations(node->getChild(3), param);
    return nullptr;
}

void SymbolScanner::setParameterDirection(StructMember *param, AstNode *directionNode)
{
    /* Extract parameter direction: in/out/inout/out byref. */
    _param_direction param_direction;
    if (nullptr != directionNode)
    {
        switch (directionNode->getToken().getToken())
        {
            case TOK_IN:
                param_direction = kInDirection;
                break;
            case TOK_OUT:
                param_direction = kOutDirection;
                break;
            case TOK_INOUT:
                param_direction = kInoutDirection;
                break;
            default:
                delete param;
                throw semantic_error(format_string("line %d: expected parameter direction type",
                                                   directionNode->getToken().getFirstLine()));
                break;
        }
    }
    else /* if no direction specified, default case is an 'in' variable */
    {
        param_direction = kInDirection;
    }
    param->setDirection(param_direction);
}

AstNode *SymbolScanner::handleExpr(AstNode *node, bottom_up)
{
    (void)node;
    /* Log::debug("expr: %s\n", node->getDescription().c_str()); */
    return nullptr;
}

DataType *SymbolScanner::lookupDataTypeByName(const Token &tok, SymbolScope *scope, bool recursive)
{
    const string typeName = tok.getStringValue();
    Symbol *dataTypeSym = scope->getSymbol(typeName, recursive);
    if (!dataTypeSym)
    {
        auto forwardDecl = m_forwardDeclarations.find(typeName);
        if (forwardDecl != m_forwardDeclarations.end())
        {
            dataTypeSym = forwardDecl->second;
        }
        else
        {
            throw semantic_error(format_string("line %d: undefined name '%s'", tok.getFirstLine(), typeName.c_str()));
        }
    }
    DataType *dataType = dynamic_cast<DataType *>(dataTypeSym);
    if (!dataType)
    {
        throw semantic_error(format_string("line %d: '%s' is not a type", tok.getFirstLine(), typeName.c_str()));
    }
    return dataType;
}

bool SymbolScanner::containsStructEnumDeclaration(const AstNode *typeNode)
{
    const Token &typeToken = typeNode->getToken();
    return (typeToken.getToken() == TOK_ENUM || typeToken.getToken() == TOK_STRUCT);
}

DataType *SymbolScanner::lookupDataType(const AstNode *typeNode)
{
    const Token &typeToken = typeNode->getToken();
    switch (typeToken.getToken())
    {
        case TOK_ARRAY:
            return createArrayType(typeNode);

        case TOK_IDENT:
            return lookupDataTypeByName(typeToken, m_globals);

        case TOK_LIST:
            return createListType(typeNode);

        case TOK_UNION:
        {
            assert(nullptr != m_currentStruct);
            return lookupDataTypeByName(typeNode->getChild(3)->getToken(), &(m_currentStruct->getScope()), false);
        }
        default:
            throw internal_error(format_string("unexpected token type %s on line %d", typeToken.getTokenName(),
                                               typeToken.getLocation().m_firstLine));
    }
}

DataType *SymbolScanner::createListType(const AstNode *typeNode)
{
    /* The only child node of a list node is the element type. */
    const AstNode *elementTypeNode = (*typeNode)[0];
    DataType *elementType = lookupDataType(elementTypeNode);

    /* Create list type. */
    ListType *list = new ListType(elementType);

    return list;
}

DataType *SymbolScanner::createArrayType(const AstNode *typeNode)
{
    /* The two children of an array type node are the element type and element count. */
    const AstNode *elementTypeNode = (*typeNode)[0];
    DataType *elementType = lookupDataType(elementTypeNode);

    const AstNode *elementCountNode = (*typeNode)[1];
    uint64_t elementCount = getIntExprValue(elementCountNode);

    /* Create list type. */
    ArrayType *array = new ArrayType(elementType, (uint32_t)elementCount);

    return array;
}

Value *SymbolScanner::getValueFromSymbol(Token &tok)
{
    if (tok.getValue() != nullptr)
    {
        string name = tok.getStringValue();
        Symbol *sym = m_globals->getSymbol(name);
        if (nullptr != sym)
        {
            Value *newVal;
            if (sym->isConstSymbol())
            {
                ConstType *constVar = dynamic_cast<ConstType *>(sym);
                assert(constVar);
                newVal = constVar->getValue()->clone();
            }
            else if (sym->isEnumMemberSymbol())
            {
                EnumMember *enumVar = dynamic_cast<EnumMember *>(sym);
                assert(enumVar);
                newVal = new IntegerValue(enumVar->getValue());
            }
            else
            {
                throw semantic_error(format_string("line %d: Cannot get value from symbol type: %s.\n",
                                                   tok.getFirstLine(), sym->getDescription().c_str()));
            }

            return newVal;
        }

        throw syntax_error(format_string("line %d: Symbol %s is not defined.\n", tok.getFirstLine(), name.c_str()));
    }

    throw syntax_error(format_string("line %d: cannot get token value.\n", tok.getFirstLine()));
}

uint64_t SymbolScanner::getIntExprValue(const AstNode *exprNode)
{
    /* Check that this node is really an expr. */
    const Token &tok = exprNode->getToken();
    assert_throw_internal((tok.getToken() == TOK_EXPR), "node expected to be an expr is not");

    /* Get the expr's value child node. */
    AstNode *valueNode = const_cast<AstNode *>(exprNode)->getChild(0);
    Token &tok2 = valueNode->getToken();

    /* If token is not an identifier, it must be either a binary expression
       that simplifies to an int literal or an int literal */
    if (tok2.isIdentifierTok())
    {
        Value *val = getValueFromSymbol(tok2);
        if (val == nullptr || kIntegerValue != val->getType())
        {
            if (val != nullptr)
            {
                delete val;
            }
            throw semantic_error(format_string("line %d: expected integer expression", tok2.getFirstLine()));
        }
        assert(nullptr != dynamic_cast<IntegerValue *>(val));
        uint64_t result = dynamic_cast<IntegerValue *>(val)->getValue();
        delete val;
        return result;
    }
    /* If it is a binary expression, simplify expression and get the value from
       the re-written token. */
    else if (tok2.isBinaryOp())
    {
        walk(valueNode);
    }
    Token &valueToken = const_cast<AstNode *>(exprNode)->getChild(0)->getToken();
    if (valueToken.getToken() != TOK_INT_LITERAL)
    {
        throw semantic_error(format_string("line %d: expected integer expression", tok2.getFirstLine()));
    }

    return valueToken.getIntValue();
}

void SymbolScanner::addAnnotations(AstNode *childNode, Symbol *symbol)
{
    if (childNode)
    {
        for (auto annotation : *childNode)
        {
            Log::SetOutputLevel logLevel(Logger::kDebug);

            // name can be optional for struct/enum
            string nameOfType;
            if (childNode->getParent()->getChild(0))
            {
                nameOfType = childNode->getParent()->getChild(0)->getToken().getStringValue();
                Log::log("Handling annotations for %s\n", nameOfType.c_str());
            }
            else
            {
                Log::log("Handling annotations\n");
            }

            Annotation::program_lang_t annLang = getAnnotationLang(annotation);

            // TOK_ANNOTATION -> ( (name) (TOK_EXPR -> (value)) )
            AstNode *annotation_name = annotation->getChild(1);

            checkAnnotationBeforeAdding(annotation, symbol);

            const Token &nameTok = annotation_name->getToken();
            Value *annValue = getAnnotationValue(annotation);

            Annotation ann(nameTok, annValue, annLang);

            symbol->addAnnotation(ann);

            // name can be optional for struct/enum
            if (childNode->getParent()->getChild(0))
            {
                Log::log("\tAdding annotation: @%s() to %s\n", ann.getName().c_str(), nameOfType.c_str());
            }
            else
            {
                Log::log("\tAdding annotation: @%s()\n", ann.getName().c_str());
            }
        }
    }
}

Value *SymbolScanner::getAnnotationValue(AstNode *annotation)
{
    if (AstNode *annotation_value = annotation->getChild(2))
    {
        // Strip TOK_EXPR token
        if (0 == strcmp("TOK_EXPR", annotation_value->getToken().getTokenName()))
        {
            return annotation_value->getChild(0)->getToken().getValue();
        }

        return annotation_value->getToken().getValue();
    }

    return nullptr;
}

Annotation::program_lang_t SymbolScanner::getAnnotationLang(AstNode *annotation)
{
    if (AstNode *annotation_value = annotation->getChild(0))
    {

        string lang = annotation_value->getToken().getValue()->toString();
        if (lang.compare("c") == 0)
        {
            return Annotation::kC;
        }
        else if (lang.compare("py") == 0)
        {
            return Annotation::kPython;
        }

        throw semantic_error(format_string("line %d: Unsupported programming language '%s' specified.",
                                           annotation->getToken().getFirstLine(), lang.c_str())
                                 .c_str());
    }

    return Annotation::kAll;
}

void SymbolScanner::checkAnnotationBeforeAdding(AstNode *annotation, Symbol *symbol)
{
    AstNode *annotation_name = annotation->getChild(1);
    Value *annValue = getAnnotationValue(annotation);

    if (annotation_name->getTokenString().compare(LENGTH_ANNOTATION) == 0)
    {
        StructMember *structMember = dynamic_cast<StructMember *>(symbol);
        if (structMember)
        {
            DataType *trueDataType = structMember->getDataType()->getTrueDataType();
            if (trueDataType && (!trueDataType->isList() && !trueDataType->isBinary()))
            {
                throw semantic_error(
                    format_string("line %d: Length annotation can only be applied to list or binary types",
                                  annotation_name->getToken().getFirstLine()));
            }

            // Check @length annotation's value.
            if (!annValue)
            {
                throw semantic_error(format_string("line %d: Length annotation must name a valid parameter or member",
                                                   annotation_name->getToken().getFirstLine()));
            }
        }
    }
    else if (annotation_name->getTokenString().compare(MAX_LENGTH_ANNOTATION) == 0)
    {
        StructMember *structMember = dynamic_cast<StructMember *>(symbol);
        if (structMember)
        {
            DataType *trueDataType = structMember->getDataType()->getTrueDataType();
            if (trueDataType && (!trueDataType->isList() && !trueDataType->isBinary() && !trueDataType->isString()))
            {
                throw semantic_error(
                    format_string("line %d: Max_length annotation can only be applied to list, binary, or string types",
                                  annotation_name->getToken().getFirstLine()));
            }

            // Check @length annotation's value.
            if (!annValue)
            {
                throw semantic_error(
                    format_string("line %d: Max_length annotation must name a valid parameter or member",
                                  annotation_name->getToken().getFirstLine()));
            }
        }
    }
}

void SymbolScanner::scanStructForAnnotations()
{
    assert(m_currentStruct);

    // go trough all structure members
    for (StructMember *structMember : m_currentStruct->getMembers())
    {
        DataType *memberType = structMember->getDataType()->getTrueDataType();
        /* Check non-encapsulated discriminated unions. */
        if (memberType->isUnion())
        {
            UnionType *unionType = dynamic_cast<UnionType *>(structMember->getDataType());
            assert(unionType);
            Symbol *disSymbol;
            if (unionType->isNonEncapsulatedUnion())
            {
                string discrimintorName = structMember->getAnnStringValue(DISCRIMINATOR_ANNOTATION, Annotation::kAll);
                if (discrimintorName.empty())
                {
                    throw syntax_error(format_string("Missing discriminator for union variable %s on line %d",
                                                     structMember->getName().c_str(), structMember->getFirstLine()));
                }

                // search in structure scope for member referenced with annotation
                disSymbol = m_currentStruct->getScope().getSymbol(discrimintorName, false);
                if (!disSymbol)
                {
                    disSymbol = m_globals->getSymbol(discrimintorName, false);
                }
                if (!disSymbol)
                {
                    throw syntax_error(
                        format_string("Value defined in annotation discriminator used for union "
                                      "variable %s on line %d has to point to variable in same/global scope.",
                                      structMember->getName().c_str(), structMember->getFirstLine()));
                }
            }
            else
            {
                disSymbol = m_currentStruct->getScope().getSymbol(unionType->getDiscriminatorName(), false);
                if (!disSymbol)
                {
                    throw syntax_error(
                        format_string("Discriminator used for union variable %s on line %d has to point "
                                      "to variable in same scope.",
                                      structMember->getName().c_str(), structMember->getFirstLine()));
                }
            }

            // check discriminator data type
            DataType *disType;
            if (StructMember *disStructMember = dynamic_cast<StructMember *>(disSymbol))
            {
                disType = disStructMember->getDataType()->getTrueDataType();
            }
            else
            {
                ConstType *disConstMember = dynamic_cast<ConstType *>(disSymbol);
                assert(disConstMember);
                disType = disConstMember->getDataType()->getTrueDataType();
            }

            assert(disType);
            if (!disType->isScalar() && !disType->isEnum())
            {
                throw syntax_error(
                    format_string("Discriminator used for union variable %s on line %d has to be "
                                  "enum, boolean, int or uint type.",
                                  structMember->getName().c_str(), structMember->getFirstLine()));
            }
        }
    }
}

void SymbolScanner::addDoxygenComments(Symbol *symbol, AstNode *above, AstNode *trailing)
{
    assert(symbol);

    // Get doxygen comments if exist.
    if (above)
    {
        symbol->setMlComment(above->getTokenString());
    }
    if (trailing)
    {
        symbol->setIlComment(trailing->getTokenString());
    }
}

StructMember *SymbolScanner::createCallbackParam(StructMember *structMember, const string &name)
{
    /* struct member name */
    string memberName;
    if (name.compare("") == 0)
    {
        memberName = structMember->getName();
    }
    else
    {
        memberName = name;
    }

    /* struct member */
    StructMember *param = new StructMember(memberName, structMember->getDataType());
    for (const Annotation &memberAnn : structMember->getAnnotations())
    {
        param->addAnnotation(memberAnn);
    }
    /* Set parameter specific data. */
    param->setContainList(structMember->getContainList());
    param->setContainString(structMember->getContainString());
    param->setDirection(structMember->getDirection());
    return param;
}

void SymbolScanner::addForwardDeclaration(DataType *dataType)
{
    if (Symbol *symbol = m_globals->getSymbol(dataType->getName()))
    {
        throw semantic_error(format_string("line %d: Declaring type '%s' already declared here '%d'",
                                           dataType->getFirstLine(), dataType->getName().c_str(),
                                           symbol->getFirstLine())
                                 .c_str());
    }

    auto findDataTypeIT = m_forwardDeclarations.find(dataType->getName());
    if (findDataTypeIT != m_forwardDeclarations.end())
    {
        if (findDataTypeIT->second->getDataType() != dataType->getDataType())
        {
            throw semantic_error(format_string("line %d: Declaring type '%s' already declared here '%d'",
                                               dataType->getFirstLine(), dataType->getName().c_str(),
                                               findDataTypeIT->second->getFirstLine())
                                     .c_str());
        }
        else
        {
            return;
        }
    }
    m_forwardDeclarations[dataType->getName()] = dataType;
}

void SymbolScanner::removeForwardDeclaration(DataType *dataType)
{
    auto findDataTypeIT = m_forwardDeclarations.find(dataType->getName());
    if (findDataTypeIT != m_forwardDeclarations.end())
    {
        if (findDataTypeIT->second->getDataType() != dataType->getDataType())
        {
            throw semantic_error(format_string("line %d: Declaring type '%s' already declared here '%d'",
                                               dataType->getFirstLine(), dataType->getName().c_str(),
                                               findDataTypeIT->second->getFirstLine())
                                     .c_str());
        }
        m_forwardDeclarations.erase(findDataTypeIT);
    }
}

void SymbolScanner::addGlobalSymbol(Symbol *symbol)
{
    auto findDataTypeIT = m_forwardDeclarations.find(symbol->getName());
    if (findDataTypeIT != m_forwardDeclarations.end())
    {
        throw semantic_error(format_string("line %d: Declaring symbol '%s' already declared here '%d'",
                                           symbol->getFirstLine(), symbol->getName().c_str(),
                                           findDataTypeIT->second->getFirstLine())
                                 .c_str());
    }
    m_globals->addSymbol(symbol);
}
