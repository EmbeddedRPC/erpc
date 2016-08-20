/*
 * Copyright (c) 2014-2015, Freescale Semiconductor, Inc.
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
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
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

#include "Generator.h"
#include "format_string.h"
#include <string.h>
#include <boost/filesystem.hpp>

using namespace erpcgen;
using namespace cpptempl;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

Generator::Generator(InterfaceDefinition *def)
: m_def(def)
, m_globals(&(m_def->getGlobals()))
{
}

void Generator::openFile(std::ofstream &fileOutputStream, const std::string &fileName)
{
    boost::filesystem::path outputDir = m_def->getOutputDirectory();
    if (!outputDir.empty())
    {
        // TODO: do we have to create a copy of the outputDir here? Doesn't make sense...
        boost::filesystem::path dir(outputDir);
        if (!boost::filesystem::is_directory(dir))
        {
            // Create_directories function return false also when it create new directory.
            // It is in case, when directory ends with slash. For these case is better use is_directory for check if
            // directories are created.
            boost::filesystem::create_directories(dir);
            if (!boost::filesystem::is_directory(dir))
            {
                throw std::runtime_error(format_string("could not create directory path '%s'", outputDir.c_str()));
            }
        }
    }
    std::string filePathWithName = (outputDir / fileName).string();
    // Open file.
    fileOutputStream.open(filePathWithName, std::ios::out | std::ios::binary);
    if (!fileOutputStream.is_open())
    {
        throw std::runtime_error(format_string("could not open output file '%s'", filePathWithName.c_str()));
    }
}

void Generator::generateOutputFile(const std::string &fileName,
                                   const std::string &templateName,
                                   data_map &templateData,
                                   const char *const kParseFile)
{
    std::ofstream fileOutputStream;
    openFile(fileOutputStream, fileName);

    // Run template and write output to output files. Catch and rethrow template exceptions
    // so we can add the name of the template that caused the error to aid in debugging.
    try
    {
        parse(fileOutputStream, kParseFile, templateData);
        fileOutputStream.close();
    }
    catch (TemplateException &e)
    {
        throw TemplateException(format_string("Template %s: %s", templateName.c_str(), e.what()));
    }
}

std::string Generator::stripExtension(const std::string &filename)
{
    auto result = filename.rfind('.');
    if (result != std::string::npos)
    {
        return filename.substr(0, result);
    }
    else
    {
        return filename;
    }
}
