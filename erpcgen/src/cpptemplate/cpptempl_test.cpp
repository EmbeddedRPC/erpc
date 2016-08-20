// Copyright (c) 2010-2014 Ryan Ginstrom
// Copyright (c) 2014 Freescale Semiconductor, Inc.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#define CPPTEMPL_UNIT_TEST
#include "cpptempl.h"
#include "cpptempl.cpp"

#define BOOST_TEST_ALTERNATIVE_INIT_API
#define BOOST_TEST_MODULE cpptemplTests

#include <boost/test/unit_test.hpp>

#pragma warning(disable : 4996) // doesn't like wcstombs

#include "unit_testing.h"

#include <utility>
#include <exception>
#include <cstdint>

using namespace boost::unit_test;
using namespace std;
using namespace cpptempl;
using namespace cpptempl::impl;

std::string gettext(node_ptr node, data_map &data)
{
    ostringstream stream;
    node->gettext(stream, data);
    return stream.str();
}

// ------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(TestCppData)

// DataMap
BOOST_AUTO_TEST_CASE(test_DataMap_getvalue)
{
    data_map items;
    data_ptr data(new DataMap(items));
    BOOST_CHECK_THROW(data->getvalue(), TemplateException);
}
BOOST_AUTO_TEST_CASE(test_DataMap_getlist_throws)
{
    data_map items;
    data_ptr data(new DataMap(items));

    BOOST_CHECK_THROW(data->getlist(), TemplateException);
}
BOOST_AUTO_TEST_CASE(test_DataMap_getitem)
{
    data_map items;
    items["key"] = data_ptr(new DataValue("foo"));
    data_ptr data(new DataMap(items));

    BOOST_CHECK_EQUAL(data->getmap()["key"]->getvalue(), "foo");
}
BOOST_AUTO_TEST_CASE(test_DataMap_parse_path)
{
    data_map items;
    data_map foo;
    data_map bar;
    data_map baz;
    items["a"] = "a";
    foo["b"] = "b";
    bar["c"] = "c";
    baz["d"] = "d";
    bar["baz"] = baz;
    foo["bar"] = bar;
    items["foo"] = foo;
    BOOST_CHECK_EQUAL(items.parse_path("a")->getvalue(), "a");
    BOOST_CHECK_EQUAL(items.parse_path("foo.b")->getvalue(), "b");
    BOOST_CHECK_EQUAL(items.parse_path("foo.bar.c")->getvalue(), "c");
    BOOST_CHECK_EQUAL(items.parse_path("foo.bar.baz.d")->getvalue(), "d");
    BOOST_CHECK_THROW(items.parse_path("xx.yy"), data_map::key_error);
    BOOST_CHECK_THROW(items.parse_path("foo.bar.yy"), data_map::key_error);
}
BOOST_AUTO_TEST_CASE(test_DataMap_parent)
{
    data_map items;
    items["key"] = "foo";
    items["a"] = "zz";
    data_ptr data(new DataMap(items));

    BOOST_CHECK_EQUAL(data->getmap()["key"]->getvalue(), "foo");
    BOOST_CHECK_EQUAL(data->getmap()["a"]->getvalue(), "zz");

    data_map child;
    child["key"] = "bar";
    data_ptr data2(new DataMap(child));
    BOOST_CHECK_EQUAL(data2->getmap()["key"]->getvalue(), "bar");
    BOOST_CHECK_EQUAL(data2->getmap().has("a"), false);

    data2->getmap().set_parent(&items);
    BOOST_CHECK_EQUAL(data2->getmap()["key"]->getvalue(), "bar");
    BOOST_CHECK_EQUAL(data2->getmap()["a"]->getvalue(), "zz");
}
BOOST_AUTO_TEST_CASE(test_DataMap_parse_path_with_parent)
{
    data_map items;
    items["key"] = "foo";
    items["a"] = "zz";
    data_ptr data(new DataMap(items));

    BOOST_CHECK_EQUAL(data->getmap().parse_path("key")->getvalue(), "foo");
    BOOST_CHECK_EQUAL(data->getmap().parse_path("a")->getvalue(), "zz");

    data_map child;
    child["key"] = "bar";
    data_ptr data2(new DataMap(child));
    BOOST_CHECK_EQUAL(data2->getmap().parse_path("key")->getvalue(), "bar");
    BOOST_CHECK_EQUAL(data2->getmap().has("a"), false);

    data2->getmap().set_parent(&items);
    BOOST_CHECK_EQUAL(data2->getmap().parse_path("key")->getvalue(), "bar");
    BOOST_CHECK_EQUAL(data2->getmap().parse_path("a")->getvalue(), "zz");
}
BOOST_AUTO_TEST_CASE(test_DataMap_move)
{
    data_map items;
    items["key"] = "foo";
    data_ptr data(new DataMap(std::move(items)));

    BOOST_CHECK_EQUAL(data->getmap()["key"]->getvalue(), "foo");
    BOOST_CHECK(!items.has("key"));
}
// DataList
BOOST_AUTO_TEST_CASE(test_DataList_getvalue)
{
    data_list items;
    data_ptr data(new DataList(items));

    BOOST_CHECK_THROW(data->getvalue(), TemplateException);
}
BOOST_AUTO_TEST_CASE(test_DataList_getlist_throws)
{
    data_list items;
    items.push_back(make_data("bar"));
    data_ptr data(new DataList(items));

    BOOST_CHECK_EQUAL(data->getlist().size(), 1u);
}
BOOST_AUTO_TEST_CASE(test_DataList_getitem_throws)
{
    data_list items;
    data_ptr data(new DataList(items));

    BOOST_CHECK_THROW(data->getmap(), TemplateException);
}
BOOST_AUTO_TEST_CASE(test_DataList_move)
{
    data_list items;
    items.push_back("a");
    items.push_back("b");
    BOOST_CHECK_EQUAL(items.size(), 2);

    data_ptr data(new DataList(std::move(items)));

    BOOST_CHECK_EQUAL(data->getlist().size(), 2);
    BOOST_CHECK_EQUAL(items.size(), 0);
    BOOST_CHECK_EQUAL(data->getlist()[0]->getvalue(), "a");
}
// DataValue
BOOST_AUTO_TEST_CASE(test_DataValue_getvalue)
{
    data_ptr data(new DataValue("foo"));

    BOOST_CHECK_EQUAL(data->getvalue(), "foo");
}
BOOST_AUTO_TEST_CASE(test_DataValue_getlist_throws)
{
    data_ptr data(new DataValue("foo"));

    BOOST_CHECK_THROW(data->getlist(), TemplateException);
}
BOOST_AUTO_TEST_CASE(test_DataValue_getitem_throws)
{
    data_ptr data(new DataValue("foo"));

    BOOST_CHECK_THROW(data->getmap(), TemplateException);
}
BOOST_AUTO_TEST_CASE(test_DataValue_move)
{
    string foo = "foo";
    BOOST_CHECK_EQUAL(foo, "foo");

    data_ptr data(new DataValue(std::move(foo)));

    BOOST_CHECK_EQUAL(data->getvalue(), "foo");
    BOOST_CHECK_EQUAL(foo.size(), 0);
}
// DataBool
BOOST_AUTO_TEST_CASE(test_DataBool_true)
{
    data_ptr data(new DataBool(true));

    BOOST_CHECK_EQUAL(data->getvalue(), "true");
    BOOST_CHECK(!data->empty());
}
BOOST_AUTO_TEST_CASE(test_DataBool_false)
{
    data_ptr data(new DataBool(false));

    BOOST_CHECK_EQUAL(data->getvalue(), "false");
    BOOST_CHECK(data->empty());
}
BOOST_AUTO_TEST_CASE(test_DataBool_getitem_throws)
{
    data_ptr data(new DataBool(false));

    BOOST_CHECK_THROW(data->getmap(), TemplateException);
}
// DataInt
BOOST_AUTO_TEST_CASE(test_DataInt_0)
{
    DataInt *i = new DataInt(0);

    BOOST_CHECK_EQUAL(i->getint(), 0);
    BOOST_CHECK_EQUAL(i->getvalue(), "0");
    BOOST_CHECK(i->empty());
}
BOOST_AUTO_TEST_CASE(test_DataInt_pos)
{
    DataInt *i = new DataInt(10);

    BOOST_CHECK_EQUAL(i->getint(), 10);
    BOOST_CHECK_EQUAL(i->getvalue(), "10");
    BOOST_CHECK(!i->empty());
}
BOOST_AUTO_TEST_CASE(test_DataInt_neg)
{
    DataInt *i = new DataInt(-10);

    BOOST_CHECK_EQUAL(i->getint(), -10);
    BOOST_CHECK_EQUAL(i->getvalue(), "-10");
    BOOST_CHECK(!i->empty());
}
BOOST_AUTO_TEST_CASE(test_DataInt_unsigned)
{
    DataInt *i = new DataInt(10U);

    BOOST_CHECK_EQUAL(i->getint(), 10);
    BOOST_CHECK_EQUAL(i->getvalue(), "10");
    BOOST_CHECK(!i->empty());
}
BOOST_AUTO_TEST_CASE(test_DataInt_max_signed)
{
    DataInt *i = new DataInt(INT32_MAX);

    BOOST_CHECK_EQUAL(i->getint(), INT32_MAX);
    BOOST_CHECK_EQUAL(i->getvalue(), "2147483647");
    BOOST_CHECK(!i->empty());
}
BOOST_AUTO_TEST_CASE(test_DataInt_max_unsigned)
{
    DataInt *i = new DataInt(UINT32_MAX);

    BOOST_CHECK_EQUAL(i->getint(), -1);
    BOOST_CHECK_EQUAL(i->getvalue(), "-1");
    BOOST_CHECK(!i->empty());
}
BOOST_AUTO_TEST_CASE(test_DataInt_wrapped)
{
    data_ptr data(new DataInt(10));

    BOOST_CHECK_EQUAL(data->getint(), 10);
    BOOST_CHECK_EQUAL(data->getvalue(), "10");
    BOOST_CHECK(!data->empty());
}
BOOST_AUTO_TEST_CASE(test_DataInt_getitem_throws)
{
    data_ptr data(new DataInt(10));

    BOOST_CHECK_THROW(data->getmap(), TemplateException);
}
BOOST_AUTO_TEST_CASE(test_DataInt_not_template)
{
    data_ptr data(new DataInt(10));

    BOOST_CHECK(!data.is_template());
}
// DataTemplate
BOOST_AUTO_TEST_CASE(test_DataTemplate_empty)
{
    DataTemplate t("");
    BOOST_CHECK(!t.empty()); // templates are always non-empty
    BOOST_CHECK_EQUAL(t.getvalue(), "");
    BOOST_CHECK_EQUAL(t.params().size(), 0u);
}
BOOST_AUTO_TEST_CASE(test_DataTemplate_is_template)
{
    data_ptr d(new DataTemplate(""));
    BOOST_CHECK(d.is_template());
}
BOOST_AUTO_TEST_SUITE_END()

// ------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(TestCppStmtTokenizer)
BOOST_AUTO_TEST_CASE(test_empty)
{
    token_vector t = tokenize_statement("");
    BOOST_CHECK_EQUAL(t.size(), 0u);
}
BOOST_AUTO_TEST_CASE(test_empty_comment)
{
    token_vector t = tokenize_statement("-- hi there");
    BOOST_CHECK_EQUAL(t.size(), 0u);
}
BOOST_AUTO_TEST_CASE(test_key_path)
{
    token_vector t = tokenize_statement("foo");
    BOOST_CHECK_EQUAL(t.size(), 1u);
    BOOST_CHECK_EQUAL(t[0].get_type(), KEY_PATH_TOKEN);
    BOOST_CHECK_EQUAL(t[0].get_value(), "foo");

    t = tokenize_statement(" foo.bar ");
    BOOST_CHECK_EQUAL(t.size(), 1u);
    BOOST_CHECK_EQUAL(t[0].get_type(), KEY_PATH_TOKEN);
    BOOST_CHECK_EQUAL(t[0].get_value(), "foo.bar");

    t = tokenize_statement("   foo.monkey.helicopter  \n  spaz.umbrella");
    BOOST_CHECK_EQUAL(t.size(), 2u);
    BOOST_CHECK_EQUAL(t[0].get_type(), KEY_PATH_TOKEN);
    BOOST_CHECK_EQUAL(t[0].get_value(), "foo.monkey.helicopter");
    BOOST_CHECK_EQUAL(t[1].get_type(), KEY_PATH_TOKEN);
    BOOST_CHECK_EQUAL(t[1].get_value(), "spaz.umbrella");
}
BOOST_AUTO_TEST_CASE(test_str_lit)
{
    token_vector t = tokenize_statement("'abc'");
    BOOST_CHECK_EQUAL(t.size(), 1u);
    BOOST_CHECK_EQUAL(t[0].get_type(), STRING_LITERAL_TOKEN);
    BOOST_CHECK_EQUAL(t[0].get_value(), "abc");

    t = tokenize_statement("\"abc\"");
    BOOST_CHECK_EQUAL(t.size(), 1u);
    BOOST_CHECK_EQUAL(t[0].get_type(), STRING_LITERAL_TOKEN);
    BOOST_CHECK_EQUAL(t[0].get_value(), "abc");
}
BOOST_AUTO_TEST_CASE(test_str_lit_spaces)
{
    token_vector t = tokenize_statement("'abc def'");
    BOOST_CHECK_EQUAL(t.size(), 1u);
    BOOST_CHECK_EQUAL(t[0].get_type(), STRING_LITERAL_TOKEN);
    BOOST_CHECK_EQUAL(t[0].get_value(), "abc def");

    t = tokenize_statement("\"abc def\"");
    BOOST_CHECK_EQUAL(t.size(), 1u);
    BOOST_CHECK_EQUAL(t[0].get_type(), STRING_LITERAL_TOKEN);
    BOOST_CHECK_EQUAL(t[0].get_value(), "abc def");
}
BOOST_AUTO_TEST_CASE(test_str_lit_escapes)
{
    token_vector t = tokenize_statement("'\\x41'");
    BOOST_CHECK_EQUAL(t.size(), 1u);
    BOOST_CHECK_EQUAL(t[0].get_type(), STRING_LITERAL_TOKEN);
    BOOST_CHECK_EQUAL(t[0].get_value(), "A");

    t = tokenize_statement("'he said,\\n\\\"hello\\\"'");
    BOOST_CHECK_EQUAL(t.size(), 1u);
    BOOST_CHECK_EQUAL(t[0].get_type(), STRING_LITERAL_TOKEN);
    BOOST_CHECK_EQUAL(t[0].get_value(), "he said,\n\"hello\"");
}
BOOST_AUTO_TEST_CASE(test_bool_lit)
{
    token_vector t = tokenize_statement("true false");
    BOOST_CHECK_EQUAL(t.size(), 2u);
    BOOST_CHECK_EQUAL(t[0].get_type(), TRUE_TOKEN);
    BOOST_CHECK_EQUAL(t[1].get_type(), FALSE_TOKEN);
}
BOOST_AUTO_TEST_CASE(test_int_lit)
{
    token_vector t = tokenize_statement("123");
    BOOST_CHECK_EQUAL(t.size(), 1u);
    BOOST_CHECK_EQUAL(t[0].get_type(), INT_LITERAL_TOKEN);
    BOOST_CHECK_EQUAL(t[0].get_value(), "123");
}
BOOST_AUTO_TEST_CASE(test_int_lit_hex)
{
    token_vector t = tokenize_statement("0x123");
    BOOST_CHECK_EQUAL(t.size(), 1u);
    BOOST_CHECK_EQUAL(t[0].get_type(), INT_LITERAL_TOKEN);
    BOOST_CHECK_EQUAL(t[0].get_value(), "0x123");
}
BOOST_AUTO_TEST_CASE(test_op)
{
    token_vector t = tokenize_statement("== != && || and or ! not ( ) ,");
    BOOST_CHECK_EQUAL(t.size(), 11u);
    BOOST_CHECK_EQUAL(t[0].get_type(), EQ_TOKEN);
    BOOST_CHECK_EQUAL(t[1].get_type(), NEQ_TOKEN);
    BOOST_CHECK_EQUAL(t[2].get_type(), AND_TOKEN);
    BOOST_CHECK_EQUAL(t[3].get_type(), OR_TOKEN);
    BOOST_CHECK_EQUAL(t[4].get_type(), AND_TOKEN);
    BOOST_CHECK_EQUAL(t[5].get_type(), OR_TOKEN);
    BOOST_CHECK_EQUAL(t[6].get_type(), NOT_TOKEN);
    BOOST_CHECK_EQUAL(t[7].get_type(), NOT_TOKEN);
    BOOST_CHECK_EQUAL(t[8].get_type(), OPEN_PAREN_TOKEN);
    BOOST_CHECK_EQUAL(t[9].get_type(), CLOSE_PAREN_TOKEN);
    BOOST_CHECK_EQUAL(t[10].get_type(), COMMA_TOKEN);

    t = tokenize_statement("==!=&&||and or!not(),");
    BOOST_CHECK_EQUAL(t.size(), 11u);
    BOOST_CHECK_EQUAL(t[0].get_type(), EQ_TOKEN);
    BOOST_CHECK_EQUAL(t[1].get_type(), NEQ_TOKEN);
    BOOST_CHECK_EQUAL(t[2].get_type(), AND_TOKEN);
    BOOST_CHECK_EQUAL(t[3].get_type(), OR_TOKEN);
    BOOST_CHECK_EQUAL(t[4].get_type(), AND_TOKEN);
    BOOST_CHECK_EQUAL(t[5].get_type(), OR_TOKEN);
    BOOST_CHECK_EQUAL(t[6].get_type(), NOT_TOKEN);
    BOOST_CHECK_EQUAL(t[7].get_type(), NOT_TOKEN);
    BOOST_CHECK_EQUAL(t[8].get_type(), OPEN_PAREN_TOKEN);
    BOOST_CHECK_EQUAL(t[9].get_type(), CLOSE_PAREN_TOKEN);
    BOOST_CHECK_EQUAL(t[10].get_type(), COMMA_TOKEN);
}
BOOST_AUTO_TEST_CASE(test_op2)
{
    token_vector t = tokenize_statement("+ - * / % &");
    BOOST_CHECK_EQUAL(t.size(), 6u);
    BOOST_CHECK_EQUAL(t[0].get_type(), PLUS_TOKEN);
    BOOST_CHECK_EQUAL(t[1].get_type(), MINUS_TOKEN);
    BOOST_CHECK_EQUAL(t[2].get_type(), TIMES_TOKEN);
    BOOST_CHECK_EQUAL(t[3].get_type(), DIVIDE_TOKEN);
    BOOST_CHECK_EQUAL(t[4].get_type(), MOD_TOKEN);
    BOOST_CHECK_EQUAL(t[5].get_type(), CONCAT_TOKEN);

    t = tokenize_statement("+-*/%&");
    BOOST_CHECK_EQUAL(t.size(), 6u);
    BOOST_CHECK_EQUAL(t[0].get_type(), PLUS_TOKEN);
    BOOST_CHECK_EQUAL(t[1].get_type(), MINUS_TOKEN);
    BOOST_CHECK_EQUAL(t[2].get_type(), TIMES_TOKEN);
    BOOST_CHECK_EQUAL(t[3].get_type(), DIVIDE_TOKEN);
    BOOST_CHECK_EQUAL(t[4].get_type(), MOD_TOKEN);
    BOOST_CHECK_EQUAL(t[5].get_type(), CONCAT_TOKEN);
}
BOOST_AUTO_TEST_CASE(test_op3)
{
    token_vector t = tokenize_statement("> >= < <=");
    BOOST_CHECK_EQUAL(t.size(), 4u);
    BOOST_CHECK_EQUAL(t[0].get_type(), GT_TOKEN);
    BOOST_CHECK_EQUAL(t[1].get_type(), GE_TOKEN);
    BOOST_CHECK_EQUAL(t[2].get_type(), LT_TOKEN);
    BOOST_CHECK_EQUAL(t[3].get_type(), LE_TOKEN);

    t = tokenize_statement(">>=<<=");
    BOOST_CHECK_EQUAL(t.size(), 4u);
    BOOST_CHECK_EQUAL(t[0].get_type(), GT_TOKEN);
    BOOST_CHECK_EQUAL(t[1].get_type(), GE_TOKEN);
    BOOST_CHECK_EQUAL(t[2].get_type(), LT_TOKEN);
    BOOST_CHECK_EQUAL(t[3].get_type(), LE_TOKEN);
}
BOOST_AUTO_TEST_CASE(test_kwd)
{
    token_vector t = tokenize_statement("for in if elif else def endfor endif enddef");
    BOOST_CHECK_EQUAL(t.size(), 9u);
    BOOST_CHECK_EQUAL(t[0].get_type(), FOR_TOKEN);
    BOOST_CHECK_EQUAL(t[1].get_type(), IN_TOKEN);
    BOOST_CHECK_EQUAL(t[2].get_type(), IF_TOKEN);
    BOOST_CHECK_EQUAL(t[3].get_type(), ELIF_TOKEN);
    BOOST_CHECK_EQUAL(t[4].get_type(), ELSE_TOKEN);
    BOOST_CHECK_EQUAL(t[5].get_type(), DEF_TOKEN);
    BOOST_CHECK_EQUAL(t[6].get_type(), ENDFOR_TOKEN);
    BOOST_CHECK_EQUAL(t[7].get_type(), ENDIF_TOKEN);
    BOOST_CHECK_EQUAL(t[8].get_type(), ENDDEF_TOKEN);
}
BOOST_AUTO_TEST_CASE(test_token_iterator_empty)
{
    token_vector t = tokenize_statement("");
    TokenIterator i(t);
    BOOST_CHECK_EQUAL(i.size(), 0u);
    BOOST_CHECK(i.empty());
    BOOST_CHECK(!i.is_valid());
    BOOST_CHECK_EQUAL(i.get()->get_type(), END_TOKEN);
    ++i;
    BOOST_CHECK_EQUAL(i.get()->get_type(), END_TOKEN);
    BOOST_CHECK_THROW(i.match(EQ_TOKEN), TemplateException);
}
BOOST_AUTO_TEST_CASE(test_token_iterator_notempty)
{
    token_vector t = tokenize_statement("key.path == 'str'");
    TokenIterator i(t);
    BOOST_CHECK_EQUAL(i.size(), 3u);
    BOOST_CHECK(!i.empty());
    BOOST_CHECK(i.is_valid());
    BOOST_CHECK_EQUAL(i.get()->get_type(), KEY_PATH_TOKEN);
    ++i;
    BOOST_CHECK(i.is_valid());
    BOOST_CHECK_EQUAL(i.get()->get_type(), EQ_TOKEN);
    BOOST_CHECK_EQUAL(i.match(EQ_TOKEN)->get_type(), EQ_TOKEN);
    BOOST_CHECK(i.is_valid());
    BOOST_CHECK_EQUAL(i.get()->get_type(), STRING_LITERAL_TOKEN);
    BOOST_CHECK_EQUAL(i.match(STRING_LITERAL_TOKEN)->get_type(), STRING_LITERAL_TOKEN);
}
BOOST_AUTO_TEST_SUITE_END()

// ------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(TestCppNode)

// NodeVar
BOOST_AUTO_TEST_CASE(TestNodeVarType)
{
    token_vector tokens = impl::tokenize_statement("foo");
    NodeVar node(tokens);
    BOOST_CHECK_EQUAL(node.gettype(), NODE_TYPE_VAR);
}
BOOST_AUTO_TEST_CASE(TestNodeVar)
{
    node_ptr node(new NodeVar(tokenize_statement("foo")));
    data_map data;
    data["foo"] = make_data("bar");
    BOOST_CHECK_EQUAL(gettext(node, data), "bar");
}
BOOST_AUTO_TEST_CASE(TestNodeVarCantHaveChildren)
{
    NodeVar node(tokenize_statement("foo"));
    node_vector children;
    BOOST_CHECK_THROW(node.set_children(children), TemplateException);
}
// NodeText
BOOST_AUTO_TEST_CASE(TestNodeTextType)
{
    NodeText node("foo");
    BOOST_CHECK_EQUAL(node.gettype(), NODE_TYPE_TEXT);
}
BOOST_AUTO_TEST_CASE(TestNodeText)
{
    node_ptr node(new NodeText("foo"));
    data_map data;
    data["foo"] = make_data("bar");
    BOOST_CHECK_EQUAL(gettext(node, data), "foo");
}
BOOST_AUTO_TEST_CASE(TestNodeTextCantHaveChildrenSet)
{
    NodeText node("foo");
    node_vector children;
    BOOST_CHECK_THROW(node.set_children(children), TemplateException);
}
BOOST_AUTO_TEST_CASE(TestNodeTextCantHaveChildrenGet)
{
    NodeText node("foo");
    node_vector children;
    BOOST_CHECK_THROW(node.get_children(), TemplateException);
}
// NodeFor
BOOST_AUTO_TEST_CASE(TestNodeForBadSyntax)
{
    BOOST_CHECK_THROW(NodeFor node(tokenize_statement("foo"), true), TemplateException);
}
BOOST_AUTO_TEST_CASE(TestNodeForType)
{
    NodeFor node(tokenize_statement("for item in items"), true);
    BOOST_CHECK_EQUAL(node.gettype(), NODE_TYPE_FOR);
}
BOOST_AUTO_TEST_CASE(TestNodeForTextEmpty)
{
    node_ptr node(new NodeFor(tokenize_statement("for item in items"), true));
    data_map data;
    data_list items;
    items.push_back(make_data("first"));
    data["items"] = make_data(items);
    BOOST_CHECK_EQUAL(gettext(node, data), "");
}
BOOST_AUTO_TEST_CASE(TestNodeForTextOneVar)
{
    node_vector children;
    children.push_back(node_ptr(new NodeVar(tokenize_statement("item"))));
    node_ptr node(new NodeFor(tokenize_statement("for item in items"), true));
    node->set_children(children);
    data_map data;
    data_list items;
    items.push_back(make_data("first "));
    items.push_back(make_data("second "));
    data["items"] = make_data(items);
    BOOST_CHECK_EQUAL(gettext(node, data), "first second ");
}
BOOST_AUTO_TEST_CASE(TestNodeForTextOneVarLoop)
{
    node_vector children;
    children.push_back(node_ptr(new NodeVar(tokenize_statement("loop.index"))));
    node_ptr node(new NodeFor(tokenize_statement("for item in items"), true));
    node->set_children(children);
    data_map data;
    data_list items;
    items.push_back(make_data("first "));
    items.push_back(make_data("second "));
    data["items"] = make_data(items);
    BOOST_CHECK_EQUAL(gettext(node, data), "12");
}
BOOST_AUTO_TEST_CASE(TestNodeForLoopTextVar)
{
    node_vector children;
    children.push_back(node_ptr(new NodeVar(tokenize_statement("loop.index"))));
    children.push_back(node_ptr(new NodeText(". ")));
    children.push_back(node_ptr(new NodeVar(tokenize_statement("item"))));
    children.push_back(node_ptr(new NodeText(" ")));
    node_ptr node(new NodeFor(tokenize_statement("for item in items"), true));
    node->set_children(children);
    data_map data;
    data_list items;
    items.push_back(make_data("first"));
    items.push_back(make_data("second"));
    data["items"] = make_data(items);
    BOOST_CHECK_EQUAL(gettext(node, data), "1. first 2. second ");
}
BOOST_AUTO_TEST_CASE(TestNodeForLoopTextVarDotted)
{
    node_vector children;
    children.push_back(node_ptr(new NodeVar(tokenize_statement("loop.index"))));
    children.push_back(node_ptr(new NodeText(". ")));
    children.push_back(node_ptr(new NodeVar(tokenize_statement("friend.name"))));
    children.push_back(node_ptr(new NodeText(" ")));
    node_ptr node(new NodeFor(tokenize_statement("for friend in person.friends"), true));
    node->set_children(children);

    data_map bob;
    bob["name"] = make_data("Bob");
    data_map betty;
    betty["name"] = make_data("Betty");
    data_list friends;
    friends.push_back(make_data(bob));
    friends.push_back(make_data(betty));
    data_map person;
    person["friends"] = make_data(friends);
    data_map data;
    data["person"] = make_data(person);

    BOOST_CHECK_EQUAL(gettext(node, data), "1. Bob 2. Betty ");
}
BOOST_AUTO_TEST_CASE(TestNodeForTextOneText)
{
    node_vector children;
    children.push_back(node_ptr(new NodeText("{--}")));
    node_ptr node(new NodeFor(tokenize_statement("for item in items"), true));
    node->set_children(children);
    data_map data;
    data_list items;
    items.push_back(make_data("first "));
    items.push_back(make_data("second "));
    data["items"] = make_data(items);
    BOOST_CHECK_EQUAL(gettext(node, data), "{--}{--}");
}

//////////////////////////////////////////////////////////////////////////
// NodeIf
//////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE(TestNodeIfType)
{
    NodeIf node(tokenize_statement("if items"));
    BOOST_CHECK_EQUAL(node.gettype(), NODE_TYPE_IF);
}
// if not empty
BOOST_AUTO_TEST_CASE(TestNodeIfText)
{
    node_vector children;
    children.push_back(node_ptr(new NodeText("{--}")));
    node_ptr node(new NodeIf(tokenize_statement("if item")));
    node->set_children(children);
    data_map data;
    data["item"] = make_data("foo");
    BOOST_CHECK_EQUAL(gettext(node, data), "{--}");
    data["item"] = false;
    BOOST_CHECK_EQUAL(gettext(node, data), "");
}
BOOST_AUTO_TEST_CASE(TestNodeIfVar)
{
    node_vector children;
    children.push_back(node_ptr(new NodeVar(tokenize_statement("item"))));
    node_ptr node(new NodeIf(tokenize_statement("if item")));
    node->set_children(children);
    data_map data;
    data["item"] = make_data("foo");
    BOOST_CHECK_EQUAL(gettext(node, data), "foo");
    data["item"] = false;
    BOOST_CHECK_EQUAL(gettext(node, data), "");
}

// ==
BOOST_AUTO_TEST_CASE(TestNodeIfEqualsTrue)
{
    node_vector children;
    children.push_back(node_ptr(new NodeVar(tokenize_statement("item"))));
    node_ptr node(new NodeIf(tokenize_statement("if item == \"foo\"")));
    node->set_children(children);
    data_map data;
    data["item"] = make_data("foo");
    BOOST_CHECK_EQUAL(gettext(node, data), "foo");
}
BOOST_AUTO_TEST_CASE(TestNodeIfEqualsFalse)
{
    node_vector children;
    children.push_back(node_ptr(new NodeVar(tokenize_statement("item"))));
    node_ptr node(new NodeIf(tokenize_statement("if item == \"bar\"")));
    node->set_children(children);
    data_map data;
    data["item"] = make_data("foo");
    BOOST_CHECK_EQUAL(gettext(node, data), "");
}
BOOST_AUTO_TEST_CASE(TestNodeIfEqualsTwoVarsTrue)
{
    node_vector children;
    children.push_back(node_ptr(new NodeVar(tokenize_statement("item"))));
    node_ptr node(new NodeIf(tokenize_statement("if item == foo")));
    node->set_children(children);
    data_map data;
    data["item"] = make_data("x");
    data["foo"] = make_data("x");
    BOOST_CHECK_EQUAL(gettext(node, data), "x");
    data["foo"] = make_data("z");
    BOOST_CHECK_EQUAL(gettext(node, data), "");
}

// !=
BOOST_AUTO_TEST_CASE(TestNodeIfNotEqualsTrue)
{
    node_vector children;
    children.push_back(node_ptr(new NodeVar(tokenize_statement("item"))));
    node_ptr node(new NodeIf(tokenize_statement("if item != \"foo\"")));
    node->set_children(children);
    data_map data;
    data["item"] = make_data("foo");
    BOOST_CHECK_EQUAL(gettext(node, data), "");
}
BOOST_AUTO_TEST_CASE(TestNodeIfNotEqualsFalse)
{
    node_vector children;
    children.push_back(node_ptr(new NodeVar(tokenize_statement("item"))));
    node_ptr node(new NodeIf(tokenize_statement("if item != \"bar\"")));
    node->set_children(children);
    data_map data;
    data["item"] = make_data("foo");
    BOOST_CHECK_EQUAL(gettext(node, data), "foo");
}

// not
BOOST_AUTO_TEST_CASE(TestNodeIfNotTrueText)
{
    node_vector children;
    children.push_back(node_ptr(new NodeText("{--}")));
    node_ptr node(new NodeIf(tokenize_statement("if not item")));
    node->set_children(children);
    data_map data;
    data["item"] = make_data("foo");
    BOOST_CHECK_EQUAL(gettext(node, data), "");
}
BOOST_AUTO_TEST_CASE(TestNodeIfNotFalseText)
{
    node_vector children;
    children.push_back(node_ptr(new NodeText("{--}")));
    node_ptr node(new NodeIf(tokenize_statement("if not item")));
    node->set_children(children);
    data_map data;
    data["item"] = make_data("");
    BOOST_CHECK_EQUAL(gettext(node, data), "{--}");
}

BOOST_AUTO_TEST_SUITE_END()

// ------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(TestCppExprParser)

BOOST_AUTO_TEST_CASE(test_get_var_value_simple)
{
    token_vector v;
    TokenIterator t(v);
    data_map d;
    d["item"] = "a";
    ExprParser p(t, d);
    data_list params;
    data_ptr r = p.get_var_value("item", params);
    BOOST_CHECK_EQUAL(r->getvalue(), "a");
}
BOOST_AUTO_TEST_CASE(test_get_var_value_dotted)
{
    token_vector v;
    TokenIterator t(v);
    data_map d;
    data_map x;
    x["a"] = "a";
    d["x"] = x;
    ExprParser p(t, d);
    data_list params;
    data_ptr r = p.get_var_value("x.a", params);
    BOOST_CHECK_EQUAL(r->getvalue(), "a");
}
BOOST_AUTO_TEST_CASE(test_get_var_value_fn_count)
{
    token_vector v;
    TokenIterator t(v);
    data_map d;
    data_list a;
    a.push_back("1");
    a.push_back("2");
    a.push_back("3");
    ExprParser p(t, d);
    data_list params;
    params.push_back(a);
    data_ptr r = p.get_var_value("count", params);
    BOOST_CHECK_EQUAL(r->getvalue(), "3");
}
BOOST_AUTO_TEST_CASE(test_get_var_value_fn_empty)
{
    token_vector v;
    TokenIterator t(v);
    data_map d;
    ExprParser p(t, d);
    data_list params;
    params.push_back("");
    data_ptr r = p.get_var_value("empty", params);
    BOOST_CHECK_EQUAL(r->getvalue(), "true");
    params[0] = "x";
    data_ptr m = p.get_var_value("empty", params);
    BOOST_CHECK_EQUAL(m->getvalue(), "false");
}
BOOST_AUTO_TEST_CASE(test_get_var_value_fn_defined)
{
    // Placeholder until defined() is fixed.
}
BOOST_AUTO_TEST_CASE(test_str_lit)
{
    token_vector v = tokenize_statement("'hi'");
    TokenIterator t(v);
    data_map d;
    ExprParser p(t, d);
    data_ptr r = p.parse_expr();
    BOOST_CHECK_EQUAL(r->getvalue(), "hi");
}
BOOST_AUTO_TEST_CASE(test_int_lit)
{
    token_vector v = tokenize_statement("123");
    TokenIterator t(v);
    data_map d;
    ExprParser p(t, d);
    data_ptr r = p.parse_expr();
    BOOST_CHECK_EQUAL(r->getint(), 123);
}
BOOST_AUTO_TEST_CASE(test_int_lit_hex)
{
    token_vector v = tokenize_statement("0x1000");
    TokenIterator t(v);
    data_map d;
    ExprParser p(t, d);
    data_ptr r = p.parse_expr();
    BOOST_CHECK_EQUAL(r->getint(), 4096);
}
BOOST_AUTO_TEST_CASE(test_and)
{
    token_vector v = tokenize_statement("a and b");
    TokenIterator t(v);
    data_map d;
    d["a"] = false;
    d["b"] = false;
    ExprParser p(t, d);
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "false");
    d["a"] = true;
    t.reset();
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "false");
    d["a"] = false;
    d["b"] = true;
    t.reset();
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "false");
    d["a"] = true;
    t.reset();
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "true");
}
BOOST_AUTO_TEST_CASE(test_and_multiline_commented)
{
    token_vector v = tokenize_statement("a -- this is a\nand -- could be &&\nb -- the second operand");
    TokenIterator t(v);
    data_map d;
    d["a"] = false;
    d["b"] = false;
    ExprParser p(t, d);
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "false");
    d["a"] = true;
    t.reset();
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "false");
    d["a"] = false;
    d["b"] = true;
    t.reset();
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "false");
    d["a"] = true;
    t.reset();
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "true");
}
BOOST_AUTO_TEST_CASE(test_or)
{
    token_vector v = tokenize_statement("a or b");
    TokenIterator t(v);
    data_map d;
    d["a"] = false;
    d["b"] = false;
    ExprParser p(t, d);
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "false");
    d["a"] = true;
    t.reset();
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "true");
    d["a"] = false;
    d["b"] = true;
    t.reset();
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "true");
    d["a"] = true;
    t.reset();
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "true");
}
BOOST_AUTO_TEST_CASE(test_or_value)
{
    token_vector v = tokenize_statement("a or b");
    TokenIterator t(v);
    data_map d;
    d["a"] = "x";
    d["b"] = "";
    ExprParser p(t, d);
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "x");
    d["a"] = "";
    d["b"] = "y";
    t.reset();
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "y");
}
BOOST_AUTO_TEST_CASE(test_equal)
{
    token_vector v = tokenize_statement("a == b");
    TokenIterator t(v);
    data_map d;
    d["a"] = "x";
    d["b"] = "y";
    ExprParser p(t, d);
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "false");
    d["a"] = "y";
    t.reset();
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "true");
}
BOOST_AUTO_TEST_CASE(test_not_equal)
{
    token_vector v = tokenize_statement("a != b");
    TokenIterator t(v);
    data_map d;
    d["a"] = "x";
    d["b"] = "y";
    ExprParser p(t, d);
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "true");
    d["a"] = "y";
    t.reset();
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "false");
}
BOOST_AUTO_TEST_CASE(test_gt)
{
    token_vector v = tokenize_statement("a > b");
    TokenIterator t(v);
    data_map d;
    d["a"] = 200;
    d["b"] = 100;
    ExprParser p(t, d);
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "true");
    d["a"] = 50;
    t.reset();
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "false");
    d["b"] = 50;
    t.reset();
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "false");
}
BOOST_AUTO_TEST_CASE(test_gt_txt)
{
    token_vector v = tokenize_statement("a > b");
    TokenIterator t(v);
    data_map d;
    d["a"] = "apple";
    d["b"] = "bear";
    ExprParser p(t, d);
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "false");
    d["a"] = "monkey";
    t.reset();
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "true");
    d["b"] = "monkey";
    t.reset();
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "false");
}
BOOST_AUTO_TEST_CASE(test_ge)
{
    token_vector v = tokenize_statement("a >= b");
    TokenIterator t(v);
    data_map d;
    d["a"] = 200;
    d["b"] = 100;
    ExprParser p(t, d);
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "true");
    d["a"] = 50;
    t.reset();
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "false");
    d["b"] = 50;
    t.reset();
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "true");
}
BOOST_AUTO_TEST_CASE(test_ge_txt)
{
    token_vector v = tokenize_statement("a >= b");
    TokenIterator t(v);
    data_map d;
    d["a"] = "apple";
    d["b"] = "bear";
    ExprParser p(t, d);
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "false");
    d["a"] = "monkey";
    t.reset();
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "true");
    d["b"] = "monkey";
    t.reset();
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "true");
}
BOOST_AUTO_TEST_CASE(test_lt)
{
    token_vector v = tokenize_statement("a < b");
    TokenIterator t(v);
    data_map d;
    d["a"] = 200;
    d["b"] = 100;
    ExprParser p(t, d);
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "false");
    d["a"] = 50;
    t.reset();
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "true");
    d["b"] = 50;
    t.reset();
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "false");
}
BOOST_AUTO_TEST_CASE(test_lt_txt)
{
    token_vector v = tokenize_statement("a < b");
    TokenIterator t(v);
    data_map d;
    d["a"] = "apple";
    d["b"] = "bear";
    ExprParser p(t, d);
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "true");
    d["a"] = "monkey";
    t.reset();
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "false");
    d["b"] = "monkey";
    t.reset();
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "false");
}
BOOST_AUTO_TEST_CASE(test_le)
{
    token_vector v = tokenize_statement("a <= b");
    TokenIterator t(v);
    data_map d;
    d["a"] = 200;
    d["b"] = 100;
    ExprParser p(t, d);
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "false");
    d["a"] = 50;
    t.reset();
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "true");
    d["b"] = 50;
    t.reset();
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "true");
}
BOOST_AUTO_TEST_CASE(test_le_txt)
{
    token_vector v = tokenize_statement("a <= b");
    TokenIterator t(v);
    data_map d;
    d["a"] = "apple";
    d["b"] = "bear";
    ExprParser p(t, d);
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "true");
    d["a"] = "monkey";
    t.reset();
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "false");
    d["b"] = "monkey";
    t.reset();
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "true");
}
BOOST_AUTO_TEST_CASE(test_str_cat)
{
    token_vector v = tokenize_statement("a & b");
    TokenIterator t(v);
    data_map d;
    d["a"] = "hello";
    d["b"] = "world";
    ExprParser p(t, d);
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "helloworld");
    d["a"] = 50;
    d["b"] = true;
    t.reset();
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "50true");
}
BOOST_AUTO_TEST_CASE(test_str_cat_lit)
{
    token_vector v = tokenize_statement("\"a\" & \"b\"");
    TokenIterator t(v);
    data_map d;
    ExprParser p(t, d);
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "ab");
}
BOOST_AUTO_TEST_CASE(test_add)
{
    token_vector v = tokenize_statement("a + b");
    TokenIterator t(v);
    data_map d;
    d["a"] = 200;
    d["b"] = 100;
    ExprParser p(t, d);
    BOOST_CHECK_EQUAL(p.parse_expr()->getint(), 300);
    d["a"] = -50;
    t.reset();
    BOOST_CHECK_EQUAL(p.parse_expr()->getint(), 50);
}
BOOST_AUTO_TEST_CASE(test_sub)
{
    token_vector v = tokenize_statement("a - b");
    TokenIterator t(v);
    data_map d;
    d["a"] = 200;
    d["b"] = 100;
    ExprParser p(t, d);
    BOOST_CHECK_EQUAL(p.parse_expr()->getint(), 100);
    d["a"] = -50;
    t.reset();
    BOOST_CHECK_EQUAL(p.parse_expr()->getint(), -150);
}
BOOST_AUTO_TEST_CASE(test_mul)
{
    token_vector v = tokenize_statement("a * b");
    TokenIterator t(v);
    data_map d;
    d["a"] = 200;
    d["b"] = 100;
    ExprParser p(t, d);
    BOOST_CHECK_EQUAL(p.parse_expr()->getint(), 20000);
    d["a"] = -50;
    t.reset();
    BOOST_CHECK_EQUAL(p.parse_expr()->getint(), -5000);
}
BOOST_AUTO_TEST_CASE(test_div)
{
    token_vector v = tokenize_statement("a / b");
    TokenIterator t(v);
    data_map d;
    d["a"] = 200;
    d["b"] = 100;
    ExprParser p(t, d);
    BOOST_CHECK_EQUAL(p.parse_expr()->getint(), 2);
    d["b"] = -5;
    t.reset();
    BOOST_CHECK_EQUAL(p.parse_expr()->getint(), -40);
}
BOOST_AUTO_TEST_CASE(test_mod)
{
    token_vector v = tokenize_statement("a % b");
    TokenIterator t(v);
    data_map d;
    d["a"] = 12;
    d["b"] = 10;
    ExprParser p(t, d);
    BOOST_CHECK_EQUAL(p.parse_expr()->getint(), 2);
    d["a"] = 10;
    t.reset();
    BOOST_CHECK_EQUAL(p.parse_expr()->getint(), 0);
}
BOOST_AUTO_TEST_CASE(test_not)
{
    token_vector v = tokenize_statement("not a");
    TokenIterator t(v);
    data_map d;
    d["a"] = true;
    ExprParser p(t, d);
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "false");
    d["a"] = false;
    t.reset();
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "true");
}
BOOST_AUTO_TEST_CASE(test_unary_minus)
{
    token_vector v = tokenize_statement("-12");
    TokenIterator t(v);
    data_map d;
    ExprParser p(t, d);
    BOOST_CHECK_EQUAL(p.parse_expr()->getint(), -12);
}
BOOST_AUTO_TEST_CASE(test_unary_minus_var)
{
    token_vector v = tokenize_statement("-a");
    TokenIterator t(v);
    data_map d;
    d["a"] = 100;
    ExprParser p(t, d);
    BOOST_CHECK_EQUAL(p.parse_expr()->getint(), -100);
}
BOOST_AUTO_TEST_CASE(test_parens)
{
    token_vector v = tokenize_statement("(a)");
    TokenIterator t(v);
    data_map d;
    d["a"] = "xyzzy";
    ExprParser p(t, d);
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "xyzzy");
}
BOOST_AUTO_TEST_CASE(test_parens_or)
{
    token_vector v = tokenize_statement("(a || b) == z");
    TokenIterator t(v);
    data_map d;
    d["a"] = "";
    d["b"] = "foo";
    d["z"] = "xyzzy";
    ExprParser p(t, d);
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "false");
    d["z"] = "foo";
    t.reset();
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "true");
    d["a"] = "bar";
    t.reset();
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "false");
    d["z"] = "bar";
    t.reset();
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "true");
}
BOOST_AUTO_TEST_CASE(test_and_or)
{
    token_vector v = tokenize_statement("a && b || c");
    TokenIterator t(v);
    data_map d;
    d["a"] = true;
    d["b"] = true;
    d["c"] = false;
    ExprParser p(t, d);
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "true");
    d["a"] = false;
    t.reset();
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "false");
    d["c"] = true;
    t.reset();
    BOOST_CHECK_EQUAL(p.parse_expr()->getvalue(), "true");
}

BOOST_AUTO_TEST_SUITE_END()

// ------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(TestCppParser)

BOOST_AUTO_TEST_CASE(test_empty)
{
    string text = "";
    node_vector nodes;
    impl::TemplateParser(text, nodes).parse();

    BOOST_CHECK_EQUAL(0u, nodes.size());
}
BOOST_AUTO_TEST_CASE(test_text_only)
{
    string text = "blah blah blah";
    node_vector nodes;
    impl::TemplateParser(text, nodes).parse();
    data_map data;

    BOOST_CHECK_EQUAL(1u, nodes.size());
    BOOST_CHECK_EQUAL(gettext(nodes[0], data), "blah blah blah");
}
BOOST_AUTO_TEST_CASE(test_brackets_no_var)
{
    string text = "{foo}";
    node_vector nodes;
    impl::TemplateParser(text, nodes).parse();
    data_map data;

    BOOST_CHECK_EQUAL(2u, nodes.size());
    BOOST_CHECK_EQUAL(gettext(nodes[0], data), "{");
    BOOST_CHECK_EQUAL(gettext(nodes[1], data), "foo}");
}
BOOST_AUTO_TEST_CASE(test_ends_with_bracket)
{
    string text = "blah blah blah{";
    node_vector nodes;
    impl::TemplateParser(text, nodes).parse();
    data_map data;

    BOOST_CHECK_EQUAL(2u, nodes.size());
    BOOST_CHECK_EQUAL(gettext(nodes[0], data), "blah blah blah");
    BOOST_CHECK_EQUAL(gettext(nodes[1], data), "{");
}
// var
BOOST_AUTO_TEST_CASE(test_var)
{
    string text = "{$foo}";
    node_vector nodes;
    impl::TemplateParser(text, nodes).parse();
    data_map data;
    data["foo"] = make_data("bar");

    BOOST_CHECK_EQUAL(1u, nodes.size());
    BOOST_CHECK_EQUAL(gettext(nodes[0], data), "bar");
}
// for
BOOST_AUTO_TEST_CASE(test_for)
{
    string text = "{% for item in items %}";
    node_vector nodes;
    impl::TemplateParser(text, nodes).parse();

    BOOST_CHECK_EQUAL(1u, nodes.size());
    BOOST_CHECK_EQUAL(nodes[0]->gettype(), NODE_TYPE_FOR);
}
BOOST_AUTO_TEST_CASE(test_for_full)
{
    string text = "{% for item in items %}{$item}{% endfor %}";
    node_vector nodes;
    impl::TemplateParser(text, nodes).parse();

    BOOST_CHECK_EQUAL(1u, nodes.size());
    BOOST_CHECK_EQUAL(1u, nodes[0]->get_children().size());
    BOOST_CHECK_EQUAL(nodes[0]->gettype(), NODE_TYPE_FOR);
    BOOST_CHECK_EQUAL(nodes[0]->get_children()[0]->gettype(), NODE_TYPE_VAR);
}
BOOST_AUTO_TEST_CASE(test_for_full_with_text)
{
    string text = "{% for item in items %}*{$item}*{% endfor %}";
    node_vector nodes;
    impl::TemplateParser(text, nodes).parse();
    data_map data;
    data["item"] = make_data("my ax");

    BOOST_CHECK_EQUAL(1u, nodes.size());
    BOOST_CHECK_EQUAL(3u, nodes[0]->get_children().size());
    BOOST_CHECK_EQUAL(nodes[0]->gettype(), NODE_TYPE_FOR);
    BOOST_CHECK_EQUAL(gettext(nodes[0]->get_children()[0], data), "*");
    BOOST_CHECK_EQUAL(nodes[0]->get_children()[1]->gettype(), NODE_TYPE_VAR);
    BOOST_CHECK_EQUAL(gettext(nodes[0]->get_children()[1], data), "my ax");
    BOOST_CHECK_EQUAL(gettext(nodes[0]->get_children()[2], data), "*");
}
// if
BOOST_AUTO_TEST_CASE(test_if)
{
    string text = "{% if foo %}";
    node_vector nodes;
    impl::TemplateParser(text, nodes).parse();

    BOOST_CHECK_EQUAL(1u, nodes.size());
    BOOST_CHECK_EQUAL(nodes[0]->gettype(), NODE_TYPE_IF);
}
BOOST_AUTO_TEST_CASE(test_if_full)
{
    string text = "{% if item %}{$item}{% endif %}";
    node_vector nodes;
    impl::TemplateParser(text, nodes).parse();

    BOOST_CHECK_EQUAL(1u, nodes.size());
    BOOST_CHECK_EQUAL(1u, nodes[0]->get_children().size());
    BOOST_CHECK_EQUAL(nodes[0]->gettype(), NODE_TYPE_IF);
    BOOST_CHECK_EQUAL(nodes[0]->get_children()[0]->gettype(), NODE_TYPE_VAR);
}
BOOST_AUTO_TEST_CASE(test_if_full_with_text)
{
    string text = "{% if item %}{{$item}}{% endif %}";
    node_vector nodes;
    impl::TemplateParser(text, nodes).parse();
    data_map data;
    data["item"] = make_data("my ax");

    BOOST_CHECK_EQUAL(1u, nodes.size());
    BOOST_CHECK_EQUAL(3u, nodes[0]->get_children().size());
    BOOST_CHECK_EQUAL(nodes[0]->gettype(), NODE_TYPE_IF);
    BOOST_CHECK_EQUAL(gettext(nodes[0]->get_children()[0], data), "{");
    BOOST_CHECK_EQUAL(nodes[0]->get_children()[1]->gettype(), NODE_TYPE_VAR);
    BOOST_CHECK_EQUAL(gettext(nodes[0]->get_children()[1], data), "my ax");
    BOOST_CHECK_EQUAL(gettext(nodes[0]->get_children()[2], data), "}");
}

BOOST_AUTO_TEST_SUITE_END()

// ------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(TestCppTemplateEval)

BOOST_AUTO_TEST_CASE(test_empty)
{
    string text = "";
    data_map data;
    string actual = parse(text, data);
    string expected = "";
    BOOST_CHECK_EQUAL(expected, actual);
}
BOOST_AUTO_TEST_CASE(test_no_vars)
{
    string text = "foo";
    data_map data;
    string actual = parse(text, data);
    string expected = "foo";
    BOOST_CHECK_EQUAL(expected, actual);
}
BOOST_AUTO_TEST_CASE(test_var)
{
    string text = "{$foo}";
    data_map data;
    data["foo"] = make_data("bar");
    string actual = parse(text, data);
    string expected = "bar";
    BOOST_CHECK_EQUAL(expected, actual);
}
BOOST_AUTO_TEST_CASE(test_var_surrounded)
{
    string text = "aaa{$foo}bbb";
    data_map data;
    data["foo"] = make_data("---");
    string actual = parse(text, data);
    string expected = "aaa---bbb";
    BOOST_CHECK_EQUAL(expected, actual);
}
BOOST_AUTO_TEST_CASE(test_example_okinawa)
{
    // The text template
    string text = "I heart {$place}!";
    // Data to feed the template engine
    cpptempl::data_map data;
    // {$place} => Okinawa
    data["place"] = cpptempl::make_data("Okinawa");
    // parse the template with the supplied data dictionary
    string result = cpptempl::parse(text, data);

    string expected = "I heart Okinawa!";
    BOOST_CHECK_EQUAL(result, expected);
}

BOOST_AUTO_TEST_SUITE_END()

// ------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(TestCppTemplateMisc)

BOOST_AUTO_TEST_CASE(test_empty_stmt)
{
    string text = "aaa{%  %}bbb{$item}ccc";
    data_map data;
    data["item"] = "xyz";
    BOOST_CHECK_EQUAL(parse(text, data), "aaabbbxyzccc");
}
BOOST_AUTO_TEST_CASE(test_comment_only_stmt)
{
    string text = "aaa{% -- a comment! %}bbb{$item -- a comment on a var}ccc";
    data_map data;
    data["item"] = "xyz";
    BOOST_CHECK_EQUAL(parse(text, data), "aaabbbxyzccc");
}
BOOST_AUTO_TEST_CASE(test_newline_elision)
{
    string text =
        "{% if predicate >%}\n"
        "{$item >}\n"
        "{% endif %}";
    data_map data;
    data["predicate"] = true;
    data["item"] = "foo";
    BOOST_CHECK_EQUAL(parse(text, data), "foo");
}
BOOST_AUTO_TEST_CASE(test_newline_eater)
{
    string text =
        "hello{%>%}\n"
        " world";
    data_map data;
    BOOST_CHECK_EQUAL(parse(text, data), "hello world");
}
BOOST_AUTO_TEST_CASE(test_newline_eater_with_comment)
{
    string text =
        "hello{% -- get rid of following newline >%}\n"
        " world";
    data_map data;
    BOOST_CHECK_EQUAL(parse(text, data), "hello world");
}

BOOST_AUTO_TEST_SUITE_END()

// ------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(TestCppTemplateIf)

BOOST_AUTO_TEST_CASE(test_if_false)
{
    string text = "{% if predicate %}{$item}{% endif %}";
    data_map data;
    data["predicate"] = false;
    data["item"] = make_data("foo");
    string actual = parse(text, data);
    string expected = "";
    BOOST_CHECK_EQUAL(expected, actual);
}
BOOST_AUTO_TEST_CASE(test_if_true)
{
    string text = "{% if predicate %}{$item}{% endif %}";
    data_map data;
    data["predicate"] = true;
    data["item"] = make_data("foo");
    string actual = parse(text, data);
    string expected = "foo";
    BOOST_CHECK_EQUAL(expected, actual);
}
BOOST_AUTO_TEST_CASE(test_if_comments)
{
    string text = "{% if item -- an if statement %}{$item}{% endif -- end of if %}";
    data_map data;
    data["item"] = make_data("foo");
    string actual = parse(text, data);
    string expected = "foo";
    BOOST_CHECK_EQUAL(expected, actual);
}
BOOST_AUTO_TEST_CASE(test_nested_if_false)
{
    string text = "{% if item %}{% if thing %}{$item}{$thing}{% endif %}{% endif %}";
    data_map data;
    data["item"] = make_data("aaa");
    data["thing"] = make_data("");
    string actual = parse(text, data);
    string expected = "";
    BOOST_CHECK_EQUAL(expected, actual);
}
BOOST_AUTO_TEST_CASE(test_nested_if_true)
{
    string text = "{% if item %}{% if thing %}{$item}{$thing}{% endif %}{% endif %}";
    data_map data;
    data["item"] = make_data("aaa");
    data["thing"] = make_data("bbb");
    string actual = parse(text, data);
    string expected = "aaabbb";
    BOOST_CHECK_EQUAL(expected, actual);
}
BOOST_AUTO_TEST_CASE(test_usage_example)
{
    string text =
        "{% if item %}{$item}{% endif %}\n"
        "{% if thing %}{$thing}{% endif %}";
    cpptempl::data_map data;
    data["item"] = cpptempl::make_data("aaa");
    data["thing"] = cpptempl::make_data("bbb");

    string result = cpptempl::parse(text, data);

    string expected = "aaa\nbbb";
    BOOST_CHECK_EQUAL(result, expected);
}
BOOST_AUTO_TEST_CASE(test_syntax_if)
{
    string text = "{% if person.name == \"Bob\" %}Full name: Robert{% endif %}";
    data_map person;
    person["name"] = make_data("Bob");
    person["occupation"] = make_data("Plumber");
    data_map data;
    data["person"] = make_data(person);

    string result = cpptempl::parse(text, data);

    string expected = "Full name: Robert";
    BOOST_CHECK_EQUAL(result, expected);
}
BOOST_AUTO_TEST_CASE(test_example_if_else)
{
    // The text template
    string text = "{% if foo %}yes{% else %}no{% endif %}";
    // Data to feed the template engine
    cpptempl::data_map data;
    data["foo"] = true;
    // parse the template with the supplied data dictionary
    BOOST_CHECK_EQUAL(cpptempl::parse(text, data), "yes");

    data["foo"] = false;
    BOOST_CHECK_EQUAL(cpptempl::parse(text, data), "no");
}
BOOST_AUTO_TEST_CASE(test_example_if_elif)
{
    // The text template
    string text = "{% if foo %}aa{% elif bar %}bb{% endif %}";
    // Data to feed the template engine
    cpptempl::data_map data;
    data["foo"] = true;
    data["bar"] = false;
    // parse the template with the supplied data dictionary
    BOOST_CHECK_EQUAL(cpptempl::parse(text, data), "aa");

    data["foo"] = false;
    BOOST_CHECK_EQUAL(cpptempl::parse(text, data), "");

    data["bar"] = true;
    BOOST_CHECK_EQUAL(cpptempl::parse(text, data), "bb");
}
BOOST_AUTO_TEST_CASE(test_example_if_elif_else)
{
    // The text template
    string text = "{% if foo %}aa{% elif bar %}bb{% else %}cc{% endif %}";
    // Data to feed the template engine
    cpptempl::data_map data;
    data["foo"] = true;
    data["bar"] = false;
    // parse the template with the supplied data dictionary
    BOOST_CHECK_EQUAL(cpptempl::parse(text, data), "aa");

    data["foo"] = false;
    BOOST_CHECK_EQUAL(cpptempl::parse(text, data), "cc");

    data["bar"] = true;
    BOOST_CHECK_EQUAL(cpptempl::parse(text, data), "bb");
}
BOOST_AUTO_TEST_CASE(test_example_if_elif_x2)
{
    // The text template
    string text = "{% if foo %}aa{% elif bar %}bb{% elif baz %}cc{% endif %}";
    // Data to feed the template engine
    cpptempl::data_map data;
    data["foo"] = false;
    data["bar"] = false;
    data["baz"] = false;
    // parse the template with the supplied data dictionary
    BOOST_CHECK_EQUAL(cpptempl::parse(text, data), "");

    data["foo"] = true;
    BOOST_CHECK_EQUAL(cpptempl::parse(text, data), "aa");

    data["foo"] = false;
    data["baz"] = true;
    BOOST_CHECK_EQUAL(cpptempl::parse(text, data), "cc");

    data["bar"] = true;
    BOOST_CHECK_EQUAL(cpptempl::parse(text, data), "bb");
}

BOOST_AUTO_TEST_SUITE_END()

// ------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(TestCppTemplateFor)

BOOST_AUTO_TEST_CASE(test_for)
{
    string text = "{% for item in items %}{$item}{% endfor %}";
    data_map data;
    data_list items;
    items.push_back(make_data("0"));
    items.push_back(make_data("1"));
    data["items"] = make_data(items);
    string actual = parse(text, data);
    string expected = "01";
    BOOST_CHECK_EQUAL(expected, actual);
}
BOOST_AUTO_TEST_CASE(test_for_loop_vars)
{
    string text =
        "{% for item in items %}\n"
        "index={$loop.index}; index0={$loop.index0}; first={$loop.first}; last={$loop.last}; "
        "even={$loop.even}; odd={$loop.odd}; count={$loop.count}; "
        "addNewLineIfNotLast={$loop.addNewLineIfNotLast}"
        "{% endfor %}";
    data_map data;
    data_list items;
    items.push_back(make_data(1));
    items.push_back(make_data(2));
    items.push_back(make_data(3));
    data["items"] = make_data(items);
    string actual = parse(text, data);
    string expected =
        "index=1; index0=0; first=true; last=false; even=false; odd=true; count=3; addNewLineIfNotLast=\n"
        "index=2; index0=1; first=false; last=false; even=true; odd=false; count=3; addNewLineIfNotLast=\n"
        "index=3; index0=2; first=false; last=true; even=false; odd=true; count=3; addNewLineIfNotLast=";
    BOOST_CHECK_EQUAL(expected, actual);
}
BOOST_AUTO_TEST_CASE(test_nested_for)
{
    string text = "{% for item in items %}{% for thing in things %}{$item}{$thing}{% endfor %}{% endfor %}";
    data_map data;
    data_list items;
    items.push_back(make_data("0"));
    items.push_back(make_data("1"));
    data["items"] = make_data(items);
    data_list things;
    things.push_back(make_data("a"));
    things.push_back(make_data("b"));
    data["things"] = make_data(things);
    string actual = parse(text, data);
    string expected = "0a0b1a1b";
    BOOST_CHECK_EQUAL(expected, actual);
}
BOOST_AUTO_TEST_CASE(test_syntax_dotted)
{
    string text =
        "{% for friend in person.friends %}"
        "{$loop.index}. {$friend.name} "
        "{% endfor %}";

    data_map bob;
    bob["name"] = make_data("Bob");
    data_map betty;
    betty["name"] = make_data("Betty");
    data_list friends;
    friends.push_back(make_data(bob));
    friends.push_back(make_data(betty));
    data_map person;
    person["friends"] = make_data(friends);
    data_map data;
    data["person"] = make_data(person);

    string result = cpptempl::parse(text, data);

    string expected = "1. Bob 2. Betty ";
    BOOST_CHECK_EQUAL(result, expected);
}
BOOST_AUTO_TEST_CASE(test_example_ul)
{
    string text =
        "<h3>Locations</h3><ul>"
        "{% for place in places %}"
        "<li>{$place}</li>"
        "{% endfor %}"
        "</ul>";

    // Create the list of items
    cpptempl::data_list places;
    places.push_back(cpptempl::make_data("Okinawa"));
    places.push_back(cpptempl::make_data("San Francisco"));
    // Now set this in the data map
    cpptempl::data_map data;
    data["places"] = cpptempl::make_data(places);
    // parse the template with the supplied data dictionary
    string result = cpptempl::parse(text, data);
    string expected =
        "<h3>Locations</h3><ul>"
        "<li>Okinawa</li>"
        "<li>San Francisco</li>"
        "</ul>";
    BOOST_CHECK_EQUAL(result, expected);
}
BOOST_AUTO_TEST_CASE(test_example_for_loop_var_restore)
{
    // The text template
    string text =
        "{% for x in items %}.{$loop.index}"
        "{% for y in more %}:{$loop.index}{% endfor %}"
        "-{$loop.index}{% endfor %}";
    // Data to feed the template engine
    data_list items;
    items.push_back("a");
    items.push_back("b");
    data_list more;
    more.push_back("1");
    more.push_back("2");
    more.push_back("3");
    cpptempl::data_map data;
    data["items"] = items;
    data["more"] = more;

    // parse the template with the supplied data dictionary
    BOOST_CHECK_EQUAL(cpptempl::parse(text, data), ".1:1:2:3-1.2:1:2:3-2");
}

BOOST_AUTO_TEST_SUITE_END()

// ------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(TestCppTemplateDef)

BOOST_AUTO_TEST_CASE(test_example_def)
{
    // The text template
    string text = "{% def foo %}hello world{% enddef %}{$foo}";
    // Data to feed the template engine
    cpptempl::data_map data;
    // parse the template with the supplied data dictionary
    BOOST_CHECK_EQUAL(cpptempl::parse(text, data), "hello world");
}
BOOST_AUTO_TEST_CASE(test_example_def_param)
{
    // The text template
    string text = "{% def foo(p) %}hello {$p} world{% enddef %}{$foo('happy')}|{$foo('sad')}";
    // Data to feed the template engine
    cpptempl::data_map data;
    // parse the template with the supplied data dictionary
    BOOST_CHECK_EQUAL(cpptempl::parse(text, data), "hello happy world|hello sad world");
}
BOOST_AUTO_TEST_CASE(test_example_multi_def)
{
    string text = "{% def foo(place) %}hello {$place}{% enddef %}";
    data_map data;
    parse(text, data);
    string text2 = "{$foo('world')}";
    BOOST_CHECK_EQUAL(parse(text2, data), "hello world");
}
BOOST_AUTO_TEST_CASE(test_example_multi_def_2)
{
    string text = "{% def foo(place) %}hello {$place}{% enddef %}";
    data_map data;
    parse(text, data);
    {
        data_map items;
        items["bar"] = data["foo"];
        data["items"] = items;
    }
    string text2 = "{$items.bar('world')}";
    BOOST_CHECK_EQUAL(parse(text2, data), "hello world");
}

data_map get_things_map()
{
    data_map data;
    data_list mems;
    data_map x;
    x["name"] = "A";
    x["value"] = "1";
    mems.push_back(x);
    data_map y;
    y["name"] = "B";
    y["value"] = "2";
    mems.push_back(y);
    data_map e;
    e["members"] = mems;
    e["name"] = "letters";
    data_list mems2;
    data_map z;
    z["name"] = "Q";
    z["value"] = "10";
    mems2.push_back(z);
    data_map f;
    f["members"] = mems2;
    f["name"] = "fun";
    data_list es;
    es.push_back(e);
    es.push_back(f);
    data["things"] = es;
    return data;
}
BOOST_AUTO_TEST_CASE(test_example_multi_def_inner_for)
{
    string text =
        "{% def defwithloop(info) %}"
        "({$info.name}:"
        "{% for thing in info.members %}"
        "[{$thing.name}{% if thing.value %}={$thing.value}{% endif%}]"
        "{% endfor %})"
        "{% enddef %}"
        "{% for x in things %}"
        "{$defwithloop(x)}"
        "{% endfor %}";
    data_map data = get_things_map();

    BOOST_CHECK_EQUAL(parse(text, data), "(letters:[A=1][B=2])(fun:[Q=10])");
}
BOOST_AUTO_TEST_CASE(test_example_multi_def_call_def)
{
    string text =
        "{% def outerdef(info) %}({$info.name}:{$defwithloop(info)}){% enddef %}"
        "{% def defwithloop(info) %}"
        "{% for thing in info.members %}"
        "[{$thing.name}{% if thing.value %}={$thing.value}{% endif%}]"
        "{% endfor %}"
        "{% enddef %}"
        "{% for x in things %}"
        "{$outerdef(x)}"
        "{% endfor %}";
    data_map data = get_things_map();

    BOOST_CHECK_EQUAL(parse(text, data), "(letters:[A=1][B=2])(fun:[Q=10])");
}

BOOST_AUTO_TEST_SUITE_END()

// According to the docs this main() should be provided by the boost unit test lib,
// but it wasn't linking until I added it.
int main(int argc, char *argv[])
{
    return boost::unit_test::unit_test_main(init_unit_test, argc, argv);
}
