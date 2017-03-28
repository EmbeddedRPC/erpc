// Copyright (c) 2010-2014 Ryan Ginstrom
// Copyright (c) 2014 Martinho Fernandes
// Copyright (c) 2014-2016 Freescale Semiconductor, Inc.
// Copyright 2016 NXP
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

#ifdef _MSC_VER
#include "stdafx.h"
#endif

#include "cpptempl.h"
#include <cctype>
#include <sstream>
#include <algorithm>
#include <stack>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <cassert>
#include <cstdlib>

namespace cpptempl
{
namespace impl
{
// Types of tokens in control statements.
enum TokenType
{
    INVALID_TOKEN,
    KEY_PATH_TOKEN,
    STRING_LITERAL_TOKEN,
    INT_LITERAL_TOKEN,
    TRUE_TOKEN,
    FALSE_TOKEN,
    FOR_TOKEN,
    IN_TOKEN,
    IF_TOKEN,
    ELIF_TOKEN,
    ELSE_TOKEN,
    DEF_TOKEN,
    SET_TOKEN,
    ENDFOR_TOKEN,
    ENDIF_TOKEN,
    ENDDEF_TOKEN,
    AND_TOKEN,
    OR_TOKEN,
    NOT_TOKEN,
    EQ_TOKEN,
    NEQ_TOKEN,
    GE_TOKEN,
    LE_TOKEN,
    GT_TOKEN = '>',
    LT_TOKEN = '<',
    PLUS_TOKEN = '+',
    MINUS_TOKEN = '-',
    TIMES_TOKEN = '*',
    DIVIDE_TOKEN = '/',
    MOD_TOKEN = '%',
    CONCAT_TOKEN = '&',
    ASSIGN_TOKEN = '=',
    OPEN_PAREN_TOKEN = '(',
    CLOSE_PAREN_TOKEN = ')',
    COMMA_TOKEN = ',',
    END_TOKEN = 255
};

// Represents a token in a control statement.
class Token
{
    TokenType m_type;
    std::string m_value;

public:
    Token(TokenType tokenType)
    : m_type(tokenType)
    , m_value()
    {
    }
    Token(TokenType tokenType, const std::string &value)
    : m_type(tokenType)
    , m_value(value)
    {
    }
    Token(const Token &other)
    : m_type(other.m_type)
    , m_value(other.m_value)
    {
    }
    Token(Token &&other)
    : m_type(other.m_type)
    , m_value(std::move(other.m_value))
    {
    }
    Token &operator=(const Token &other) = default;
    Token &operator=(Token &&other)
    {
        m_type = other.m_type;
        m_value.assign(std::move(other.m_value));
        return *this;
    }
    ~Token() = default;

    TokenType get_type() const { return m_type; }
    const std::string &get_value() const { return m_value; }
};

typedef std::vector<Token> token_vector;

// Helper class for parsing tokens.
class TokenIterator
{
    const token_vector &m_tokens;
    size_t m_index;
    static Token s_endToken;

public:
    TokenIterator(const token_vector &seq)
    : m_tokens(seq)
    , m_index(0)
    {
    }
    TokenIterator(const TokenIterator &other)
    : m_tokens(other.m_tokens)
    , m_index(other.m_index)
    {
    }

    size_t size() const { return m_tokens.size(); }
    bool empty() const { return size() == 0; }
    bool is_valid() const { return m_index < size(); }
    const Token *get() const;
    const Token *next();
    const Token *match(TokenType tokenType, const char *failure_msg = nullptr);
    void reset() { m_index = 0; }
    TokenIterator &operator++();

    const Token *operator->() const { return get(); }
    const Token &operator*() const { return *get(); }
};

class ExprParser
{
    TokenIterator &m_tok;
    data_map &m_data;

public:
    ExprParser(TokenIterator &seq, data_map &data)
    : m_tok(seq)
    , m_data(data)
    {
    }

    data_ptr parse_expr();
    data_ptr parse_oterm();
    data_ptr parse_bterm();
    data_ptr parse_bfactor();
    data_ptr parse_gfactor();
    data_ptr parse_afactor();
    data_ptr parse_mfactor();
    data_ptr parse_factor();
    data_ptr get_var_value(const std::string &path, data_list &params);
};

typedef enum
{
    NODE_TYPE_NONE,
    NODE_TYPE_TEXT,
    NODE_TYPE_VAR,
    NODE_TYPE_IF,
    NODE_TYPE_ELIF,
    NODE_TYPE_ELSE,
    NODE_TYPE_FOR,
    NODE_TYPE_DEF,
    NODE_TYPE_SET,
} NodeType;

// Template nodes
// base class for all node types
class Node
{
    uint32_t m_line;

public:
    Node(uint32_t line)
    : m_line(line)
    {
    }
    virtual NodeType gettype() = 0;
    virtual void gettext(std::ostream &stream, data_map &data) = 0;
    virtual void set_children(node_vector &children);
    virtual node_vector &get_children();
    uint32_t get_line() { return m_line; }
    void set_line(uint32_t line) { m_line = line; }
};

// node with children
class NodeParent : public Node
{
protected:
    node_vector m_children;

public:
    NodeParent(uint32_t line)
    : Node(line)
    {
    }
    void set_children(node_vector &children);
    node_vector &get_children();
};

// normal text
class NodeText : public Node
{
    std::string m_text;

public:
    NodeText(std::string text, uint32_t line = 0)
    : Node(line)
    , m_text(text)
    {
    }
    NodeType gettype();
    void gettext(std::ostream &stream, data_map &data);
};

// variable
class NodeVar : public Node
{
    token_vector m_expr;
    bool m_removeNewLine;

public:
    NodeVar(const token_vector &expr, uint32_t line = 0, bool removeNewLine = false)
    : Node(line)
    , m_expr(expr)
    , m_removeNewLine(removeNewLine)
    {
    }
    NodeType gettype();
    void gettext(std::ostream &stream, data_map &data);
};

// for block
class NodeFor : public NodeParent
{
    std::string m_key;
    std::string m_val;
    bool m_is_top;
    bool m_has_predicate;
    token_vector m_predicate_tokens;

public:
    NodeFor(const token_vector &tokens, bool is_top, uint32_t line = 0);
    NodeType gettype();
    void gettext(std::ostream &stream, data_map &data);
    data_map build_loop_map(size_t i, size_t count);
};

// if block
class NodeIf : public NodeParent
{
    token_vector m_expr;
    node_ptr m_else_if;
    NodeType m_if_type;

public:
    NodeIf(const token_vector &expr, uint32_t line = 0);
    NodeType gettype();
    void set_else_if(node_ptr else_if);
    void gettext(std::ostream &stream, data_map &data);
    bool is_true(data_map &data);
    bool is_else();
};

// def block
class NodeDef : public NodeParent
{
    std::string m_name;
    string_vector m_params;

public:
    NodeDef(const token_vector &expr, uint32_t line = 0);
    NodeType gettype();
    void gettext(std::ostream &stream, data_map &data);
};

// set variable
class NodeSet : public Node
{
    token_vector m_expr;

public:
    NodeSet(const token_vector &expr, uint32_t line = 0)
    : Node(line)
    , m_expr(expr)
    {
    }
    NodeType gettype();
    void gettext(std::ostream &stream, data_map &data);
};

// Lexer states for statement tokenizer.
enum lexer_state_t
{
    INIT_STATE,           //!< Initial state, skip whitespace, process single char tokens.
    KEY_PATH_STATE,       //!< Scan a key path.
    STRING_LITERAL_STATE, //!< Scan a string literal.
    INT_LITERAL_STATE,    //!< Scan an integer literal.
    COMMENT_STATE,        //!< Single-line comment.
};

struct KeywordDef
{
    TokenType tok;
    const char *keyword;
};

class TemplateParser
{
    std::string m_text;
    node_vector &m_top_nodes;
    uint32_t m_current_line;
    std::stack<std::pair<node_ptr, TokenType> > m_node_stack;
    node_ptr m_current_node;
    node_vector *m_current_nodes;
    bool m_eol_precedes;
    bool m_last_was_eol;
    TokenType m_until;

public:
    TemplateParser(const std::string &text, node_vector &nodes);

    node_vector &parse();

private:
    void parse_var();
    void parse_stmt();
    void parse_comment();

    void push_node(Node *new_node, TokenType until);
    void check_omit_eol(size_t pos, bool force_omit);
};

std::string indent(int level);
inline bool is_key_path_char(char c);
TokenType get_keyword_token(const std::string &s);
void create_id_token(token_vector &tokens, const std::string &s);
int append_string_escape(std::string &str, std::function<char(unsigned)> peek);
token_vector tokenize_statement(const std::string &text);
inline size_t count_newlines(const std::string &text);
}

// This allows inclusion of the cpptempl::impl declarations into the unit test
// without also bringing in the implementation.
#if defined(CPPTEMPL_UNIT_TEST)
}
#else

//! Flag to track whether to remove the next newline that appears in the output.
//!
//! @bug This global breaks reentrancy.
static bool s_removeNewLine;

//////////////////////////////////////////////////////////////////////////
// Data classes
//////////////////////////////////////////////////////////////////////////

// These ctors are defined here to resolve definition ordering issues with clang.
data_ptr::data_ptr(DataBool *data)
: ptr(data)
{
}
data_ptr::data_ptr(DataInt *data)
: ptr(data)
{
}
data_ptr::data_ptr(DataValue *data)
: ptr(data)
{
}
data_ptr::data_ptr(DataList *data)
: ptr(data)
{
}
data_ptr::data_ptr(DataMap *data)
: ptr(data)
{
}
data_ptr::data_ptr(DataTemplate *data)
: ptr(data)
{
}

// data_map
data_ptr &data_map::operator[](const std::string &key)
{
    if (data.find(key) == data.end() && parent)
    {
        return (*parent)[key];
    }
    return data[key];
}
bool data_map::empty()
{
    return data.empty();
}
bool data_map::has(const std::string &key)
{
    bool local_has = data.find(key) != data.end();
    return !local_has && parent ? parent->has(key) : local_has;
}

// data_ptr
template <>
void data_ptr::operator=(const bool &data)
{
    ptr.reset(new DataBool(data));
}

template <>
void data_ptr::operator=(const int &data)
{
    ptr.reset(new DataInt(data));
}

template <>
void data_ptr::operator=(const unsigned int &data)
{
    ptr.reset(new DataInt(data));
}

template <>
void data_ptr::operator=(const std::string &data)
{
    ptr.reset(new DataValue(data));
}

template <>
void data_ptr::operator=(const data_map &data)
{
    ptr.reset(new DataMap(data));
}

template <>
void data_ptr::operator=(const data_list &data)
{
    ptr.reset(new DataList(data));
}

data_ptr &data_ptr::operator=(std::string &&data)
{
    ptr.reset(new DataValue(std::move(data)));
    return *this;
}

data_ptr &data_ptr::operator=(data_map &&data)
{
    ptr.reset(new DataMap(std::move(data)));
    return *this;
}

data_ptr &data_ptr::operator=(data_list &&data)
{
    ptr.reset(new DataList(std::move(data)));
    return *this;
}

void data_ptr::push_back(const data_ptr &data)
{
    if (!ptr)
    {
        ptr.reset(new DataList(data_list()));
    }
    data_list &list = ptr->getlist();
    list.push_back(data);
}

// base data
std::string Data::getvalue()
{
    throw TemplateException("Data item is not a value");
}

data_list &Data::getlist()
{
    throw TemplateException("Data item is not a list");
}
data_map &Data::getmap()
{
    throw TemplateException("Data item is not a dictionary");
}
int Data::getint() const
{
    return 0;
}
// data bool
std::string DataBool::getvalue()
{
    return m_value ? "true" : "false";
}
bool DataBool::empty()
{
    return !m_value;
}
void DataBool::dump(int indent)
{
    std::cout << "(bool)" << getvalue() << std::endl;
}
int DataBool::getint() const
{
    return static_cast<int>(m_value);
}
// data int
std::string DataInt::getvalue()
{
    std::ostringstream stream;
    stream << m_value;
    return stream.str();
}
bool DataInt::empty()
{
    return !m_value;
}
void DataInt::dump(int indent)
{
    std::cout << "(int)" << m_value << std::endl;
}
int DataInt::getint() const
{
    return m_value;
}
// data value
std::string DataValue::getvalue()
{
    return m_value;
}
bool DataValue::empty()
{
    return m_value.empty();
}
int DataValue::getint() const
{
    return static_cast<int>(std::strtol(m_value.c_str(), NULL, 0));
}
void DataValue::dump(int indent)
{
    std::string text = boost::algorithm::replace_all_copy(getvalue(), "\n", "\\n");
    std::cout << "\"" << text << "\"" << std::endl;
}
// data list
data_list &DataList::getlist()
{
    return m_items;
}

bool DataList::empty()
{
    return m_items.empty();
}

void DataList::dump(int indent)
{
    std::cout << "(list)" << std::endl;
    int n = 0;
    for (auto it : m_items)
    {
        std::cout << impl::indent(indent) << n << ": ";
        it->dump(indent + 1);
        ++n;
    };
}
// data map
data_map &DataMap::getmap()
{
    return m_items;
}
bool DataMap::empty()
{
    return m_items.empty();
}
void DataMap::dump(int indent)
{
    std::cout << "(map)" << std::endl;
    for (auto it : m_items.data)
    {
        std::cout << impl::indent(indent) << it.first << ": ";
        it.second->dump(indent + 1);
    };
}

// data template
DataTemplate::DataTemplate(const std::string &templateText)
{
    // Parse the template
    impl::TemplateParser(templateText, m_tree).parse();
}

std::string DataTemplate::getvalue()
{
    return "";
}

bool DataTemplate::empty()
{
    return false;
}

void DataTemplate::dump(int indent)
{
    std::cout << "(template)\n";
}

std::string DataTemplate::eval(data_map &data, data_list *param_values)
{
    std::ostringstream stream;
    eval(stream, data, param_values);
    return stream.str();
}

void DataTemplate::eval(std::ostream &stream, data_map &data, data_list *param_values)
{
    data_map *use_data = &data;

    // Build map of param names to provided param values. The params map's
    // parent is set to the main data_map. This will cause params to
    // override keys in the main map, without modifying the main map.
    data_map params_map;
    if (param_values)
    {
        // Check number of params.
        if (param_values->size() > m_params.size())
        {
            throw TemplateException("too many parameter(s) provided to subtemplate");
        }

        for (size_t i = 0; i < std::min(m_params.size(), param_values->size()); ++i)
        {
            const std::string &key = m_params[i];
            data_ptr &value = (*param_values)[i];
            params_map[key] = value;
        }

        params_map.set_parent(&data);
        use_data = &params_map;
    }

    // Recursively calls gettext on each node in the tree.
    // gettext returns the appropriate text for that node.
    for (auto node : m_tree)
    {
        node->gettext(stream, *use_data);
    }
}

bool data_ptr::is_template() const
{
    return (dynamic_cast<DataTemplate *>(ptr.get()) != nullptr);
}

TemplateException::TemplateException(size_t line, std::string reason)
: std::exception()
, m_line(0)
, m_reason(reason)
{
    set_line_if_missing(line);
}

void TemplateException::set_line_if_missing(size_t line)
{
    if (!m_line)
    {
        m_line = line;
        m_reason = std::string("Line ") + boost::lexical_cast<std::string>(line) + ": " + m_reason;
    }
}

//////////////////////////////////////////////////////////////////////////
// parse_path
//////////////////////////////////////////////////////////////////////////
data_ptr &data_map::parse_path(const std::string &key, bool create)
{
    if (key.empty())
    {
        throw key_error("empty map key");
    }

    // check for dotted notation, i.e [foo.bar]
    size_t index = key.find(".");
    if (index == std::string::npos)
    {
        if (!has(key))
        {
            if (!create)
            {
                printf("invalid map key: %s\n", key.c_str());
                throw key_error("invalid map key");
            }
            data[key] = make_data("");
        }
        return operator[](key);
    }

    std::string sub_key = key.substr(0, index);
    if (!has(sub_key))
    {
        printf("invalid map key: %s\n", sub_key.c_str());
        throw key_error("invalid map key");
    }

    return operator[](sub_key)->getmap().parse_path(key.substr(index + 1), create);
}

void dump_data(data_ptr data)
{
    data->dump();
}

namespace impl
{
std::string indent(int level)
{
    std::string result;
    while (level--)
    {
        result += "    ";
    }
    return result;
}

Token TokenIterator::s_endToken(END_TOKEN);

const Token *TokenIterator::get() const
{
    if (is_valid())
    {
        return &m_tokens[m_index];
    }
    else
    {
        return &s_endToken;
    }
}

const Token *TokenIterator::next()
{
    if (m_index < size())
    {
        ++m_index;
    }
    return get();
}

const Token *TokenIterator::match(TokenType tokenType, const char *failure_msg)
{
    const Token *result = get();
    if (result->get_type() != tokenType)
    {
        throw TemplateException(failure_msg ? failure_msg : "unexpected token");
    }
    next();
    return result;
}

TokenIterator &TokenIterator::operator++()
{
    next();
    return *this;
}

inline bool is_key_path_char(char c)
{
    return (isalnum(c) || c == '.' || c == '_');
}

const KeywordDef k_keywords[] = { { TRUE_TOKEN, "true" },     { FALSE_TOKEN, "false" }, { FOR_TOKEN, "for" },
                                  { IN_TOKEN, "in" },         { IF_TOKEN, "if" },       { ELIF_TOKEN, "elif" },
                                  { ELSE_TOKEN, "else" },     { DEF_TOKEN, "def" },     { SET_TOKEN, "set" },
                                  { ENDFOR_TOKEN, "endfor" }, { ENDIF_TOKEN, "endif" }, { ENDDEF_TOKEN, "enddef" },
                                  { AND_TOKEN, "and" },       { OR_TOKEN, "or" },       { NOT_TOKEN, "not" },
                                  { INVALID_TOKEN } };

TokenType get_keyword_token(const std::string &s)
{
    const KeywordDef *k = k_keywords;
    for (; k->tok != INVALID_TOKEN; ++k)
    {
        if (s == k->keyword)
        {
            return k->tok;
        }
    }
    return INVALID_TOKEN;
}

void create_id_token(token_vector &tokens, const std::string &s)
{
    TokenType t = get_keyword_token(s);
    if (t == INVALID_TOKEN)
    {
        // Create key path token.
        tokens.emplace_back(KEY_PATH_TOKEN, s);
    }
    else
    {
        tokens.emplace_back(t);
    }
}

int append_string_escape(std::string &str, std::function<char(unsigned)> peek)
{
    char esc = peek(1);
    std::string hex;
    int n = 2;
    switch (esc)
    {
        // standard C escape sequences
        case 'a':
            esc = '\a';
            break;
        case 'b':
            esc = '\b';
            break;
        case 'f':
            esc = '\f';
            break;
        case 'n':
            esc = '\n';
            break;
        case 'r':
            esc = '\r';
            break;
        case 't':
            esc = '\t';
            break;
        case 'v':
            esc = '\v';
            break;
        case '0':
            esc = '\0';
            break;
        // handle hex escapes like \x2a
        case 'x':
            for (; std::isxdigit(peek(n)); ++n)
            {
                hex += peek(n);
            }
            esc = static_cast<char>(hex.empty() ? 0 : std::strtoul(hex.c_str(), nullptr, 16));
            break;
    }
    str += esc;
    return n - 1;
}

token_vector tokenize_statement(const std::string &text)
{
    token_vector tokens;
    lexer_state_t state = INIT_STATE;
    unsigned i = 0;
    uint32_t pos = 0;
    char str_open_quote = 0;
    std::string literal;
    bool is_hex_literal = false;

    // closure to get the next char without advancing
    auto peek = [&](unsigned n)
    {
        return i + n < text.size() ? text[i + n] : 0;
    };

    for (; i < text.size(); ++i)
    {
        char c = text[i];

        switch (state)
        {
            case INIT_STATE:
                // Skip any whitespace
                if (isspace(c))
                {
                    continue;
                }
                else if (isdigit(c))
                {
                    literal = c;
                    if (c == '0' && peek(1) == 'x')
                    {
                        literal += "x";
                        is_hex_literal = true;
                        ++i;
                    }
                    else
                    {
                        is_hex_literal = false;
                    }
                    state = INT_LITERAL_STATE;
                }
                else if (is_key_path_char(c))
                {
                    pos = i;
                    state = KEY_PATH_STATE;
                }
                else if (c == '(' || c == ')' || c == ',' || c == '+' || c == '*' || c == '/' || c == '%')
                {
                    tokens.emplace_back(static_cast<TokenType>(c));
                }
                else if (c == '\"' || c == '\'')
                {
                    str_open_quote = c;
                    literal.clear();
                    state = STRING_LITERAL_STATE;
                }
                else if (c == '=')
                {
                    if (peek(1) == '=')
                    {
                        tokens.emplace_back(EQ_TOKEN);
                        ++i;
                    }
                    else
                    {
                        tokens.emplace_back(ASSIGN_TOKEN);
                    }
                }
                else if (c == '>')
                {
                    if (peek(1) == '=')
                    {
                        tokens.emplace_back(GE_TOKEN);
                        ++i;
                    }
                    else
                    {
                        tokens.emplace_back(GT_TOKEN);
                    }
                }
                else if (c == '<')
                {
                    if (peek(1) == '=')
                    {
                        tokens.emplace_back(LE_TOKEN);
                        ++i;
                    }
                    else
                    {
                        tokens.emplace_back(LT_TOKEN);
                    }
                }
                else if (c == '!')
                {
                    if (peek(1) == '=')
                    {
                        tokens.emplace_back(NEQ_TOKEN);
                        ++i;
                    }
                    else
                    {
                        tokens.emplace_back(NOT_TOKEN);
                    }
                }
                else if (c == '&')
                {
                    if (peek(1) == '&')
                    {
                        tokens.emplace_back(AND_TOKEN);
                        ++i;
                    }
                    else
                    {
                        tokens.emplace_back(CONCAT_TOKEN);
                    }
                }
                else if (c == '|' && peek(1) == '|')
                {
                    tokens.emplace_back(OR_TOKEN);
                    ++i;
                }
                else if (c == '-')
                {
                    if (peek(1) == '-')
                    {
                        state = COMMENT_STATE;
                    }
                    else
                    {
                        tokens.emplace_back(MINUS_TOKEN);
                    }
                }
                else
                {
                    literal = "unexpected character '";
                    literal += c;
                    literal += "'";
                    throw TemplateException(literal);
                }
                break;

            case KEY_PATH_STATE:
                if (!is_key_path_char(c))
                {
                    create_id_token(tokens, text.substr(pos, i - pos));

                    // Reprocess this char in the initial state.
                    state = INIT_STATE;
                    --i;
                }
                break;

            case STRING_LITERAL_STATE:
                if (c == str_open_quote)
                {
                    // Create the string literal token and return to init state.
                    tokens.emplace_back(STRING_LITERAL_TOKEN, literal);
                    state = INIT_STATE;
                }
                else if (c == '\\')
                {
                    i += append_string_escape(literal, peek);
                }
                else
                {
                    literal += c;
                }
                break;

            case INT_LITERAL_STATE:
                if (is_hex_literal ? isxdigit(c) : isdigit(c))
                {
                    literal += c;
                }
                else
                {
                    tokens.emplace_back(INT_LITERAL_TOKEN, literal);
                    state = INIT_STATE;
                    --i;
                }
                break;

            case COMMENT_STATE:
                if (c == '\n')
                {
                    state = INIT_STATE;
                }
                break;
        }
    }

    // Handle a key path terminated by end of string.
    if (state == KEY_PATH_STATE)
    {
        create_id_token(tokens, text.substr(pos, i - pos));
    }
    else if (state == STRING_LITERAL_STATE)
    {
        throw TemplateException("unterminated string literal");
    }
    else if (state == INT_LITERAL_STATE)
    {
        tokens.emplace_back(INT_LITERAL_TOKEN, literal);
    }

    return tokens;
}

//////////////////////////////////////////////////////////////////////////
// Expr parser
//////////////////////////////////////////////////////////////////////////

// Expression grammar
//
// expr         ::= oterm [ "if" oterm "else" oterm ]
// oterm        ::= bterm ( "or" bterm )*
// bterm        ::= bfactor ( "and" bfactor )*
// bfactor      ::= gfactor [ ( "==" | "!=" ) gfactor ]
// gfactor      ::= afactor [ ( ">" | ">=" | "<" | "<=" ) afactor ]
// afactor      ::= mfactor [ ( "&" | "+" | "-" ) afactor ]
// mfactor      ::= factor [ ( "*" | "/" | "%" ) mfactor ]
// factor       ::= "not" expr
//              |   "-" expr
//              |   "(" expr ")"
//              |   ( "true" | "false" )
//              |   KEY_PATH [ args ]
//              |   INT
// args         ::= "(" [ expr ( "," expr )* ")"

data_ptr ExprParser::get_var_value(const std::string &path, data_list &params)
{
    // Check if this is a pseudo function.
    bool is_fn = false;
    if (path == "count" || path == "empty" || path == "defined" || path == "addIndent" || path == "int" ||
        path == "str" || path == "upper" || path == "lower")
    {
        is_fn = true;
    }

    try
    {
        data_ptr result;
        if (is_fn)
        {
            if (params.size() != 1 && path != "addIndent")
            {
                throw TemplateException("function " + path + " requires 1 parameter");
            }
            else if (params.size() != 2 && path == "addIndent")
            {
                throw TemplateException("function " + path + " requires 2 parameters");
            }

            if (path == "count")
            {
                result = params[0]->getlist().size();
            }
            else if (path == "empty")
            {
                result = params[0]->empty();
            }
            else if (path == "defined")
            {
                // TODO: handle undefined case for defined fn
                result = true;
            }
            else if (path == "addIndent")
            {
                std::stringstream ss(params[1]->getvalue());
                if (!ss.eof())
                {
                    std::string line;
                    std::string resultValue;
                    int c = 0;
                    while (std::getline(ss, line))
                    {
                        ++c;
                        if (c > 1)
                        {
                            resultValue += '\n';
                        }
                        resultValue += params[0]->getvalue() + line;
                    }
                    result = resultValue;
                }
            }
            else if (path == "int")
            {
                result = params[0]->getint();
            }
            else if (path == "str")
            {
                result = params[0]->getvalue();
            }
            else if (path == "upper")
            {
                std::string s = params[0]->getvalue();
                std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c)
                               {
                                   return std::toupper(c);
                               });
                result = s;
            }
            else if (path == "lower")
            {
                std::string s = params[0]->getvalue();
                std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c)
                               {
                                   return std::tolower(c);
                               });
                result = s;
            }
        }
        else
        {
            result = m_data.parse_path(path);

            // Handle subtemplates.
            if (result.is_template())
            {
                std::shared_ptr<Data> tmplData = result.get();
                DataTemplate *tmpl = dynamic_cast<DataTemplate *>(tmplData.get());
                assert(tmpl);
                result = tmpl->eval(m_data, &params);
            }
        }

        return result;
    }
    catch (data_map::key_error &)
    {
        // Return an empty string for invalid key so it will eval to false.
        return "";
    }
}

data_ptr ExprParser::parse_factor()
{
    TokenType tokType = m_tok->get_type();
    data_ptr result;
    switch (tokType)
    {
        case NOT_TOKEN:
            m_tok.next();
            result = parse_expr()->empty();
            break;
        case MINUS_TOKEN:
            m_tok.next();
            result = -(parse_expr()->getint());
            break;
        case OPEN_PAREN_TOKEN:
            m_tok.next();
            result = parse_expr();
            m_tok.match(CLOSE_PAREN_TOKEN, "expected close paren");
            break;
        case STRING_LITERAL_TOKEN:
            result = m_tok.match(STRING_LITERAL_TOKEN)->get_value();
            break;
        case TRUE_TOKEN:
            m_tok.next();
            result = true;
            break;
        case FALSE_TOKEN:
            m_tok.next();
            result = false;
            break;
        case INT_LITERAL_TOKEN:
        {
            const Token *literal = m_tok.match(INT_LITERAL_TOKEN, "expected int literal");
            return new DataInt((int)std::strtol(literal->get_value().c_str(), NULL, 0));
            break;
        }
        case KEY_PATH_TOKEN:
        {
            const Token *path = m_tok.match(KEY_PATH_TOKEN, "expected key path");

            data_list params;
            if (m_tok->get_type() == OPEN_PAREN_TOKEN)
            {
                m_tok.match(OPEN_PAREN_TOKEN);

                while (m_tok->get_type() != CLOSE_PAREN_TOKEN)
                {
                    params.push_back(parse_expr());

                    if (m_tok->get_type() != CLOSE_PAREN_TOKEN)
                    {
                        m_tok.match(COMMA_TOKEN, "expected comma");
                    }
                }
                m_tok.match(CLOSE_PAREN_TOKEN, "expected close paren");
            }

            return get_var_value(path->get_value(), params);
        }
        default:
            throw TemplateException("syntax error");
    }
    return result;
}

data_ptr ExprParser::parse_bfactor()
{
    data_ptr ldata = parse_gfactor();

    TokenType tokType = m_tok->get_type();
    if (tokType == EQ_TOKEN || tokType == NEQ_TOKEN)
    {
        m_tok.next();

        data_ptr rdata = parse_gfactor();

        std::string lhs = ldata->getvalue();
        std::string rhs = rdata->getvalue();
        switch (tokType)
        {
            case EQ_TOKEN:
                ldata = (lhs == rhs);
                break;
            case NEQ_TOKEN:
                ldata = (lhs != rhs);
                break;
            default:
                break;
        }
    }
    return ldata;
}

data_ptr ExprParser::parse_gfactor()
{
    data_ptr ldata = parse_afactor();

    TokenType tokType = m_tok->get_type();
    if (tokType == GT_TOKEN || tokType == GE_TOKEN || tokType == LT_TOKEN || tokType == LE_TOKEN)
    {
        m_tok.next();

        data_ptr rdata = parse_afactor();

        std::shared_ptr<DataInt> li = std::dynamic_pointer_cast<DataInt>(ldata.get());
        std::shared_ptr<DataInt> ri = std::dynamic_pointer_cast<DataInt>(rdata.get());

        if (li && ri)
        {
            int l = li->getint();
            int r = ri->getint();
            switch (tokType)
            {
                case GT_TOKEN:
                    ldata = (l > r);
                    break;
                case GE_TOKEN:
                    ldata = (l >= r);
                    break;
                case LT_TOKEN:
                    ldata = (l < r);
                    break;
                case LE_TOKEN:
                    ldata = (l <= r);
                    break;
                default:
                    break;
            }
        }
        else
        {
            std::string lhs = ldata->getvalue();
            std::string rhs = rdata->getvalue();
            switch (tokType)
            {
                case GT_TOKEN:
                    ldata = (lhs > rhs);
                    break;
                case GE_TOKEN:
                    ldata = (lhs >= rhs);
                    break;
                case LT_TOKEN:
                    ldata = (lhs < rhs);
                    break;
                case LE_TOKEN:
                    ldata = (lhs <= rhs);
                    break;
                default:
                    break;
            }
        }
    }
    return ldata;
}

data_ptr ExprParser::parse_afactor()
{
    data_ptr ldata = parse_mfactor();

    TokenType tokType = m_tok->get_type();
    if (tokType == PLUS_TOKEN || tokType == MINUS_TOKEN || tokType == CONCAT_TOKEN)
    {
        m_tok.next();

        data_ptr rdata = parse_afactor();

        switch (tokType)
        {
            case CONCAT_TOKEN:
                ldata = ldata->getvalue() + rdata->getvalue();
                break;
            case PLUS_TOKEN:
                ldata = ldata->getint() + rdata->getint();
                break;
            case MINUS_TOKEN:
                ldata = ldata->getint() - rdata->getint();
                break;
            default:
                break;
        }
    }
    return ldata;
}

data_ptr ExprParser::parse_mfactor()
{
    data_ptr ldata = parse_factor();

    TokenType tokType = m_tok->get_type();
    if (tokType == TIMES_TOKEN || tokType == DIVIDE_TOKEN || tokType == MOD_TOKEN)
    {
        m_tok.next();

        data_ptr rdata = parse_mfactor();

        switch (tokType)
        {
            case TIMES_TOKEN:
                ldata = ldata->getint() * rdata->getint();
                break;
            case DIVIDE_TOKEN:
                if (rdata->getint() == 0)
                {
                    ldata = 0;
                }
                else
                {
                    ldata = ldata->getint() / rdata->getint();
                }
                break;
            case MOD_TOKEN:
                if (rdata->getint() == 0)
                {
                    ldata = 0;
                }
                else
                {
                    ldata = ldata->getint() % rdata->getint();
                }
                break;
            default:
                break;
        }
    }
    return ldata;
}

data_ptr ExprParser::parse_bterm()
{
    data_ptr ldata = parse_bfactor();

    while (m_tok->get_type() == AND_TOKEN)
    {
        m_tok.match(AND_TOKEN);

        data_ptr rdata = parse_bfactor();

        ldata = (!ldata->empty() && !rdata->empty());
    }
    return ldata;
}

data_ptr ExprParser::parse_oterm()
{
    data_ptr ldata = parse_bterm();

    while (m_tok->get_type() == OR_TOKEN)
    {
        m_tok.match(OR_TOKEN);

        data_ptr rdata = parse_bterm();

        if (ldata->empty())
        {
            ldata = rdata;
        }
    }

    return ldata;
}

data_ptr ExprParser::parse_expr()
{
    data_ptr ldata = parse_oterm();

    if (m_tok->get_type() == IF_TOKEN)
    {
        m_tok.match(IF_TOKEN);
        data_ptr predicate = parse_oterm();
        m_tok.match(ELSE_TOKEN);
        data_ptr rdata = parse_oterm();

        if (predicate->empty())
        {
            ldata = rdata;
        }
    }

    return ldata;
}

//////////////////////////////////////////////////////////////////////////
// Node classes
//////////////////////////////////////////////////////////////////////////

// defaults, overridden by subclasses with children
void Node::set_children(node_vector &)
{
    throw TemplateException(get_line(), "This node type cannot have children");
}

node_vector &Node::get_children()
{
    throw TemplateException(get_line(), "This node type cannot have children");
}

// NodeText
NodeType NodeText::gettype()
{
    return NODE_TYPE_TEXT;
}

#if __CYGWIN__ || _WIN32
void normalize_eol(std::string &str)
{
    std::string str2 = "";
    for (int i = 0; i < str.size(); i++)
    {
        if (str[i] == '\n')
        {
            str2 += "\r\n";
        }
        else if (str[i] != '\r')
        {
            str2 += str[i];
        }
    }
    str = str2;
}
#endif

void NodeText::gettext(std::ostream &stream, data_map &)
{
    std::string str;
    if (s_removeNewLine && m_text[0] == '\n')
    {
        if (m_text.size() > 1)
        {
            str = m_text.substr(1, m_text.size() - 1);
        }
    }
    else
    {
        str = m_text;
    }
    s_removeNewLine = false;

#if __CYGWIN__ || _WIN32
    normalize_eol(str);
#endif

    stream << str;
}

// NodeVar
NodeType NodeVar::gettype()
{
    return NODE_TYPE_VAR;
}

void NodeVar::gettext(std::ostream &stream, data_map &data)
{
    try
    {
        TokenIterator it(m_expr);
        ExprParser expr(it, data);
        data_ptr result = expr.parse_expr();
        std::string str = result->getvalue();
        if (str == "" && m_removeNewLine)
        {
            s_removeNewLine = true;
        }

#if __CYGWIN__ || _WIN32
        normalize_eol(str);
#endif

        stream << str;
    }
    catch (TemplateException e)
    {
        e.set_line_if_missing(get_line());
        throw e;
    }
}

// NodeVar
void NodeParent::set_children(node_vector &children)
{
    m_children.assign(children.begin(), children.end());
}

node_vector &NodeParent::get_children()
{
    return m_children;
}

// NodeFor
NodeFor::NodeFor(const token_vector &tokens, bool is_top, uint32_t line)
: NodeParent(line)
, m_is_top(is_top)
, m_has_predicate(false)
{
    TokenIterator tok(tokens);
    tok.match(FOR_TOKEN, "expected 'for'");
    m_val = tok.match(KEY_PATH_TOKEN, "expected key path")->get_value();
    tok.match(IN_TOKEN, "expected 'in'");
    m_key = tok.match(KEY_PATH_TOKEN, "expected key path")->get_value();
    if (tok->get_type() != END_TOKEN)
    {
        tok.match(IF_TOKEN, "expected 'if'");
        while (tok->get_type() != END_TOKEN)
        {
            m_predicate_tokens.push_back(*tok.get());
            ++tok;
        }
        m_has_predicate = true;
    }
    tok.match(END_TOKEN, "expected end of statement");
}

NodeType NodeFor::gettype()
{
    return NODE_TYPE_FOR;
}

data_map NodeFor::build_loop_map(size_t i, size_t count)
{
    data_map loop;
    loop["index"] = make_data(i + 1);
    loop["index0"] = make_data(i);
    loop["first"] = make_data(i == 0);
    loop["last"] = make_data(i == count - 1);
    loop["even"] = make_data((i + 1) % 2 == 0);
    loop["odd"] = make_data((i + 1) % 2 == 1);
    loop["count"] = make_data(count);
    loop["addNewLineIfNotLast"] = (i != count - 1) ? "\n" : "";
    return loop;
}

void NodeFor::gettext(std::ostream &stream, data_map &data)
{
    try
    {
        data_ptr saved_loop;
        if (!m_is_top)
        {
            saved_loop = data["loop"];
        }
        data_ptr value = data.parse_path(m_key);
        data_list filtered_items;
        if (m_has_predicate)
        {
            data_list &items_to_filter = value->getlist();
            for (size_t i = 0; i < items_to_filter.size(); ++i)
            {
                data["loop"] = make_data(build_loop_map(i, items_to_filter.size()));
                data[m_val] = items_to_filter[i];
                TokenIterator it(m_predicate_tokens);
                ExprParser parser(it, data);
                data_ptr d = parser.parse_expr();
                if (!d->empty())
                {
                    filtered_items.push_back(items_to_filter[i]);
                }
            }
        }
        data_list &items = m_has_predicate ? filtered_items : value->getlist();
        for (size_t i = 0; i < items.size(); ++i)
        {
            data["loop"] = make_data(build_loop_map(i, items.size()));
            data[m_val] = items[i];
            for (size_t j = 0; j < m_children.size(); ++j)
            {
                m_children[j]->gettext(stream, data);
            }
        }
        if (!m_is_top)
        {
            data["loop"] = saved_loop;
        }
    }
    catch (data_map::key_error &)
    {
        // ignore exception - the for loop key variable doesn't exist, so just
        // don't execute the for loop at all
    }
    catch (TemplateException e)
    {
        e.set_line_if_missing(get_line());
        throw e;
    }
}

// NodeIf
NodeIf::NodeIf(const token_vector &expr, uint32_t line)
: NodeParent(line)
, m_expr(expr)
, m_else_if(nullptr)
, m_if_type(NODE_TYPE_IF)
{
    if (expr[0].get_type() == ELIF_TOKEN)
    {
        m_if_type = NODE_TYPE_ELIF;
    }
    else if (expr[0].get_type() == ELSE_TOKEN)
    {
        m_if_type = NODE_TYPE_ELSE;
    }
}

NodeType NodeIf::gettype()
{
    return m_if_type;
}

void NodeIf::set_else_if(node_ptr else_if)
{
    m_else_if = else_if;
}

void NodeIf::gettext(std::ostream &stream, data_map &data)
{
    if (is_true(data))
    {
        for (size_t j = 0; j < m_children.size(); ++j)
        {
            m_children[j]->gettext(stream, data);
        }
    }
    else if (m_else_if)
    {
        m_else_if->gettext(stream, data);
    }
}

bool NodeIf::is_true(data_map &data)
{
    try
    {
        TokenIterator it(m_expr);
        if (m_if_type == NODE_TYPE_IF)
        {
            it.match(IF_TOKEN, "expected 'if' keyword");
        }
        else if (m_if_type == NODE_TYPE_ELIF)
        {
            it.match(ELIF_TOKEN, "expected 'elif' keyword");
        }
        else if (m_if_type == NODE_TYPE_ELSE)
        {
            it.match(ELSE_TOKEN, "expected 'else' keyword");
            it.match(END_TOKEN, "expected end of statement");
            return true;
        }
        ExprParser parser(it, data);
        data_ptr d = parser.parse_expr();
        it.match(END_TOKEN, "expected end of statement");

        return !d->empty();
    }
    catch (TemplateException e)
    {
        e.set_line_if_missing(get_line());
        throw e;
    }
}

bool NodeIf::is_else()
{
    return m_if_type == NODE_TYPE_ELSE;
}

// NodeDef
NodeDef::NodeDef(const token_vector &expr, uint32_t line)
: NodeParent(line)
{
    TokenIterator tok(expr);
    tok.match(DEF_TOKEN, "expected 'def'");

    m_name = tok.match(KEY_PATH_TOKEN, "expected key path")->get_value();

    if (tok->get_type() == OPEN_PAREN_TOKEN)
    {
        tok.match(OPEN_PAREN_TOKEN);

        while (tok->get_type() != CLOSE_PAREN_TOKEN)
        {
            m_params.push_back(tok.match(KEY_PATH_TOKEN, "expected key path")->get_value());

            if (tok->get_type() != CLOSE_PAREN_TOKEN)
            {
                tok.match(COMMA_TOKEN, "expected comma");
            }
        }
        tok.match(CLOSE_PAREN_TOKEN, "expected close paren");
    }
    tok.match(END_TOKEN, "expected end of statement");
}

NodeType NodeDef::gettype()
{
    return NODE_TYPE_DEF;
}

void NodeDef::gettext(std::ostream &stream, data_map &data)
{
    // Follow the key path.
    data_ptr &target = data.parse_path(m_name, true);

    // Set the map entry's value to a newly created template. The nodes were already
    // parsed and set as our m_children vector. The names of the template's parameters
    // are set from the param names we parsed in the ctor.
    DataTemplate *tmpl = new DataTemplate(m_children);
    tmpl->params() = m_params;
    target = data_ptr(tmpl);
}

// NodeSet
NodeType NodeSet::gettype()
{
    return NODE_TYPE_SET;
}

void NodeSet::gettext(std::ostream &stream, data_map &data)
{
    TokenIterator tok(m_expr);
    tok.match(SET_TOKEN, "expected 'set'");
    std::string path = tok.match(KEY_PATH_TOKEN, "expected key path")->get_value();
    tok.match(ASSIGN_TOKEN);
    ExprParser parser(tok, data);
    data_ptr value = parser.parse_expr();
    tok.match(END_TOKEN, "expected end of statement");

    // Follow the key path, creating the key if missing.
    data_ptr &target = data.parse_path(path, true);
    target = value;
}

inline size_t count_newlines(const std::string &text)
{
    return std::count(text.begin(), text.end(), '\n');
}

//////////////////////////////////////////////////////////////////////////
// tokenize
// parses a template into nodes (text, for, if, variable, def)
//////////////////////////////////////////////////////////////////////////

TemplateParser::TemplateParser(const std::string &text, node_vector &nodes)
: m_text(text)
, m_top_nodes(nodes)
, m_current_line(1)
, m_node_stack()
, m_current_node()
, m_current_nodes(&m_top_nodes)
, m_eol_precedes(true)
, m_last_was_eol(true)
, m_until(INVALID_TOKEN)
{
}

node_vector &TemplateParser::parse()
{
    try
    {
        while (!m_text.empty())
        {
            // search for the start of a block
            size_t pos = m_text.find("{");
            if (pos == std::string::npos)
            {
                if (!m_text.empty())
                {
                    m_current_nodes->push_back(node_ptr(new NodeText(m_text, m_current_line)));
                }
                return m_top_nodes;
            }
            std::string pre_text = m_text.substr(0, pos);
            m_current_line += count_newlines(pre_text);

            // Track whether there was an EOL prior to this open brace.
            bool newLastWasEol = pos > 0 && m_text[pos - 1] == '\n';
            m_eol_precedes = (pos == 0 && m_last_was_eol) || newLastWasEol;
            if (pos > 0)
            {
                m_last_was_eol = newLastWasEol;
            }

            bool has_kill_ws = m_text.size() > pos + 2 && m_text[pos + 2] == '<';
            if (has_kill_ws)
            {
                // remove whitespace back to the last newline
                while (std::isspace(pre_text.back()) && pre_text.back() != '\n')
                {
                    pre_text.pop_back();
                }
            }

            if (!pre_text.empty())
            {
                m_current_nodes->push_back(node_ptr(new NodeText(pre_text, m_current_line)));
            }

            m_text = m_text.substr(pos + 1);
            if (m_text.empty())
            {
                m_current_nodes->push_back(node_ptr(new NodeText("{", m_current_line)));
                return m_top_nodes;
            }

            // process a block
            switch (m_text[0])
            {
                case '$':
                    parse_var();
                    break;
                case '%':
                    parse_stmt();
                    break;
                case '#':
                    parse_comment();
                    break;
                default:
                    m_current_nodes->push_back(node_ptr(new NodeText("{", m_current_line)));
            }
        }

        return m_top_nodes;
    }
    catch (TemplateException e)
    {
        e.set_line_if_missing(m_current_line);
        throw e;
    }
}

void TemplateParser::parse_var()
{
    size_t pos = m_text.find("}");
    if (pos == std::string::npos)
    {
        throw TemplateException(m_current_line, "unterminated variable block");
    }

    std::string var_text = m_text.substr(1, pos - 1);

    bool has_kill_newline_if_empty = !var_text.empty() && var_text[0] == '>' && var_text.size() > 2;
    if (has_kill_newline_if_empty)
    {
        var_text = var_text.substr(1, pos - 2);
    }

    bool has_kill_newline = !var_text.empty() && var_text.back() == '>';
    if (has_kill_newline)
    {
        var_text.pop_back();
    }

    bool eol_follows = m_text.size() > pos + 1 && m_text[pos + 1] == '\n';
    m_text = m_text.substr(pos + 1 + (has_kill_newline && eol_follows ? 1 : 0));

    token_vector stmt_tokens = tokenize_statement(var_text);
    m_current_nodes->push_back(node_ptr(new NodeVar(stmt_tokens, m_current_line, has_kill_newline_if_empty)));

    m_current_line += count_newlines(var_text);
    m_last_was_eol = false;
}

void TemplateParser::push_node(Node *new_node, TokenType until)
{
    m_node_stack.push(std::make_pair(m_current_node, m_until));
    m_current_node = node_ptr(new_node);
    m_current_nodes->push_back(m_current_node);
    m_current_nodes = &m_current_node->get_children();
    m_until = until;
}

void TemplateParser::parse_stmt()
{
    size_t pos = m_text.find("%}");
    if (pos == std::string::npos)
    {
        throw TemplateException(m_current_line, "unterminated statement block");
    }

    std::string stmt_text = m_text.substr(1, pos - 1);
    bool has_kill_newline = !stmt_text.empty() && stmt_text.back() == '>';
    if (has_kill_newline)
    {
        stmt_text.pop_back();
    }
    uint32_t lineCount = count_newlines(stmt_text);

    // Tokenize the control statement.
    token_vector stmt_tokens = tokenize_statement(stmt_text);
    if (!stmt_tokens.empty())
    {
        TokenType first_token_type = stmt_tokens[0].get_type();

        // Create control statement nodes.
        switch (first_token_type)
        {
            case FOR_TOKEN:
                push_node(new NodeFor(stmt_tokens, m_node_stack.empty(), m_current_line), ENDFOR_TOKEN);
                break;

            case IF_TOKEN:
                push_node(new NodeIf(stmt_tokens, m_current_line), ENDIF_TOKEN);
                break;

            case ELIF_TOKEN:
            case ELSE_TOKEN:
            {
                auto current_if = dynamic_cast<NodeIf *>(m_current_node.get());
                if (!current_if)
                {
                    throw TemplateException(m_current_line, "else/elif without if");
                }

                if (current_if->is_else())
                {
                    throw TemplateException(m_current_line, "if already has else");
                }

                m_current_node = node_ptr(new NodeIf(stmt_tokens, m_current_line));
                current_if->set_else_if(m_current_node);
                m_current_nodes = &m_current_node->get_children();
                break;
            }

            case DEF_TOKEN:
                push_node(new NodeDef(stmt_tokens, m_current_line), ENDDEF_TOKEN);
                break;

            case SET_TOKEN:
                m_current_nodes->push_back(node_ptr(new NodeSet(stmt_tokens, m_current_line)));
                break;

            case ENDFOR_TOKEN:
            case ENDIF_TOKEN:
            case ENDDEF_TOKEN:
                if (m_until == first_token_type)
                {
                    assert(!m_node_stack.empty());
                    auto top = m_node_stack.top();
                    m_node_stack.pop();
                    if (top.first)
                    {
                        m_current_node = top.first;
                        m_current_nodes = &m_current_node->get_children();
                        m_until = top.second;
                    }
                    else
                    {
                        m_current_node.reset();
                        m_current_nodes = &m_top_nodes;
                        m_until = INVALID_TOKEN;
                    }
                }
                else
                {
                    throw TemplateException(m_current_line, "unexpected end statement");
                }
                break;

            default:
                throw TemplateException(m_current_line, "invalid control statement");
        }
    }

    // Chop off following eol if this control statement is on a line by itself.
    check_omit_eol(pos, has_kill_newline);

    m_current_line += lineCount;
}

void TemplateParser::parse_comment()
{
    size_t pos = m_text.find("#}");
    if (pos == std::string::npos)
    {
        return;
    }

    std::string comment_text = m_text.substr(1, pos - 1);
    m_current_line += count_newlines(comment_text);

    check_omit_eol(pos, false);
}

void TemplateParser::check_omit_eol(size_t pos, bool force_omit)
{
    pos += 2;
    bool eol_follows = m_text.size() > pos && m_text[pos] == '\n';

    // Chop off following eol if this block is on a line by itself.
    if ((force_omit || m_eol_precedes) && eol_follows)
    {
        ++pos;
        ++m_current_line;
        m_last_was_eol = true;
    }

    m_text = m_text.substr(pos);
}

} // namespace impl

/************************************************************************
* parse
*
*  1. tokenizes template
*  2. parses tokens into tree
*  3. resolves template
*  4. returns converted text
************************************************************************/
std::string parse(const std::string &templ_text, data_map &data)
{
    return DataTemplate(templ_text).eval(data);
}
void parse(std::ostream &stream, const std::string &templ_text, data_map &data)
{
    DataTemplate(templ_text).eval(stream, data);
}
}

#endif // defined(CPPTEMPL_UNIT_TEST)
