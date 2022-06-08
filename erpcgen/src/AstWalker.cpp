/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "AstWalker.hpp"

#include "ErpcLexer.hpp"
#include "Logging.hpp"

using namespace erpcgen;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

void AstWalker::startWalk(AstNode *node)
{
    top_down dirTopDown;
    handleRoot(node, dirTopDown);
    walk(node);
    bottom_up dirBottomUp;
    handleRoot(node, dirBottomUp);
}

void AstWalker::walk(AstNode *node)
{
    if (!node)
    {
        return;
    }

    // Process the node top-down.
    dispatch<top_down>(node);

    for (auto i : *node)
    {
        walk(i);
    }

    // Process this node bottom-up.
    dispatch<bottom_up>(node);
}

template <typename D>
void AstWalker::dispatch(AstNode *node)
{
    D dir;

    if (!node)
    {
        return;
    }

    Token &tok = node->getToken();

    AstNode *rewrite = nullptr;
    switch (tok.getToken())
    {
        case TOK_PROGRAM:
            rewrite = handleProgram(node, dir);
            break;
        case TOK_CHILDREN:
            break;
        case TOK_TYPE:
            rewrite = handleType(node, dir);
            break;
        case TOK_ENUM:
            rewrite = handleEnum(node, dir);
            break;
        case TOK_ENUM_MEMBER:
            rewrite = handleEnumMember(node, dir);
            break;
        case TOK_STRUCT:
            rewrite = handleStruct(node, dir);
            break;
        case TOK_STRUCT_MEMBER:
            rewrite = handleStructMember(node, dir);
            break;
        case TOK_UNION:
            rewrite = handleUnion(node, dir);
            break;
        case TOK_UNION_CASE:
            rewrite = handleUnionCase(node, dir);
            break;
        case TOK_INTERFACE:
            rewrite = handleInterface(node, dir);
            break;
        case TOK_FUNCTION:
            rewrite = handleFunction(node, dir);
            break;
        case TOK_PARAM:
            rewrite = handleParam(node, dir);
            break;
        case TOK_CONST:
            rewrite = handleConst(node, dir);
            break;
        case TOK_EXPR:
            rewrite = handleExpr(node, dir);
            break;
        case TOK_ANNOTATION:
            rewrite = handleAnnotation(node, dir);
            break;
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
        case '&':
        case '|':
        case '^':
        case TOK_LSHIFT:
        case TOK_RSHIFT:
            rewrite = handleBinaryOp(node, dir);
            break;
        case TOK_UNARY_NEGATE:
        case '~':
            rewrite = handleUnaryOp(node, dir);
            break;
    }

    if (rewrite)
    {
        if (node->getParent())
        {
            Log::debug("rewriting %s to %s\n", node->getDescription().c_str(), rewrite->getDescription().c_str());
            node->getParent()->replaceChild(node, rewrite);
        }
        else
        {
            throw internal_error("attempted rewrite of root node!!");
        }
    }
}
