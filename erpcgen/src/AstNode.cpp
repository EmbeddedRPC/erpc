/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "AstNode.hpp"

#include "ErpcLexer.hpp"
#include "Utils.hpp"
#include "format_string.hpp"

#include <cstdio>

using namespace erpcgen;
using namespace std;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

AstNode::AstNode(const AstNode &other)
: m_token(other.m_token)
, m_parent(other.m_parent)
{
    // Clone children.
    for (auto it : other.m_children)
    {
        if (it)
        {
            appendChild(it->clone());
        }
        else
        {
            appendChild(nullptr);
        }
    }

    // Clone attributes.
    for (auto it : other.m_attributes)
    {
        if (it.second)
        {
            m_attributes[it.first] = it.second->clone();
        }
    }
}

AstNode &AstNode::operator=(const AstNode &other)
{
    m_token = other.m_token;
    m_parent = other.m_parent;

    // Clone children.
    for (auto it : other.m_children)
    {
        if (it)
        {
            appendChild(it->clone());
        }
        else
        {
            appendChild(nullptr);
        }
    }

    // Clone attributes.
    for (auto it : other.m_attributes)
    {
        if (it.second)
        {
            m_attributes[it.first] = it.second->clone();
        }
    }

    return *this;
}

AstNode::~AstNode()
{
    // Delete children.
    for (auto it : m_children)
    {
        if (it)
        {
            delete it;
        }
    }

    // Delete attribute values.
    for (auto it : m_attributes)
    {
        if (it.second)
        {
            delete it.second;
        }
    }
}

bool AstNode::hasAttribute(const string &name) const
{
    auto it = m_attributes.find(name);
    return it != m_attributes.end();
}

Value *AstNode::getAttribute(const string &name)
{
    auto it = m_attributes.find(name);
    if (it == m_attributes.end())
    {
        throw runtime_error(format_string("no attribute with name '%s'", name.c_str()));
    }
    else
    {
        return it->second;
    }
}

void AstNode::setAttribute(const string &name, Value *node)
{
    m_attributes[name] = node;
}

void AstNode::removeAttribute(const string &name)
{
    if (hasAttribute(name))
    {
        m_attributes.erase(name);
    }
}

void AstNode::appendChild(AstNode *node)
{
    if (node)
    {
        node->setParent(this);
    }
    m_children.push_back(node);
}

size_t AstNode::getIndexOfChild(AstNode *child)
{
    size_t n = 0;
    for (auto i : m_children)
    {
        if (child == i)
        {
            return n;
        }
        ++n;
    }
    return -1;
}

size_t AstNode::getIndex()
{
    return m_parent ? m_parent->getIndexOfChild(this) : 0;
}

void AstNode::reverseExpr()
{
    AstNode *parent = this;
    AstNode *backup = this->getChild(1);
    if (parent->getChild(0)->getToken().getToken() == TOK_ARRAY)
    {
        while (parent->getChild(0)->getToken().getToken() == TOK_ARRAY)
        {
            AstNode *parentExpr = parent->getChild(1);
            AstNode *childExpr = parent->getChild(0)->getChild(1);
            parent->replaceChild(parentExpr, childExpr);
            parent = parent->getChild(0);
        }
        parent->replaceChild(parent->getChild(1), backup);
    }
}

void AstNode::replaceChild(AstNode *original, AstNode *replacement)
{
    size_t index = getIndexOfChild(original);
    m_children[index] = replacement;
    replacement->setParent(this);
    //    delete original;
}

string AstNode::getDescription() const
{
    const Token &tok = getToken();
    const Value *val = tok.getValue();
    const char *tokenName = get_token_name(tok.getToken());
    string output = format_string("%s", tokenName);
    string valToString = val ? val->toString().c_str() : "<null>";
    const token_loc_t &loc = tok.getLocation();
    if (val && tok.getToken() == TOK_ML_COMMENT)
    {
        replaceAll(valToString, "\r\n", " ");
        replaceAll(valToString, "\n", " ");
        if (valToString.size() > 55)
        {
            valToString = valToString.substr(0, 25) + " ... " + valToString.substr(valToString.size() - 20, 20);
        }
    }

    /*
     * Add to output information about lines and columns positions for tokens.
     * This informations is not for virtual tokens used for AST.
     */
    switch (m_token.getToken())
    {
        case TOK_CHILDREN:
        case TOK_ENUM_MEMBER:
        case TOK_STRUCT_MEMBER:
        case TOK_FUNCTION:
        case TOK_PARAM:
        case TOK_EXPR:
        case TOK_ANNOTATION:
        case TOK_UNARY_NEGATE:
        case TOK_ARRAY:
        case TOK_UNION_CASE:
            break;
        default: {
            output += " " + valToString + " ";
            if (valToString.size() == 1)
            {
                output += format_string("[%d:%d]", loc.m_firstLine, loc.m_firstChar);
            }
            else
            {
                output +=
                    format_string("[%d:%d-%d:%d]", loc.m_firstLine, loc.m_firstChar, loc.m_lastLine, loc.m_lastChar);
            }
            break;
        }
    }
    return output;
}

void AstPrinter::dispatch(AstNode *node, int childIndex)
{
    // Print this node.
    print(node, childIndex);

    if (node)
    {
        // Push current depth.
        m_depthStack.push(m_depth);
        ++m_depth;

        // Dispatch to children.
        childIndex = 0;
        for (auto i : *node)
        {
            dispatch(i, childIndex);
            ++childIndex;
        }

        // Pop depth.
        m_depth = m_depthStack.top();
        m_depthStack.pop();
    }
}

void AstPrinter::print(AstNode *node, int childIndex)
{
    printIndent(m_depth);

    if (!node)
    {
        Log::debug2("%d: <null>\n", childIndex);
        return;
    }

    Log::debug2("%d: %s\n", childIndex, node->getDescription().c_str());
}

void AstPrinter::printIndent(int indent) const
{
    int i;
    for (i = 0; i < indent; ++i)
    {
        Log::debug2("   ");
    }
}
