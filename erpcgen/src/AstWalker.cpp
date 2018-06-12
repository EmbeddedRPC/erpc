/*
 * The Clear BSD License
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted (subject to the limitations in the disclaimer below) provided
 * that the following conditions are met:
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

#include "AstWalker.h"
#include "ErpcLexer.h"
#include "Logging.h"

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
