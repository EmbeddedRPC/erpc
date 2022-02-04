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

#pragma once

#include <iostream>
#include <ostream>
#include <sstream>
#ifndef _MSC_VER
#include <boost/locale.hpp>
#else
#include "windows.h"
#include "winnls.h" // unicode-multibyte conversion

#include <boost/scoped_array.hpp>
#endif

inline std::wstring utf8_to_wide(const std::string &text)
{
#ifndef _MSC_VER
    return boost::locale::conv::to_utf<wchar_t>(text, "UTF-8");
#else
    // Calculate the required length of the buffer
    const size_t len_needed = ::MultiByteToWideChar(CP_UTF8, 0, text.c_str(), (UINT)(text.length()), NULL, 0);
    boost::scoped_array<wchar_t> buff(new wchar_t[len_needed + 1]);
    const size_t num_copied = ::MultiByteToWideChar(CP_UTF8, 0, text.c_str(), text.size(), buff.get(), len_needed + 1);
    return std::wstring(buff.get(), num_copied);
#endif
}

inline std::string wide_to_utf8(const std::wstring &text)
{
#ifndef _MSC_VER
    return boost::locale::conv::from_utf<>(text, "UTF-8");
#else
    const size_t len_needed =
        ::WideCharToMultiByte(CP_UTF8, 0, text.c_str(), (UINT)(text.length()), NULL, 0, NULL, NULL);
    boost::scoped_array<char> buff(new char[len_needed + 1]);
    const size_t num_copied =
        ::WideCharToMultiByte(CP_UTF8, 0, text.c_str(), (UINT)(text.length()), buff.get(), len_needed + 1, NULL, NULL);
    return std::string(buff.get(), num_copied);
#endif
}

namespace std {
inline ostream &operator<<(ostream &out, const wchar_t *value)
{
    wstring text(value);
    out << wide_to_utf8(text);
    return out;
}

inline ostream &operator<<(ostream &out, const wstring &value)
{
    out << wide_to_utf8(value);
    return out;
}
} // namespace std
