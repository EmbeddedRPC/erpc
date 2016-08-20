cpptempl 2.0
=================
This is a template engine for C++.

| Original author: Ryan Ginstrom
| Additions by Martinho Fernandes
| Extensive modifications by Chris Reed
| Additional changes by Dušan Červenka

Requirements
==================
1. The boost library.
2. Compiler with C++11 support.

Usage
=======================
Complete example::

    std::string text = "{% if item %}{$item}{% endif %}\n"
        "{% if thing %}{$thing}{% endif %}" ;
    cpptempl::data_map data ;
    data["item"] = "aaa" ;
    data["thing"] = "bbb" ;

    std::string result = cpptempl::parse(text, data) ;

There are two main functions that users will interact with, both called ``parse()``. They
are in the ``cpptempl`` namespace. The prototypes for these functions are as follows::

    std::string parse(const std::string &templ_text, data_map &data);

    void parse(std::ostream &stream, const std::string &templ_text, data_map &data);

Both take the template text as a ``std::string`` along with a ``data_map`` that has the variable
values to substitute. One form of ``parse()`` simply returns the result of the template
as another ``std::string``.

The other form takes a ``std::ostream`` reference as the first argument and writes the
template output to that stream. This form is actually more memory efficient, because it
does not build up the complete template output in memory as a string before returning it.

Another way to use cpptempl is to create a ``DataTemplate`` object. This is helpful if you
need to use a template more than once because it only parses the template text a single
time. Example::

    cpptempl::DataTemplate tmpl(text);
    std::string result = tmpl.eval(data);

As with the ``parse()`` functions, there are two overloads of the ``DataTemplate::eval()``
method. One returns the template output as a ``std::string``, while the other accepts a
``std::ostream`` reference to which the output will be written.

Syntax
=================
:Variables:
    ``{$variable_name}``
:Loops:
    ``{% for person in people %}Name: {$person.name}{% endfor %}``<br/>
    ``{% for person in people if person.age > 30 %}Name: {$person.name}{% endfor %}``
:If:
    ``{% if foo %}gorilla{% elif bar %}bonobo{% else %}orangutan{% endif %}``
:Def:
    ``{% def foobar(x) %}The value of x is {$x} today{% enddef %}``
:Set:
    ``{% set foo = 'fun' if on_vacation else 'work' %}``
:Comment:
    ``{# comment goes here #}``

There are three types of statements: variable substitutions, control statements, and
comments.

For loops iterate over the ``data_list`` selected by the key path named after the "in"
keyword. An optional Boolean filter expression can be added after an "if" keyword.

If statements conditionally execute one section or another of the template based on
the value of one or more Boolean expressions. As one would expect, if statements may
have zero or more elif branches and one optional else branch.

Def statements are used to create subtemplates. They are described in detail below,
in the Subtemplates section.

Set statements allow one to assign a key path to the value of an expression evaluated
at runtime. The value expression may be any expression supported in a variable
substitution, including subtemplate invocations.

Empty statements are also allowed, as are statements that consist only of single-line
comments (see the section on comments below).

Anywhere a variable name is used, you may use a dotted key path to walk through the
keys of nested ``data_map`` objects. The leftmost key name is looked up in the
``data_map`` that was passed into the ``parse()`` or ``DataTemplate::eval()`` function
call, which is called the "global data map". If the specified key does not exist, its
value is the empty string.

Whitespace, including newlines, is ignored in all statements including variable
substitutions. The only exception is in key paths. There may not be any space between
the dots and the key names. So ``{$var.name}`` and ``{$ var.name }`` are equivalent,
but ``{$var . name}`` is invalid.

Expressions
-----------
If statements, variable substitution blocks, for loop filters, and set statements accept
arbitrary expressions.

Operators for expressions are shown in the following table, listed in order of highest
to lowest precedence. x and y are subexpressions, and p is a predicate subexpression.

==================  =======================================================
``sub(x,y,...)``    Subtemplate invocation with parameters
``(x)``             Parenthesized subexpression
``!x``              True if x is empty or false
``-x``              Integer negation
``x || y``          Boolean or
``x && y``          Boolean and
``x == y``          Equality
``x != y``          Inequality
``x > y``           Greater than
``x >= y``          Greater or equal
``x < y``           Less than
``x <= y``          Less or equal
``x + y``           Add
``x - y``           Subtract
``x * y``           Multiply
``x / y``           Divide
``x % y``           Modulus
``x & y``           String concatenation
``x if p else y``   Inline if statement
==================  =======================================================

Note that the keywords "not", "and", and "or" are also supported in place of the C-style
operators. Thus, ``not (x and y)`` is completely equivalent to ``!(x && y)``.

In all cases where an expression produces a Boolean result, an empty string is considered
to be false while a non-empty string is true. Similarly, an integer value of 0 is false and
a non-zero integer is true.

The Boolean OR operator (``||`` or ``or``) does not produces a Boolean result. Instead, it
returns the value of its non-empty, or true, operand. If both operands are non-empty, then
it returns the left operand's value. Thus, ``false or 'lizard'`` returns ``'lizard'``.

Comparison operators such as ``>`` or ``<`` can be used on both integers and strings. Strings
are compared alphabetically. Only if both operands are integers will they be compared
numerically.

The binary arithmetic operators will convert their operands to a signed integer, if not
one already, before performing the operation. The result is always an integer.

There are also a few built-in functions that may be used in expressions.

======================  ===========================================================
``count(x)``            Returns the number of items in the specified list.
``defined(x)``          Returns true if the key path specifies an existing key.
``empty(x)``            True if the variable path x is the empty string.
``int(x)``              Coerce to integer value
``str(x)``              Coerce to string value
``addIndent(x,y)``      If y is not empty, it will add x-spaces on begin of y.
``upper(x)``            Convert the string to uppercase.
``lower(x)``            Convert the string to lowercase.
======================  ===========================================================

Supported value types in expressions:

==============  ===================================================================
``key``         Name of key in top-level data_map (simple case of key path).
``key.path``    Dotted path of data_map keys.
``123``         Signed integer (32-bit).
``0x123``       Hexadecimal signed integer.
``true``        Boolean true.
``false``       Boolean false.
``"text"``      String literal with double quotes.
``'text'``      String literal with single quotes.
==============  ===================================================================

If the expression in an if statement produces a non-Boolean value such as a string,
then the expression is considered true if the value is not empty.

String literals may include backslash escape sequences as in C/C++. All the standard
C single-character escapes are supported. Any other character that is escaped results
in that character.

Hexadecimal character code escapes are also supported. The format is, again,
the same as in C. The first escape character must be "x" and is followed by one or
more hexadecimal digits. Hex escape sequences have no length limit and terminate
at the first character that is not a valid hexadecimal digit. If the value
represented by the escape sequence does not fit into an 8-bit character, only its
lower 8 bits are inserted into the output.

Loop variable
-------------
Inside a for statement block, a "loop" map variable is defined with these keys:

==========================  =======================================================
``index``                   Base-1 current index of loop
``index0``                  Base-0 current index of loop
``first``                   True if this is the first iteration through the loop
``last``                    True if this is the last iteration through the loop
``even``                    True on all even iterations, starting with the second
``odd``                     True on all odd iterations, starting with the first
``count``                   Total number of elements in the list
``addNewLineIfNotLast``     This will add new line when it is not last iteration through the loop.
==========================  =======================================================

The "loop" variable remains available after the for statement completes. It will also be
accessible in the data map after the template finishes execution. Of course, a subsequent
for loop will change the "loop" variable's contents.

The "loop" variable works more or less as expected with nested for loops. During the inner
loop, the outer loop's "loop" variable is not accessible. But once the inner loop completes,
the "loop" variable switches back to the outer loop's values. If you need access to the
outer loop's "loop" variable inside the inner loop, use a set statement to assign it to
another key path::

    {% for person in people %}
        {% set person_loop = loop %}
        {% for name in person.friends %}
            {% if person_loop.last %}...{% endif %}
        {% endfor %}
    {% endfor %}

For loop filtering
------------------

A for loop statement may optionally contain an "if" keyword followed by a Boolean expression.
The expression will be evaluated for each list element. The for loop will iterate over only
those list elements for which the expression evaluated to true (non-empty).

Both the for loop iterator variable and "loop" variable are available in the filter expression.
Inside the body of the for loop, the "loop" variable will have the correct values for iterating
over the filtered list.

For instance::

    {% for x in mylist if loop.odd %}
        {$loop.index}: {$x.value}
    {% endfor %}

In this example, if mylist has 10 elements, then the "loop" variable in the filter expression
will have a "count" key of 10. Filtering will reduce the list to 5 elements (the odd elements).
Inside the for loop body, the "loop" variable will have a "count" key of 5.

Newline control
---------------
Control statements on a line by themselves will eat the newline following the statement.
This also applies for cases where the open brace of the control statement is at the
start of a line and the close brace is at the end of another line. In addition, this will
work for multiple consecutive control statements as long as they completely occupy the
lines on which they reside with no intervening space.

For additional control over newlines, you can place a ">" character, called the newline
eliding modifier (or just newline elider), as the last character before the closing brace
sequence of a variable substitution or control statement  (i.e., ``{% ... >%}`` or
``{$ ... >}``). This will cause a newline that immediately follows the "}" to be omitted
from the output. If a newline does not immediately follow the close brace, this option will
have no effect.

You may combine an empty or comment-only statement with the newline elider to form a
"newline-eater" statement. It looks like ``{%>%}``, or ``{% -- comment >%}`` with a
comment. This can be very useful is situations where you want to break a complex sequence
of statements into multiple lines for better maintainability.

Variable substitution statements also support a special newline control option enabled by
placing a ">" character after the "$", like ``{$> ... }``. If the substitution expression
evaluated to an empty string, then the next newline that would normally be output will be
removed. This works even when there are multiple other statements between the variable
substitution statement and the next newline.

Comments
--------
Control statements inside ``{% %}`` brackets may be commented with single-line comments. A
single-comment is started with ``--`` and runs to either the close bracket of the statement
or the next line as demonstrated here::

    {%
      for person -- loop variable
      in people -- list to loop over
    %}
    Name: {$person.name}
    {% endfor -- end the person loop %}

You may also put comments in ``{# #}`` brackets. These comments may span multiple lines
and contain only comment text. They will not be copied to the output under any circumstances.
As with all control statements, if such a comment is on a line by itself, the newline
following the comment is absorbed and not reproduced in the output.

Types
==================
All values are stored in a ``data_ptr`` variant object.

These are the supported data types and associated ``Data`` subclasses:

=============== ================
``std::string`` ``DataValue``
``bool``        ``DataBool``
``int``         ``DataInt``
``data_list``   ``DataList``
``data_map``    ``DataMap``
subtemplate     ``DataTemplate``
=============== ================

You normally do not need to use the ``Data`` subclasses directly. ``data_ptr`` objects can
be assigned any of the supported types directly. The same applies to ``data_map`` values.
All other types are converted to strings using ``boost::lexical_cast`` when set in
a ``data_ptr`` or ``data_map``.

Bool values will result in either "true" or "false" when substituted. ``data_list`` or
``data_map`` values will cause a ``TemplateException`` to be thrown if you attempt to
substitute them as a variable.

Subtemplates
==================
Subtemplates are a special type. They allow you to define a template once and reuse
it multiple times by substituting it as a variable. A subtemplate is completely
re-evaluated every time it is substituted, using the current values of any variables.
This is particularly useful within a loop.

Subtemplates may take parameters. These are defined when the subtemplate is created
via either of the methods described below. When a subtemplate is used in a variable
substitution in a template, you may pass values for its parameters just as you would
for a function call.

There are two ways to define a subtemplate. The first is to use the ``make_template()``
function. It takes a ``std::string`` and returns a subtemplate ``data_ptr``, which may then
be stored in a ``data_map``. It may also optionally be provided a vector of parameter
name strings.

The second way to create a subtemplate is to use the def statement within a template.
Def statements define a subtemplate with the template contents between the def and
enddef statements. The subtemplate is stored in the named variable, which may be a path.
The elements of the key path will be created if they do not exist. As with all
subtemplates, the contents are evaluated at the point where the def variable is used.

Note that the new subtemplate will remain in the global data map after the template is
done executing. This means it can be extracted or passed to another template.

The parameters for a subtemplate may be specified in a def statement. This is done by
listing the parameter names in parentheses after the subtemplate's key path, as shown
in this example::

    {% def mytmpl(foo, bar) %}
    foo={$foo}
    bar={$bar}
    {% enddef %}

To use this subtemplate, you would do something like this::

    {$mytmpl("a", "b")}

This variable substitution expression will pass the string constants "a" and "b" for the
subtemplate parameters "foo" and "bar", respectively. During the evaluation of the
subtemplate, parameter variables will be set to the specified values. If there is
already a key in the global data map with the same name as a parameter, the parameter
will shadow the global key. The global data map is not modified permanently. Any
parameter keys will be restored to the original state, including being undefined, once the
subtemplate evaluation is completed. Any expression may be used to generate the parameter
values.

Handy Functions
========================
``make_data()`` : Feed it a bool, int, string, data_map, or data_list to create a data entry.
Example::

    data_map person ;
    person["name"] = make_data("Bob") ;
    person["occupation"] = make_data("Plumber") ;
    data_map data ;
    data["person"] = make_data(person) ;
    std::string result = parse(templ_text, data) ;

Note that using ``make_data()`` is only one method. You may also assign values directly to
data_map elements::

    data_map person;
    person["age"] = 35;
    person["name"] = "Fred";
    person["has_pet"] = true;

``make_template()`` : Creates a subtemplate from a std::string. The template string is
passed as the first parameter. An optional pointer to a std::string vector can be provided
as a second parameter to specify the names of subtemplate parameters.

Example of creating a subtemplate with params::

    string_vector params{"foo", "bar"};
    data_ptr subtmpl = make_template(template_text, &params);

Errors
==================
Any template errors will result in a ``TemplateException`` being thrown.

The ``TemplateException`` class is a subclass of ``std::exception``, so it has a ``what()``
method. This method will return an error string describing the error. In most cases,
the message will be prefixed with the line number of the input template that caused the
error.

Known Issues
==================
- "defined" pseudo-function is broken, always returning true.
- Stripping of newlines after statements on a line by themselves does not work correctly
  for CRLF line endings.
- The only way to output the variable substitution or control statement open block
  sequences is to substitute a string literal with that value, i.e. ``{$"{%"}``.

Copyright
==================
| Copyright (c) 2010-2014 Ryan Ginstrom
| Copyright (c) 2014 Martinho Fernandes
| Copyright (c) 2014-2016 Freescale Semiconductor, Inc.

License
==================

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
