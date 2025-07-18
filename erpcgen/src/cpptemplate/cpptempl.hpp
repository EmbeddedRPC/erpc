// Copyright (c) 2010-2014 Ryan Ginstrom
// Copyright (c) 2014 Martinho Fernandes
// Copyright (c) 2014-2016 Freescale Semiconductor, Inc.
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

#pragma once

#ifdef _WIN32
#pragma warning(disable : 4996) // 'std::copy': Function call with parameters that may be unsafe - this call relies on
                                // the caller to check that the passed values are correct. To disable this warning, use
                                // -D_SCL_SECURE_NO_WARNINGS. See documentation on how to use Visual C++ 'Checked
                                // Iterators'
#pragma warning(disable : 4512) // 'std::copy': Function call with parameters that may be unsafe - this call relies on
                                // the caller to check that the passed values are correct. To disable this warning, use
                                // -D_SCL_SECURE_NO_WARNINGS. See documentation on how to use Visual C++ 'Checked
                                // Iterators'
#define NOEXCEPT                // hide unsupported noexcept keyword under VC++
#else
#define NOEXCEPT noexcept
#endif // NOEXCEPT

#if __MINGW32__
#define NOTHROW throw() // add throw() keyword under MinGW
#else
#define NOTHROW
#endif // NOTHROW

#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>

namespace cpptempl {

// various typedefs
class data_ptr;
class data_map;
class DataMap;
class DataTemplate;

typedef std::vector<data_ptr> data_list;

// data classes
class Data
{
public:
    virtual ~Data() = default;
    virtual bool empty() = 0;
    virtual std::string getvalue();
    virtual data_list &getlist();
    virtual data_map &getmap();
    virtual int getint() const;
    virtual void dump(int indent = 0) = 0;
};

class DataBool : public Data
{
    bool m_value;

public:
    DataBool(bool value)
    : m_value(value)
    {
    }
    std::string getvalue();
    virtual int getint() const;
    bool empty();
    virtual void dump(int indent = 0);
};

class DataInt : public Data
{
    int m_value;

public:
    DataInt(int value)
    : m_value(value)
    {
    }
    DataInt(unsigned int value)
    : m_value(value)
    {
    }
    std::string getvalue();
    virtual int getint() const;
    bool empty();
    virtual void dump(int indent = 0);
};

class DataValue : public Data
{
    std::string m_value;

public:
    DataValue(const std::string &value)
    : m_value(value)
    {
    }
    DataValue(std::string &&value)
    : m_value(std::move(value))
    {
    }
    std::string getvalue();
    virtual int getint() const;
    bool empty();
    virtual void dump(int indent = 0);
};

class DataList : public Data
{
    data_list m_items;

public:
    DataList(const data_list &items)
    : m_items(items)
    {
    }
    DataList(data_list &&items)
    : m_items(std::move(items))
    {
    }
    data_list &getlist();
    bool empty();
    void dump(int indent = 0);
};

class data_ptr
{
public:
    data_ptr() {}
    template <typename T>
    data_ptr(const T &data)
    {
        this->operator=(data);
    }
    data_ptr(DataBool *data);
    data_ptr(DataInt *data);
    data_ptr(DataValue *data);
    data_ptr(DataList *data);
    data_ptr(DataMap *data);
    data_ptr(DataTemplate *data);
    data_ptr(const data_ptr &data) { ptr = data.ptr; }
    data_ptr(data_ptr &&data) { ptr = std::move(data.ptr); }
    data_ptr &operator=(const data_ptr &data)
    {
        ptr = data.ptr;
        return *this;
    }
    data_ptr &operator=(data_ptr &&data)
    {
        ptr = std::move(data.ptr);
        return *this;
    }
    data_ptr &operator=(std::string &&data);
    data_ptr &operator=(data_map &&data);
    data_ptr &operator=(data_list &&data);
    template <typename T>
    void operator=(const T &data);
    void push_back(const data_ptr &data);
    virtual ~data_ptr() {}
    Data *operator->() { return ptr.get(); }
    std::shared_ptr<Data> get() { return ptr; }
    bool is_template() const;

private:
    std::shared_ptr<Data> ptr;
};

class data_map
{
public:
    class key_error : public std::runtime_error
    {
    public:
        key_error(const std::string &msg)
        : std::runtime_error(msg)
        {
        }
    };

    data_map()
    : data()
    , parent(nullptr)
    {
    }
    data_ptr &operator[](const std::string &key);
    bool empty();
    bool has(const std::string &key);
    data_ptr &parse_path(const std::string &key, bool create = false);
    void set_parent(data_map *p) { parent = p; }

private:
    std::unordered_map<std::string, data_ptr> data;
    data_map *parent;

    friend class DataMap;
};

class DataMap : public Data
{
    data_map m_items;

public:
    DataMap(const data_map &items)
    : m_items(items)
    {
    }
    DataMap(data_map &&items)
    : m_items(std::move(items))
    {
    }
    data_map &getmap();
    bool empty();
    void dump(int indent = 0);
};

template <>
void data_ptr::operator=(const bool &data);
template <>
void data_ptr::operator=(const int &data);
template <>
void data_ptr::operator=(const unsigned int &data);
template <>
void data_ptr::operator=(const std::string &data);
template <>
void data_ptr::operator=(const data_map &data);
template <>
void data_ptr::operator=(const data_list &data);
template <typename T>
void data_ptr::operator=(const T &data)
{
    std::ostringstream ss;
    ss << data;
    this->operator=(ss.str());
}

// Custom exception class for library errors
class TemplateException : public std::exception
{
    uint32_t m_line;
    std::string m_reason;

public:
    TemplateException(std::string reason)
    : std::exception()
    , m_line(0)
    , m_reason(reason)
    {
    }
    TemplateException(size_t line, std::string reason);
    TemplateException(const TemplateException &other) = default;
    TemplateException &operator=(const TemplateException &other) = default;
    virtual ~TemplateException() = default;

    void set_reason(std::string reason) { m_reason = reason; }
    void set_line_if_missing(size_t line);

    virtual const char *what() const NOEXCEPT NOTHROW { return m_reason.c_str(); }
};

// convenience functions for making data objects
inline data_ptr make_data(bool val)
{
    return data_ptr(new DataBool(val));
}
inline data_ptr make_data(int val)
{
    return data_ptr(new DataInt(val));
}
inline data_ptr make_data(unsigned int val)
{
    return data_ptr(new DataInt(val));
}
inline data_ptr make_data(std::string &val)
{
    return data_ptr(new DataValue(val));
}
inline data_ptr make_data(std::string &&val)
{
    return data_ptr(new DataValue(val));
}
inline data_ptr make_data(data_list &val)
{
    return data_ptr(new DataList(val));
}
inline data_ptr make_data(data_list &&val)
{
    return data_ptr(new DataList(val));
}
inline data_ptr make_data(data_map &val)
{
    return data_ptr(new DataMap(val));
}
inline data_ptr make_data(data_map &&val)
{
    return data_ptr(new DataMap(val));
}
template <typename T>
data_ptr make_data(const T &val)
{
    std::ostringstream ss;
    ss << val;
    return data_ptr(ss.str());
}

void dump_data(data_ptr data);

namespace impl {

// node classes
class Node;
typedef std::shared_ptr<Node> node_ptr;
typedef std::vector<node_ptr> node_vector;

} // namespace impl

// List of param names.
typedef std::vector<std::string> string_vector;

class DataTemplate : public Data
{
    impl::node_vector m_tree;
    string_vector m_params;

public:
    DataTemplate(const std::string &templateText);
    DataTemplate(const impl::node_vector &tree)
    : m_tree(tree)
    {
    }
    DataTemplate(impl::node_vector &&tree)
    : m_tree(std::move(tree))
    {
    }
    virtual std::string getvalue();
    virtual bool empty();
    std::string eval(data_map &data, data_list *param_values = nullptr);
    void eval(std::ostream &stream, data_map &data, data_list *param_values = nullptr);
    string_vector &params() { return m_params; }
    void dump(int indent = 0);
};

inline data_ptr make_template(const std::string &templateText, const string_vector *param_names = nullptr)
{
    DataTemplate *t = new DataTemplate(templateText);
    if (param_names)
    {
        t->params() = *param_names;
    }
    return data_ptr(t);
}

// The big daddy. Pass in the template and data,
// and get out a completed doc.
void parse(std::ostream &stream, const std::string &templ_text, data_map &data);
std::string parse(const std::string &templ_text, data_map &data);
} // namespace cpptempl
