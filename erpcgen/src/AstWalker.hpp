/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__ASTWALKER_H_
#define _EMBEDDED_RPC__ASTWALKER_H_

#include "AstNode.hpp"

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpcgen {

/*!
 * @brief Performs a depth-first walk of an AST.
 *
 * Handles these token types:
 * - TOK_PROGRAM
 * - TOK_CONST
 * - TOK_ENUM
 * - TOK_STRUCT
 * - TOK_TYPE
 * - TOK_INTERFACE
 * - TOK_CHILDREN
 * - TOK_ENUM_MEMBER
 * - TOK_STRUCT_MEMBER
 * - TOK_FUNCTION
 * - TOK_PARAM
 * - TOK_EXPR
 * - TOK_ANNOTATION
 * - TOK_LIST
 * - TOK_ARRAY
 */
class AstWalker
{
public:
    /*!
     * @brief default constructor
     */
    AstWalker() = default;

    /*!
     * @brief Constructor.
     *
     * This function initializes object attributes.
     *
     * @param[in] inputFile Parsed file name.
     */
    explicit AstWalker(const std::string &inputFile)
    : m_fileName(inputFile)
    {
    }

    /*!
     * @brief destructor
     */
    virtual ~AstWalker() = default;

    /*!
     * @brief This function walk the AstNode depth-first and invoke handler methods.
     *
     * @param[in] node The root node of the tree to walk.
     *
     * @see void AstWalker::dispatch(AstNode * node)
     * @see void AstWalker::walk(AstNode * node)
     */
    virtual void startWalk(AstNode *node);

protected:
    std::string m_fileName; /*!< @brief Stores name of file from which AST was built. */
    struct top_down
    {
    }; /*!< @brief Specifier type to select top-down handler using ADL. */
    struct bottom_up
    {
    }; /*!< @brief Specifier type to select bottom-up handler using ADL. */

    /*!
     * @brief This function walk the AstNode depth-first and invoke handler methods.
     *
     * This function call for dispatch given node before and after calling walk
     * function for node children.
     *
     * @param[in] node The root node of the tree to walk.
     *
     * @see void AstWalker::dispatch(AstNode * node)
     */
    virtual void walk(AstNode *node);

    /*!
     * @brief This function invoke the appropriate handler method for the node's token type..
     *
     * This function call handle functions for AstNode, which is specified with node token.
     * The template argument @c D specifies whether the top-down or bottom-up handler should
     * be called, by passing either of the top_down or bottom_up typenames.
     *
     * @param[in] node Node to handle.
     *
     * @exception internal_error Thrown if handle function return new node, which want replace given node, but given
     * node is root.
     *
     * @see void AstWalker::walk(AstNode * node)
     */
    template <typename D>
    void dispatch(AstNode *node);

    /*
     * @name Top-down handlers
     *
     * @brief Top-down handlers types, which can be called.
     */
    //@{
    virtual void handleRoot(AstNode *node, top_down) { (void)node; };
    virtual AstNode *handleProgram(AstNode *node, top_down)
    {
        (void)node;
        return nullptr;
    }
    virtual AstNode *handleConst(AstNode *node, top_down)
    {
        (void)node;
        return nullptr;
    }
    virtual AstNode *handleChildren(AstNode *node, top_down)
    {
        (void)node;
        return nullptr;
    }
    virtual AstNode *handleType(AstNode *node, top_down)
    {
        (void)node;
        return nullptr;
    }
    virtual AstNode *handleEnum(AstNode *node, top_down)
    {
        (void)node;
        return nullptr;
    }
    virtual AstNode *handleEnumMember(AstNode *node, top_down)
    {
        (void)node;
        return nullptr;
    }
    virtual AstNode *handleStruct(AstNode *node, top_down)
    {
        (void)node;
        return nullptr;
    }
    virtual AstNode *handleStructMember(AstNode *node, top_down)
    {
        (void)node;
        return nullptr;
    }
    virtual AstNode *handleUnion(AstNode *node, top_down)
    {
        (void)node;
        return nullptr;
    }
    virtual AstNode *handleUnionCase(AstNode *node, top_down)
    {
        (void)node;
        return nullptr;
    }
    virtual AstNode *handleInterface(AstNode *node, top_down)
    {
        (void)node;
        return nullptr;
    }
    virtual AstNode *handleFunction(AstNode *node, top_down)
    {
        (void)node;
        return nullptr;
    }
    virtual AstNode *handleParam(AstNode *node, top_down)
    {
        (void)node;
        return nullptr;
    }
    virtual AstNode *handleExpr(AstNode *node, top_down)
    {
        (void)node;
        return nullptr;
    }
    virtual AstNode *handleBinaryOp(AstNode *node, top_down)
    {
        (void)node;
        return nullptr;
    }
    virtual AstNode *handleUnaryOp(AstNode *node, top_down)
    {
        (void)node;
        return nullptr;
    }
    virtual AstNode *handleAnnotation(AstNode *node, top_down)
    {
        (void)node;
        return nullptr;
    }
    //@}

    /*
     * @name  Bottom-up handlers
     *
     * @brief Bottom-up handlers types, which can be called.
     */
    //@{
    virtual void handleRoot(AstNode *node, bottom_up) { (void)node; };
    virtual AstNode *handleProgram(AstNode *node, bottom_up)
    {
        (void)node;
        return nullptr;
    }
    virtual AstNode *handleConst(AstNode *node, bottom_up)
    {
        (void)node;
        return nullptr;
    }
    virtual AstNode *handleChildren(AstNode *node, bottom_up)
    {
        (void)node;
        return nullptr;
    }
    virtual AstNode *handleType(AstNode *node, bottom_up)
    {
        (void)node;
        return nullptr;
    }
    virtual AstNode *handleEnum(AstNode *node, bottom_up)
    {
        (void)node;
        return nullptr;
    }
    virtual AstNode *handleEnumMember(AstNode *node, bottom_up)
    {
        (void)node;
        return nullptr;
    }
    virtual AstNode *handleStruct(AstNode *node, bottom_up)
    {
        (void)node;
        return nullptr;
    }
    virtual AstNode *handleStructMember(AstNode *node, bottom_up)
    {
        (void)node;
        return nullptr;
    }
    virtual AstNode *handleUnion(AstNode *node, bottom_up)
    {
        (void)node;
        return nullptr;
    }
    virtual AstNode *handleUnionCase(AstNode *node, bottom_up)
    {
        (void)node;
        return nullptr;
    }
    virtual AstNode *handleInterface(AstNode *node, bottom_up)
    {
        (void)node;
        return nullptr;
    }
    virtual AstNode *handleFunction(AstNode *node, bottom_up)
    {
        (void)node;
        return nullptr;
    }
    virtual AstNode *handleParam(AstNode *node, bottom_up)
    {
        (void)node;
        return nullptr;
    }
    virtual AstNode *handleExpr(AstNode *node, bottom_up)
    {
        (void)node;
        return nullptr;
    }
    virtual AstNode *handleBinaryOp(AstNode *node, bottom_up)
    {
        (void)node;
        return nullptr;
    }
    virtual AstNode *handleUnaryOp(AstNode *node, bottom_up)
    {
        (void)node;
        return nullptr;
    }
    virtual AstNode *handleAnnotation(AstNode *node, bottom_up)
    {
        (void)node;
        return nullptr;
    }
    //@}
};

} // namespace erpcgen

#endif // _EMBEDDED_RPC__ASTWALKER_H_
