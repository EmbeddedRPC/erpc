/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
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

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* write header with token defines */
%defines

/* make it reentrant */
%define api.pure full

/* put more info in error messages */
%error-verbose

/* enable location processing */
%locations

// generate a token tables name
%token-table

/*******************************************************************************
 * Code
 ******************************************************************************/

%{
#include "ErpcLexer.h"
#include "AstNode.h"
#include "Logging.h"
#include "format_string.h"

using namespace erpcgen;

/*! Our special location type. */
#define YYLTYPE token_loc_t

// this indicates that we're using our own type. it should be unset automatically
// but that's not working for some reason with the .hpp file.
#if defined(YYLTYPE_IS_TRIVIAL)
    #undef YYLTYPE_IS_TRIVIAL
    #define YYLTYPE_IS_TRIVIAL 0
#endif

/*! Default location action */
#define YYLLOC_DEFAULT(Current, Rhs, N) \
    do {        \
        if (N)  \
        {       \
            (Current).m_firstLine = YYRHSLOC(Rhs, 1).m_firstLine;   \
            (Current).m_lastLine = YYRHSLOC(Rhs, N).m_lastLine;     \
        }       \
        else    \
        {       \
            (Current).m_firstLine = (Current).m_lastLine = YYRHSLOC(Rhs, 0).m_lastLine; \
        }       \
    } while (0)
%}

/* symbol types */
%union {
    int64_t m_int;
/*    std::string * m_str;*/
    erpcgen::Token * m_token;
    erpcgen::AstNode * m_ast;
}

%code provides {
/*! @brief Returns the string containing the human-readable name of a token. */
const char * get_token_name(int tok);
}

%code {
/*! Forward declaration of yylex(). */
static int yylex(YYSTYPE * lvalp, YYLTYPE * yylloc, ErpcLexer * lexer);

/*! Forward declaration of error handling function. */
static void yyerror(YYLTYPE * yylloc, ErpcLexer * lexer, AstNode ** resultAST, const char * error);
}

%code {
/*! Function for merge two strings */
static AstNode * mergeString(Token * t1, Token * t2);

/*! Function for merge two token locations */
token_loc_t mergeLocation(const token_loc_t & l1, const token_loc_t & l2);
}

/* extra parameters for the parser and lexer */
%parse-param    {erpcgen::ErpcLexer * lexer}
%parse-param    {erpcgen::AstNode ** resultAST}
%lex-param      {erpcgen::ErpcLexer * lexer}

// token definitions
%token <m_token> TOK_IDENT            "identifier"
%token <m_token> TOK_STRING_LITERAL   "string"
%token <m_token> TOK_INT_LITERAL      "integer"
%token <m_token> TOK_FLOAT_LITERAL    "float"
%token <m_token> '@'
%token <m_token> '('
%token <m_token> ')'
%token <m_token> '{'
%token <m_token> '}'
%token <m_token> '['
%token <m_token> ']'
%token <m_token> '<'
%token <m_token> '>'
%token <m_token> '='
%token <m_token> ','
%token <m_token> ';'
%token <m_token> ':'
%token <m_token> '+'
%token <m_token> '-'
%token <m_token> '*'
%token <m_token> '/'
%token <m_token> '%'
%token <m_token> '^'
%token <m_token> '~'
%token <m_token> '&'
%token <m_token> '|'
%token <m_token> TOK_LSHIFT       "<<"
%token <m_token> TOK_RSHIFT       ">>"
%token <m_token> TOK_ARROW        "->"
%token <m_token> TOK_OPTIONS      "options"
%token <m_token> TOK_CONST        "const"
%token <m_token> TOK_IMPORT       "import"
%token <m_token> TOK_ENUM         "enum"
%token <m_token> TOK_STRUCT       "struct"
%token <m_token> TOK_UNION        "union"
%token <m_token> TOK_SWITCH       "switch"
%token <m_token> TOK_CASE         "case"
%token <m_token> TOK_DEFAULT      "default"
%token <m_token> TOK_OPTIONAL     "optional"
%token <m_token> TOK_BYREF        "byref"
%token <m_token> TOK_TYPE         "type"
%token <m_token> TOK_INTERFACE    "interface"
%token <m_token> TOK_VERSION      "version"
%token <m_token> TOK_IN           "in"
%token <m_token> TOK_OUT          "out"
%token <m_token> TOK_INOUT        "inout"
%token <m_token> TOK_ASYNC        "async"
%token <m_token> TOK_ONEWAY       "oneway"
%token <m_token> TOK_LIST         "list"
%token <m_token> TOK_REF          "ref"
%token <m_token> TOK_TRUE         "true"
%token <m_token> TOK_FALSE        "false"
%token <m_token> TOK_VOID         "void"
%token <m_token> TOK_ML_COMMENT   "doxygen ml. comment"
%token <m_token> TOK_IL_COMMENT   "doxygen il. comment"
%token <m_token> TOK_PROGRAM      "program"
%token END       0                "end of file"

// virtual tokens used for AST
%token <m_token> TOK_CHILDREN
%token <m_token> TOK_ENUM_MEMBER
%token <m_token> TOK_STRUCT_MEMBER
%token <m_token> TOK_UNION_CASE
%token <m_token> TOK_FUNCTION
%token <m_token> TOK_PARAM
%token <m_token> TOK_RETURN
%token <m_token> TOK_EXPR
%token <m_token> TOK_ANNOTATION
%token <m_token> TOK_UNARY_NEGATE
%token <m_token> TOK_ARRAY

// operator precedence
%left '|'
%left '^'
%left '&'
%left "<<" ">>"
%left '+' '-'
%left '*' '/' '%'
%right UNARY_OP

// nonterminal grammar nodes
%type <m_ast> annotation
%type <m_ast> annotation_list
%type <m_ast> annotation_list_opt
%type <m_ast> annotation_value_opt
%type <m_ast> array_type
%type <m_ast> callback_def
%type <m_ast> callback_param
%type <m_ast> callback_param_list
%type <m_ast> callback_param_list_opt
%type <m_ast> const_def
%type <m_ast> const_expr
%type <m_ast> data_type
%type <m_ast> def_list
%type <m_ast> definition
%type <m_ast> definition_base
%type <m_ast> doxy_ml_comment
%type <m_ast> doxy_ml_comment_opt
%type <m_ast> doxy_il_comment_opt
%type <m_ast> enum_def
%type <m_ast> enumerator
%type <m_ast> enumerator_list
%type <m_ast> enumerator_list_opt
%type <m_ast> expr
%type <m_ast> float_value
%type <m_ast> function_def
%type <m_ast> function_def_list
%type <m_ast> function_def_list_opt
%type <m_ast> function_return_type
%type <m_ast> function_type_base_def
%type <m_ast> function_type_def
%type <m_ast> ident
%type <m_ast> ident_opt
%type <m_ast> int_const_expr
%type <m_ast> int_value
%type <m_ast> interface_def
%type <m_ast> list_type
%type <m_ast> param_def
%type <m_ast> param_def_in
%type <m_ast> param_list
%type <m_ast> param_list_in
%type <m_ast> param_list_opt
%type <m_ast> param_list_opt_in
%type <m_ast> program
%type <m_ast> root_def
%type <m_ast> simple_data_type
%type <m_ast> string_literal
%type <m_ast> struct_def
%type <m_ast> struct_data_type
%type <m_ast> struct_member
%type <m_ast> struct_member_list
%type <m_ast> name_opt
%type <m_ast> typedef_def
%type <m_ast> typename
%type <m_ast> unary_expr
%type <m_ast> union_def
%type <m_ast> union_type_def
%type <m_ast> union_case_list
%type <m_ast> union_case_expr_list
%type <m_ast> union_case
%type <m_ast> union_member
%type <m_ast> union_member_list_opt
%type <m_ast> union_member_list
%type <m_token> param_dir
%type <m_token> param_dir_in
%type <m_int> struct_member_options
%type <m_int> struct_member_options_list

// Destructor for token objects.
%destructor { if ($$) { delete $$; } } <m_token>

// Beginning of grammar.
%%

root_def        :   program def_list
                        {
                            *resultAST = new AstNode(Token(TOK_CHILDREN));
                            (*resultAST)->appendChild($1);
                            (*resultAST)->appendChild($2);
                        }
                |   def_list
                        {
                            *resultAST = new AstNode(Token(TOK_CHILDREN));
                            (*resultAST)->appendChild($1);
                        }
                |   /* empty */
                        {
                            *resultAST = new AstNode(Token(TOK_CHILDREN));
                        }
                ;

program         :  doxy_ml_comment_opt annotation_list_opt TOK_PROGRAM ident[name] semi_opt
                        {
                            AstNode * prog  = new AstNode(Token(TOK_PROGRAM));
                            prog->appendChild(new AstNode(*$name));
                            prog->appendChild($annotation_list_opt);
                            prog->appendChild($doxy_ml_comment_opt);
                            $$ = prog;
                        }
                ;

/*
 * TOK_CHILDREN -> ( definition* )
 */
def_list[result]
                :   definition
                        {
                            AstNode * l = new AstNode(Token(TOK_CHILDREN));
                            if ($definition)
                            {
                                l->appendChild($definition);
                            }
                            $result = l;
                        }
                |   def_list definition
                        {
                            if ($definition)
                            {
                                $def_list->appendChild($definition);
                            }
                            $result = $def_list;
                        }
                ;

definition      :   import_stmt
                        {
                            $$ = NULL;
                        }
                |   doxy_ml_comment_opt annotation_list_opt definition_base semi_opt doxy_il_comment_opt
                        {
                            $$ = $definition_base;
                            $$->appendChild($annotation_list_opt);
                            $$->appendChild($doxy_ml_comment_opt);
                            $$->appendChild($doxy_il_comment_opt);
                        }

definition_base :   const_def
                        {
                            $$ = $const_def;
                        }
                |   enum_def
                        {
                            $$ = $enum_def;
                        }
                |   struct_def
                        {
                            $$ = $struct_def;
                        }
                |   union_type_def
                        {
                            $$ = $union_type_def;
                        }
                |   typedef_def
                        {
                            $$ = $typedef_def;
                        }
                |   interface_def
                        {
                            $$ = $interface_def;
                        }
                |
                    function_type_def
                        {
                            $$ = $function_type_def;
                        }
                ;

import_stmt     :   "import" TOK_STRING_LITERAL
                        {
                            std::string s = $TOK_STRING_LITERAL->getStringValue();
                            lexer->pushFile(s);
                        }
                ;

/*
 * TOK_CONST -> ( simple_data_type ident const_expr )
 */
const_def       :   "const" simple_data_type ident '=' const_expr
                        {
                            $$ = new AstNode(*$1);
                            $$->appendChild($simple_data_type);
                            $$->appendChild($ident);
                            $$->appendChild($const_expr);
                        }
                ;

/*
 * TOK_ENUM -> ( ident, ( TOK_CHILDREN -> enums* ) )
 */
enum_def        :   "enum" name_opt[name] '{' enumerator_list_opt '}'
                        {
                            if ($enumerator_list_opt == NULL)
                            {
                                throw semantic_error(format_string("Enum on the %d.line must have at least one member.", $1->getFirstLine()));
                            }
                            else
                            {
                                $$ = new AstNode(*$1);
                                $$->appendChild($name);
                                $$->appendChild($enumerator_list_opt);
                            }
                        }
                ;

enumerator_list_opt
                :   enumerator_list
                        {
                            $$ = $enumerator_list;
                        }
                |   /* empty */
                        {
                            $$ = NULL;
                        }
                ;

comma_opt       :   ','
                |   /* empty */
                ;

semi_opt        :   ';'
                |   /* empty */
                ;

comma_semi_opt  :   ','
                |   ';'
                |   /* empty */
                ;
/*
 * TOK_CHILDREN -> ( enumerator* )
 */
enumerator_list[result]
                :   enumerator
                        {
                            AstNode * n = new AstNode(Token(TOK_CHILDREN));
                            n->appendChild($enumerator);
                            $result = n;
                        }
                |   enumerator_list[enum_list] enumerator
                        {
                            $enum_list->appendChild($enumerator);
                            $result = $enum_list;
                        }
                ;

/*
 * TOK_ENUM_MEMBER -> ( ident TOK_EXPR )
 */
enumerator      :   doxy_ml_comment_opt ident '=' int_const_expr annotation_list_opt[annotations] comma_opt doxy_il_comment_opt
                        {
                            $$ = new AstNode(Token(TOK_ENUM_MEMBER, NULL, @1));
                            $$->appendChild($ident);
                            $$->appendChild($int_const_expr);
                            $$->appendChild($annotations);
                            $$->appendChild($doxy_ml_comment_opt);
                            $$->appendChild($doxy_il_comment_opt);
                        }
                |   doxy_ml_comment_opt ident annotation_list_opt[annotations] comma_opt doxy_il_comment_opt
                        {
                            $$ = new AstNode(Token(TOK_ENUM_MEMBER, NULL, @1));
                            $$->appendChild($ident);
                            $$->appendChild(NULL);
                            $$->appendChild($annotations);
                            $$->appendChild($doxy_ml_comment_opt);
                            $$->appendChild($doxy_il_comment_opt);
                        }
                ;

/*
 * TOK_INTERFACE -> ( ident ( TOK_CHILDREN -> TOK_FUNCTION* ) ( TOK_CHILDREN -> TOK_ANNOTATION* ) )
 */
interface_def   :   "interface"[iface] ident[name] '{' function_def_list_opt[functions] '}'
                        {
                            $$ = new AstNode(*$iface);
                            $$->appendChild($name);
                            $$->appendChild($functions);
                        }
                ;

function_def_list_opt
                :   function_def_list
                        {
                            $$ = $function_def_list;
                        }
                |   /* empty */
                        {
                            $$ = NULL;
                        }
                ;

/*
 * TOK_CHILDREN -> ( function_def* )
 */
function_def_list
                :   function_def
                        {
                            $$ = new AstNode(Token(TOK_CHILDREN));
                            $$->appendChild($function_def);
                        }
                |   function_def_list[fun_def_list] function_def
                        {
                            $fun_def_list->appendChild($function_def);
                            $$ = $fun_def_list;
                        }
                ;

function_def    :   doxy_ml_comment_opt annotation_list_opt function_type_base_def comma_semi_opt doxy_il_comment_opt
                        {
                            $$ = $function_type_base_def;
                            $$->appendChild($annotation_list_opt);
                            $$->appendChild($doxy_ml_comment_opt);
                            $$->appendChild($doxy_il_comment_opt);
                        }

function_type_base_def
                :   function_type_def
                        {
                            $$ = $function_type_def;
                        }
                |   callback_def
                        {
                            $$ = $callback_def;
                        }
                ;
/*
 * TOK_FUNCTION -> ( ident ( simple_data_type | TOK_VOID | TOK_ONEWAY ) ( TOK_CHILDREN -> param_def* ) ( TOK_CHILDREN -> TOK_ANNOTATION* ) )
 */
function_type_def
                :   ident[name] '(' param_list_opt[params] ')' "->" function_return_type[return_type]
                        {
                            $$ = new AstNode(Token(TOK_FUNCTION, NULL, @name));
                            $$->appendChild($name);
                            $$->appendChild($return_type);
                            $$->appendChild(NULL);  // function type null to recognize function and callback
                            $$->appendChild($params);
                        }
                |    "oneway"[oneway] ident[name] '(' param_list_opt_in[params] ')'
                        {
                            $$ = new AstNode(Token(TOK_FUNCTION, NULL, @name));
                            $$->appendChild($name);
                            AstNode * returnAst = new AstNode(Token(TOK_RETURN));
                            returnAst->appendChild(new AstNode(*$oneway));
                            returnAst->appendChild(NULL);
                            $$->appendChild(returnAst);
                            $$->appendChild(NULL);  // function type null to recognize function and callback
                            $$->appendChild($params);
                        }

                ;

/* TODO: Annotations should be defined after data type. */
function_return_type
                :   annotation_list_opt simple_data_type
                        {
                            $$ = new AstNode(Token(TOK_RETURN));
                            $$->appendChild($simple_data_type);
                            $$->appendChild($annotation_list_opt);
                        }
                |   "void"
                        {
                            $$ = new AstNode(Token(TOK_RETURN));
                            $$->appendChild(new AstNode(*$1));
                            $$->appendChild(NULL);
                        }
                ;

param_list_opt  :   param_list
                        {
                            $$ = $param_list;
                        }
                |   /* empty */
                        {
                            $$ = NULL;
                        }
                ;

param_list_opt_in
                :   param_list_in
                        {
                            $$ = $param_list_in;
                        }
                |   /* empty */
                        {
                            $$ = NULL;
                        }
                ;

/*
 * TOK_CHILDREN -> ( param_def* )
 */
param_list      :   param_def
                        {
                            $$ = new AstNode(Token(TOK_CHILDREN));
                            $$->appendChild($param_def);
                        }
                |   param_list[parameter_list] ',' param_def
                        {
                            $parameter_list->appendChild($param_def);
                            $$ = $parameter_list;
                        }
                ;

param_list_in   :   param_def_in
                        {
                            $$ = new AstNode(Token(TOK_CHILDREN));
                            $$->appendChild($param_def_in);
                        }
                |   param_list_in[param_list] ',' param_def_in
                        {
                            $param_list->appendChild($param_def_in);
                            $$ = $param_list;
                        }
                ;

/*
 * TOK_PARAM -> ( ident simple_data_type ( TOK_CHILDREN -> TOK_ANNOTATION* ) )
 */
param_def       :   param_dir[dir] simple_data_type[datatype] ident_opt[name] annotation_list_opt[annotations]
                        {
                            $$ = new AstNode(Token(TOK_PARAM, NULL, @name));
                            $$->appendChild($name);
                            $$->appendChild($datatype);
                            AstNode *n = ($dir) ? new AstNode(*$dir) : NULL;
                            $$->appendChild(n);
                            $$->appendChild($annotations);
                        }
                ;

param_def_in    :   param_dir_in[dir] simple_data_type[datatype] ident_opt[name] annotation_list_opt[annotations]
                        {
                            $$ = new AstNode(Token(TOK_PARAM, NULL, @name));
                            $$->appendChild($name);
                            $$->appendChild($datatype);
                            AstNode *n = ($dir) ? new AstNode(*$dir) : NULL;
                            $$->appendChild(n);
                            $$->appendChild($annotations);
                        }
                ;

/*!
 * TODO set right place!
 */
param_dir_in    :   "in"
                        {
                            $$ = $1;
                        }
                |   /* empty */
                        {
                            $$ = NULL;
                        }

param_dir       :   param_dir_in
                        {
                            $$ = $param_dir_in;
                        }
                |   "out"
                        {
                            $$ = $1;
                        }
                |   "inout"
                        {
                            $$ = $1;
                        }
                ;

/*
 * TOK_FUNCTION -> ( ident ( IDENT ) ( TOK_CHILDREN -> ident* ) ( TOK_CHILDREN -> TOK_ANNOTATION* ) )
 */
callback_def    :  ident[type] ident[name]
                        {
                            $$ = new AstNode(Token(TOK_FUNCTION, NULL, @name));
                            $$->appendChild($name);
                            $$->appendChild(NULL);  // return type null to recognize function and callback
                            $$->appendChild($type);
                            $$->appendChild(NULL);
                        }
                |   ident[type] ident[name] '(' callback_param_list_opt[params] ')'
                        {
                            $$ = new AstNode(Token(TOK_FUNCTION, NULL, @name));
                            $$->appendChild($name);
                            $$->appendChild(NULL);  // return type null to recognize function and callback
                            $$->appendChild($type);
                            $$->appendChild($params);
                        }
                ;

callback_param_list_opt
                :   callback_param_list
                        {
                            $$ = $callback_param_list;
                        }
                |   /* empty */
                        {
                            $$ = NULL;
                        }
                ;

callback_param_list
                :   callback_param
                        {
                            $$ = new AstNode(Token(TOK_CHILDREN));
                            $$->appendChild($callback_param);
                        }
                |   callback_param_list[param_list] comma_opt callback_param
                        {
                            $$ = $param_list;
                            $param_list->appendChild($callback_param);
                        }
                ;

callback_param  :   ident[name]
                        {
                            $$ = new AstNode(Token(TOK_PARAM, NULL, @name));
                            $$->appendChild($name);
                            /* To be commaptible with param_def. */
                            $$->appendChild(NULL);
                            $$->appendChild(NULL);
                            $$->appendChild(NULL);
                        }
                ;

/*
 * TOK_TYPE -> ( ident data_type ( TOK_CHILDREN -> annotation* ) )
 */
typedef_def     :   "type"[type] ident[name] '=' data_type[typedef]
                        {
                            $$ = new AstNode(*$type);
                            $$->appendChild($name);
                            $$->appendChild($typedef);
                        }
                ;

/*
 * TOK_STRUCT -> ( ident ( TOK_CHILDREN -> struct_member* ) ( TOK_CHILDREN -> annotation* ) )
 */
struct_def      :   "struct"[struct] name_opt[name] '{' struct_member_list[members] '}'
                        {
                            $$ = new AstNode(*$struct);
                            $$->appendChild($name);
                            $$->appendChild($members);
                        }
                ;

/*
 * TOK_CHILDREN -> ( struct_member* )
 */
struct_member_list[result]
                :   struct_member
                        {
                            $result = new AstNode(Token(TOK_CHILDREN));
                            $result->appendChild($struct_member);
                        }
                |   struct_member_list[member_list]  struct_member
                        {
                            $member_list->appendChild($struct_member);
                            $result = $member_list;
                        }
                ;

/*
 * TOK_STRUCT_MEMBER -> ( ident simple_data_type ( TOK_CHILDREN -> annotation* ) )
 */
struct_member   :   doxy_ml_comment_opt struct_member_options_list struct_data_type[datatype] ident[name] annotation_list_opt[annotations] semi_opt doxy_il_comment_opt
                        {
                            $$ = new AstNode(Token(TOK_STRUCT_MEMBER));
                            $$->appendChild($name);
                            $$->appendChild($datatype);
                            $$->appendChild(new AstNode(Token(TOK_INT_LITERAL, new IntegerValue($struct_member_options_list))));
                            $$->appendChild($annotations);
                            $$->appendChild($doxy_ml_comment_opt);
                            $$->appendChild($doxy_il_comment_opt);
                        }
                ;

struct_member_options_list
                :   struct_member_options_list[member_options_list] struct_member_options
                        {
                            $$ = $member_options_list | $struct_member_options;
                        }
                |   /* empty */
                        {
                            $$ = 0;
                        }
                ;

struct_member_options
                :   "optional"
                        {
                            $$ = 1;
                        }
                |   "byref"
                        {
                            $$ = 2;
                        }
                ;

struct_data_type
                :   simple_data_type
                        {
                            $$ = $simple_data_type;
                        }
                |   union_def
                        {
                            $$ = $union_def;
                        }
                ;

union_def       :   "union"[union] '(' ident[discriminator] ')' '{' union_case_list[cases] '}'
                        {
                            $$ = new AstNode(*$union);
                            $$->appendChild(NULL);
                            $$->appendChild($discriminator);
                            $$->appendChild($cases);
                        }

union_type_def       :   "union"[union] ident[name] '{' union_case_list[cases] '}'
                        {
                            $$ = new AstNode(*$union);
                            $$->appendChild($name);
                            $$->appendChild(NULL);
                            $$->appendChild($cases);
                        }

union_case_list
                :   union_case
                        {
                            $$ = new AstNode(Token(TOK_CHILDREN));
                            $$->appendChild($union_case);
                        }
                |   union_case_list[case_list] union_case
                        {
                            $case_list->appendChild($union_case);
                            $$ = $case_list;
                        }
                ;

/* only allowing one type per case at first, including structs */
union_case      :   "case" union_case_expr_list[case_exprs] ':' union_member_list_opt[decl_list]
                        {
                            $$ = new AstNode(Token(TOK_UNION_CASE));
                            $$->appendChild($case_exprs);
                            $$->appendChild($decl_list);
                        }
                |   "default" ':' union_member_list_opt[decl_list]
                        {
                            $$ = new AstNode(Token(TOK_UNION_CASE));
                            $$->appendChild(new AstNode(*$1));
                            $$->appendChild($decl_list);
                        }
                ;

union_case_expr_list
                :   int_const_expr[case_expr]
                        {
                            $$ = new AstNode(Token(TOK_CHILDREN));
                            $$->appendChild($case_expr);
                        }
                |   union_case_expr_list[case_expr_list] ',' int_const_expr[case_expr]
                        {
                            $case_expr_list->appendChild($case_expr);
                            $$ = $case_expr_list;
                        }
                ;

union_member_list_opt
                :   union_member_list[decl]
                        {
                            $$ = $decl;
                        }
                |   "void"
                        {
                            $$ = new AstNode(Token(TOK_CHILDREN));
                            $$->appendChild(new AstNode(Token(*$1)));
                        }
                |   /* empty */
                        {
                            $$ = NULL;
                        }
                ;

union_member_list
                :   union_member
                        {
                            $$ = new AstNode(Token(TOK_CHILDREN));
                            $$->appendChild($union_member);

                        }
                |   union_member_list[member_list] union_member
                        {
                            $member_list->appendChild($union_member);
                            $$ = $member_list;
                        }
                ;
union_member
                :   doxy_ml_comment_opt struct_member_options_list simple_data_type[decl_type] ident[decl_name] annotation_list_opt[annotations] semi_opt doxy_il_comment_opt
                        {
                            $$ = new AstNode(Token(TOK_CHILDREN));
                            $$->appendChild($decl_name);
                            $$->appendChild($decl_type);
                            $$->appendChild(new AstNode(Token(TOK_INT_LITERAL, new IntegerValue($struct_member_options_list))));
                            $$->appendChild($annotations);
                            $$->appendChild($doxy_ml_comment_opt);
                            $$->appendChild($doxy_il_comment_opt);
                        }
                ;

simple_data_type
                :   list_type
                        {
                            $$ = $list_type;
                        }
                |   array_type
                        {
                            $$ = $array_type;
                        }
                |   typename
                        {
                            $$ = $typename;
                        }

                ;

data_type       :   simple_data_type
                        {
                            $$ = $simple_data_type;
                        }
                |   struct_def
                        {
                            $$ = $struct_def;
                        }
                |   enum_def
                        {
                            $$ = $enum_def;
                        }
                ;

typename        :   ident
                        {
                            $$ = $ident;
                        }
                ;

name_opt        :   ident
                        {
                            $$ = $ident;
                        }
                |   /* empty */
                        {
                            $$ = NULL;
                        }
                ;

/*
 * TOK_LIST -> ( simple_data_type )
 */
list_type       :   "list" '<' simple_data_type '>'
                        {
                            $$ = new AstNode(*$1);
                            $$->appendChild($simple_data_type);
                        }
                ;

/*
 * TOK_ARRAY -> ( simple_data_type, int_const_expr )
 */
array_type      :   simple_data_type '[' int_const_expr ']'
                        {
                            $$ = new AstNode(Token(TOK_ARRAY));
                            $$->appendChild($simple_data_type);
                            $$->appendChild($int_const_expr);
                            $$->reverseExpr();
                        }
                ;

annotation_list_opt
                :   annotation_list
                        {
                            $$ = $annotation_list;
                        }
                |   /* empty */
                        {
                            $$ = NULL;
                        }
                ;

/*
 * TOK_CHILDREN -> ( annotation* )
 */
annotation_list :   annotation
                        {
                            $$ = new AstNode(Token(TOK_CHILDREN));
                            $$->appendChild($annotation);
                        }
                |   annotation_list[ann_list] annotation
                        {
                            $ann_list->appendChild($annotation);
                            $$ = $ann_list;
                        }
                ;

/*
 * TOK_ANNOTATION -> ( ident const_expr )
 */
annotation      :   '@' ident annotation_value_opt
                        {
                            $$ = new AstNode(Token(TOK_ANNOTATION));
                            $$->appendChild($ident);
                            $$->appendChild($annotation_value_opt);
                        }
                ;

annotation_value_opt
                :   '(' const_expr ')'
                        {
                            $$ = $const_expr;
                        }
                |   /* empty */
                        {
                            $$ = NULL;
                        }
                ;

const_expr      :   int_const_expr
                        {
                            $$ = $int_const_expr;
                        }
                |   string_literal
                        {
                            $$ = $string_literal;
                        }
                ;

doxy_ml_comment_opt
                :   doxy_ml_comment
                        {
                            $$ = $doxy_ml_comment;
                        }
                |   /* empty */
                        {
                            $$ = NULL;
                        }
                ;

doxy_ml_comment:   TOK_ML_COMMENT
                        {
                            // Merge string literals into one string. Remove first '\n'
                            Value * v = $TOK_ML_COMMENT->getValue();
                            StringValue * s = dynamic_cast<StringValue *>(v);
                            assert(s);
                            s = new StringValue(s->toString().substr(1, s->toString().size() - 1));
                            $TOK_ML_COMMENT->setValue(s);
                            $$ = new AstNode(*$TOK_ML_COMMENT);
                        }
                |   doxy_ml_comment[doxygen_ml_comment] TOK_ML_COMMENT
                        {
                            $$ =  mergeString(&$doxygen_ml_comment->getToken(), $2);
                        }
                ;

doxy_il_comment_opt
                :   doxy_il_comment_opt[doxygen_il_comment] TOK_IL_COMMENT
                        {
                            if($doxygen_il_comment)
                            {
                                $$ =  mergeString(&$doxygen_il_comment->getToken(), $2);
                            }
                            else
                            {
                                $$ =  new AstNode(*$2);
                            }
                        }
                |   /* empty */
                        {
                            $$ = NULL;
                        }
                ;

/*
 * TOK_EXPR -> ( expr )
 */
int_const_expr  :   expr
                        {
                            $$ = new AstNode(Token(TOK_EXPR, NULL, @1));
                            $$->appendChild($expr);
                        }
                ;

expr            :   int_value
                        {
                            $$ = $int_value;
                        }
                |   float_value
                        {
                            $$ = $float_value;
                        }
                |   ident
                        {
                            $$ = $ident;
                        }
                |   expr '+' expr
                        {
                            $$ = new AstNode(*$2);
                            $$->appendChild($1);
                            $$->appendChild($3);
                        }
                |   expr '-' expr
                        {
                            $$ = new AstNode(*$2);
                            $$->appendChild($1);
                            $$->appendChild($3);
                        }
                |   expr '*' expr
                        {
                            $$ = new AstNode(*$2);
                            $$->appendChild($1);
                            $$->appendChild($3);
                        }
                |   expr '/' expr
                        {
                            $$ = new AstNode(*$2);
                            $$->appendChild($1);
                            $$->appendChild($3);
                        }
                |   expr '%' expr
                        {
                            $$ = new AstNode(*$2);
                            $$->appendChild($1);
                            $$->appendChild($3);
                        }
                |   expr '&' expr
                        {
                            $$ = new AstNode(*$2);
                            $$->appendChild($1);
                            $$->appendChild($3);
                        }
                |   expr '|' expr
                        {
                            $$ = new AstNode(*$2);
                            $$->appendChild($1);
                            $$->appendChild($3);
                        }
                |   expr '^' expr
                        {
                            $$ = new AstNode(*$2);
                            $$->appendChild($1);
                            $$->appendChild($3);
                        }
                |   expr "<<" expr
                        {
                            $$ = new AstNode(*$2);
                            $$->appendChild($1);
                            $$->appendChild($3);
                        }
                |   expr ">>" expr
                        {
                            $$ = new AstNode(*$2);
                            $$->appendChild($1);
                            $$->appendChild($3);
                        }
                |   unary_expr
                            {
                                $$ = $unary_expr;
                            }
                |   '(' expr ')'
                        {
                            $$ = $2;
                            //$$->setLocation(@1, @3);
                        }
                ;

unary_expr      :   '+' expr %prec UNARY_OP
                        {
                            $$ = $expr;
                        }
                |   '-' expr %prec UNARY_OP
                        {
                            $$ = new AstNode(Token(TOK_UNARY_NEGATE, NULL, @1));
                            $$->appendChild($expr);
                        }
                |   '~' expr %prec UNARY_OP
                        {
                            $$ = new AstNode(*$1);
                            $$->appendChild($expr);
                        }
                ;

int_value       :   TOK_INT_LITERAL
                        {
                            $$ = new AstNode(*$TOK_INT_LITERAL);
                        }
                |   "true"
                        {
                            $$ = new AstNode(Token(TOK_INT_LITERAL, new IntegerValue(1), @1));
                        }
                |   "false"
                        {
                            $$ = new AstNode(Token(TOK_INT_LITERAL, new IntegerValue(0), @1));
                        }
                ;

float_value     :   TOK_FLOAT_LITERAL
                        {
                            $$ = new AstNode(*$TOK_FLOAT_LITERAL);
                        }
                ;

string_literal  :   TOK_STRING_LITERAL
                        {
                            $$ = new AstNode(*$TOK_STRING_LITERAL);
                        }
                |   string_literal[string] TOK_STRING_LITERAL
                        {
                            $$ =  mergeString(&$string->getToken(), $TOK_STRING_LITERAL);
                        }
                ;

ident           :   TOK_IDENT
                        {
                            $$ = new AstNode(*$TOK_IDENT);
                        }
                ;

ident_opt       :   ident
                        {
                            $$ = $ident;
                        }
                |   /* empty */
                        {
                            $$ = NULL;
                        }
                ;

%%

/* code goes here */

static int yylex(YYSTYPE * lvalp, YYLTYPE * yylloc, ErpcLexer * lexer)
{
    Token * token = lexer->getNextToken();
    if (!token)
    {
        return END;
    }

    lvalp->m_token = token;
    *yylloc = token->getLocation();
    return token->getToken();
}

static void yyerror(YYLTYPE * yylloc, ErpcLexer * lexer, AstNode ** resultAST, const char * error)
{
    throw syntax_error(format_string("file %s:%d:%d: %s\n", lexer->getFileName().c_str(),
        yylloc->m_firstLine, yylloc->m_firstChar, error));
}

const char * get_token_name(int tok)
{
    return yytname[YYTRANSLATE(tok)];
}

static AstNode * mergeString(Token * t1, Token * t2)
{
    // Merge string literals into one string.
    Value * v1 = t1->getValue();
    Value * v2 = t2->getValue();

    if (v1 && v2)
    {
        StringValue * s1 = dynamic_cast<StringValue *>(v1);
        StringValue * s2 = dynamic_cast<StringValue *>(v2);
        if (s1 && s2)
        {
            StringValue * s = new StringValue(std::string(s1->toString()) + std::string(s2->toString()));
            token_loc_t newTokLocation = mergeLocation(t1->getLocation(), t2->getLocation());
            return new AstNode(Token(t2->getToken(), s, newTokLocation));
        }
    }

    return new AstNode(*t2);
}

token_loc_t mergeLocation(const token_loc_t & l1, const token_loc_t & l2)
{
    token_loc_t newTokLocation;
    newTokLocation.m_firstLine = l1.m_firstLine;
    newTokLocation.m_firstChar = l1.m_firstChar;
    newTokLocation.m_lastLine = l2.m_lastLine;
    newTokLocation.m_lastChar = l2.m_lastChar;
    return newTokLocation;
}
