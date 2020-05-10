/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "Sniffer.h"
#include "erpc_c/infra/erpc_message_buffer.h"
#include "Logging.h"
#include "annotations.h"
#include <boost/algorithm/string.hpp>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <iomanip>

using namespace erpcgen;
using namespace erpcsniffer;
using namespace erpc;
using namespace cpptempl;
using namespace std;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

erpc_status_t Sniffer::run()
{
    Log::info("receiving messages\n");
    uint64_t n = 0;
    ofstream outputFileStream;

    if (m_outputFilePath)
    {
        openFile(outputFileStream);
    }

    chrono::_V2::system_clock::time_point previousTime(chrono::nanoseconds(0));

    while (m_quantity == 0 || m_quantity > n)
    {
        ++n;

        // Receive message.
        uint8_t buf[1024];
        MessageBuffer message(buf, 1024);
        erpc_status_t result = m_transport->receive(&message);
        chrono::_V2::system_clock::time_point currentTime = chrono::high_resolution_clock::now();
        if (result)
        {
            Log::error("message error %d\n", result);
            return result;
        }

        Log::info("message received\n");
        m_codec->setBuffer(message);

        // Save time when message was received.
        time_t now = chrono::system_clock::to_time_t(currentTime);
        char buffer[80];
        struct tm *timeinfo;
        timeinfo = localtime(&now);
        strftime(buffer, 80, "%T %D", timeinfo);

        // Time difference between current and previous received message.
        string timeDifference =
            format_string("%d", (previousTime.time_since_epoch() == chrono::nanoseconds(0)) ?
                                    0 :
                                    chrono::duration_cast<chrono::nanoseconds>(currentTime - previousTime).count());
        uint32_t timeDifferenceSize = timeDifference.size();
        uint32_t countSpaces = floor((timeDifferenceSize - 1) / 3);
        for (uint32_t i = 1; i <= countSpaces; ++i)
        {
            timeDifference = timeDifference.insert(timeDifferenceSize - i * 3, " ");
        }

        previousTime = currentTime;

        // Analyze message.
        string analyzedMessage = format_string("%lu. ", n);
        result = analyzeMessage(analyzedMessage, timeDifference.c_str(), buffer);
        if (result)
        {
            Log::error("analyzed message error  %d\n", result);
            return result;
        }

        // Print analyzed message.
        printf("%s", analyzedMessage.c_str());

        // Record message into file.
        if (outputFileStream.is_open())
        {
            outputFileStream.write(analyzedMessage.c_str(), analyzedMessage.size());
        }
    }

    if (m_outputFilePath)
    {
        outputFileStream.close();
    }

    return kErpcStatus_Success;
}

void Sniffer::openFile(ofstream &outputFileStream)
{
    boost::filesystem::path outputFilePath = m_outputFilePath;
    boost::filesystem::path outputPath = outputFilePath.parent_path();

    if (!outputPath.empty())
    {
        // TODO: do we have to create a copy of the outputDir here? Doesn't make sense...
        boost::filesystem::path dir(outputPath);
        if (!boost::filesystem::is_directory(dir))
        {
            // Create_directories function return false also when it create new directory.
            // It is in case, when directory ends with slash. For these case is better use is_directory for check if
            // directories are created.
            boost::filesystem::create_directories(dir);
            if (!boost::filesystem::is_directory(dir))
            {
                throw runtime_error(format_string("could not create directory path '%s'", outputPath.c_str()));
            }
        }
    }

    // Open file.
    outputFileStream.open(m_outputFilePath, ofstream::out | ofstream::binary);
    if (!outputFileStream.is_open())
    {
        throw runtime_error(format_string("could not open output file '%s'", m_outputFilePath));
    }
}

erpc_status_t Sniffer::readNullFlag(StructMember *structMember, string &nullFlag)
{
    if (structMember && structMember->findAnnotation(NULLABLE_ANNOTATION, Annotation::kC) != nullptr)
    {
        bool nullF;
        m_codec->readNullFlag(&nullF);
        if (nullF)
        {
            nullFlag = "NULL";
        }
    }
    return m_codec->getStatus();
}

erpc_status_t Sniffer::readSharedAddress(StructMember *structMember, string &address)
{
    erpc_status_t err = kErpcStatus_Success;
    if (structMember->findAnnotation(SHARED_ANNOTATION, Annotation::kC) != nullptr)
    {
        uintptr_t ptr;
        m_codec->readPtr(&ptr);
        err = m_codec->getStatus();
        stringstream sstream;
        sstream << hex << ptr;
        address = "0x" + sstream.str();
    }
    return err;
}

uint8_t Sniffer::countSpaces(uint64_t value)
{
    // log(0) NaN; log(1) = 0 => value < 2 = 2
    return ceil(log((value < 2) ? 2 : value) / log(10));
}

void Sniffer::addSpaces(string &text, uint spacesCount)
{
    string indent = "\n";
    indent = indent.append(spacesCount, ' ');
    boost::replace_all(text, "\n", indent);
}

erpc_status_t Sniffer::parseDataType(DataType *dataType, string &parsedDataInfo)
{
    erpc_status_t err;
    switch (dataType->getDataType())
    {
        case DataType::_data_type::kAliasType:
        {
            AliasType *aliasType = dynamic_cast<AliasType *>(dataType);
            assert(aliasType);
            string parseDataInfo;
            err = parseDataType(aliasType->getElementType(), parseDataInfo);
            if (err)
            {
                return err;
            }

            parsedDataInfo = format_string("AliasType '%s':\n", aliasType->getName().c_str()) + parseDataInfo;
            addSpaces(parsedDataInfo, 2);
            break;
        }
        case DataType::_data_type::kArrayType:
        {
            ArrayType *arrayType = dynamic_cast<ArrayType *>(dataType);
            assert(arrayType);
            uint32_t arraySize = arrayType->getElementCount();
            for (uint32_t i = 0; i < arraySize; ++i)
            {
                string parseDataInfo;
                err = parseDataType(arrayType->getElementType(), parseDataInfo);
                if (err)
                {
                    return err;
                }
                parseDataInfo =
                    format_string("[%u]:", i).append(countSpaces(arraySize) - countSpaces(i + 1), ' ') + parseDataInfo;
                addSpaces(parseDataInfo, countSpaces(arraySize) + 3);
                if (i > 0)
                {
                    parsedDataInfo += "\n";
                }
                parsedDataInfo += parseDataInfo;
            }
            break;
        }
        case DataType::_data_type::kBuiltinType:
        {
            parsedDataInfo = " value: ";
            BuiltinType *builtinType = dynamic_cast<BuiltinType *>(dataType);
            assert(builtinType);
            switch (builtinType->getBuiltinType())
            {
                case BuiltinType::_builtin_type::kBoolType:
                {
                    bool value;
                    m_codec->read(&value);
                    if ((err = m_codec->getStatus()))
                    {
                        return err;
                    }
                    parsedDataInfo = "bool" + parsedDataInfo + ((value) ? "true" : "false");
                    break;
                }
                case BuiltinType::_builtin_type::kInt8Type:
                {
                    int8_t value;
                    m_codec->read(&value);
                    if ((err = m_codec->getStatus()))
                    {
                        return err;
                    }
                    parsedDataInfo = "int8_t" + parsedDataInfo + format_string("%d", value);
                    break;
                }
                case BuiltinType::_builtin_type::kInt16Type:
                {
                    int16_t value;
                    m_codec->read(&value);
                    if ((err = m_codec->getStatus()))
                    {
                        return err;
                    }
                    parsedDataInfo = "int16_t" + parsedDataInfo + format_string("%d", value);
                    break;
                }
                case BuiltinType::_builtin_type::kInt32Type:
                {
                    int32_t value;
                    m_codec->read(&value);
                    if ((err = m_codec->getStatus()))
                    {
                        return err;
                    }
                    parsedDataInfo = "int32_t" + parsedDataInfo + format_string("%d", value);
                    break;
                }
                case BuiltinType::_builtin_type::kInt64Type:
                {
                    int64_t value;
                    m_codec->read(&value);
                    if ((err = m_codec->getStatus()))
                    {
                        return err;
                    }
                    parsedDataInfo = "int64_t" + parsedDataInfo + format_string("%ld", value);
                    break;
                }
                case BuiltinType::_builtin_type::kUInt8Type:
                {
                    uint8_t value;
                    m_codec->read(&value);
                    if ((err = m_codec->getStatus()))
                    {
                        return err;
                    }
                    parsedDataInfo = "uint8_t" + parsedDataInfo + format_string("%u", value);
                    break;
                }
                case BuiltinType::_builtin_type::kUInt16Type:
                {
                    uint16_t value;
                    m_codec->read(&value);
                    if ((err = m_codec->getStatus()))
                    {
                        return err;
                    }
                    parsedDataInfo = "uint16_t" + parsedDataInfo + format_string("%u", value);
                    break;
                }
                case BuiltinType::_builtin_type::kUInt32Type:
                {
                    uint32_t value;
                    m_codec->read(&value);
                    if ((err = m_codec->getStatus()))
                    {
                        return err;
                    }
                    parsedDataInfo = "uint32_t" + parsedDataInfo + format_string("%u", value);
                    break;
                }
                case BuiltinType::_builtin_type::kUInt64Type:
                {
                    uint64_t value;
                    m_codec->read(&value);
                    if ((err = m_codec->getStatus()))
                    {
                        return err;
                    }
                    parsedDataInfo = "uint64_t" + parsedDataInfo + format_string("%lu", value);
                    break;
                }
                case BuiltinType::_builtin_type::kFloatType:
                {
                    float value;
                    m_codec->read(&value);
                    if ((err = m_codec->getStatus()))
                    {
                        return err;
                    }
                    parsedDataInfo = "float" + parsedDataInfo + format_string("%f", value);
                    break;
                }
                case BuiltinType::_builtin_type::kDoubleType:
                {
                    double value;
                    m_codec->read(&value);
                    if ((err = m_codec->getStatus()))
                    {
                        return err;
                    }
                    parsedDataInfo = "double" + parsedDataInfo + format_string("%f", value);
                    break;
                }
                case BuiltinType::_builtin_type::kStringType:
                {
                    char *value;
                    uint32_t length;
                    m_codec->readString(&length, &value);
                    if ((err = m_codec->getStatus()))
                    {
                        return err;
                    }
                    parsedDataInfo = "string" + parsedDataInfo + format_string("%.*s", length, value);
                    break;
                }
                case BuiltinType::_builtin_type::kBinaryType:
                {
                    uint8_t *value;
                    uint32_t length;
                    m_codec->readBinary(&length, &value);
                    if ((err = m_codec->getStatus()))
                    {
                        return err;
                    }
                    string binaryValue;
                    for (int i = 0; i < length; ++i)
                    {
                        binaryValue += format_string("%d|", value[i]);
                    }
                    parsedDataInfo = "binary" + parsedDataInfo + binaryValue;
                    break;
                }
                default:
                {
                    throw runtime_error("Unrecognized builtin type.\n");
                }
            }
            break;
        }
        case DataType::_data_type::kEnumType:
        {
            EnumType *e = dynamic_cast<EnumType *>(dataType);
            assert(e);
            int32_t value;
            m_codec->read(&value);
            if ((err = m_codec->getStatus()))
            {
                return err;
            }
            string enumMemberName;
            for (EnumMember *enumMember : e->getMembers())
            {
                if (enumMember->getValue() == value)
                {
                    enumMemberName = enumMember->getName();
                    break;
                }
            }
            if (enumMemberName.empty())
            {
                enumMemberName = format_string("%d", value);
            }
            parsedDataInfo = format_string("%s value: %s", e->getName().c_str(), enumMemberName.c_str());
            break;
        }
        case DataType::_data_type::kFunctionType:
        {
            FunctionType *f = dynamic_cast<FunctionType *>(dataType);
            assert(f);
            int32_t value;
            m_codec->read(&value);
            if ((err = m_codec->getStatus()))
            {
                return err;
            }
            Function *function = f->getCallbackFuns()[value];
            if (function)
            {
                parsedDataInfo = format_string("%s value: %s", f->getName().c_str(), function->getName().c_str());
            }
            else
            {
                parsedDataInfo = format_string("%s unknown value: %d", f->getName().c_str(), value);
            }
            break;
        }
        case DataType::_data_type::kListType:
        {
            ListType *listType = dynamic_cast<ListType *>(dataType);
            assert(listType);
            uint32_t listSize;
            m_codec->startReadList(&listSize);
            if ((err = m_codec->getStatus()))
            {
                return err;
            }
            for (int i = 0; i < listSize; i++)
            {
                string parseDataInfo;
                err = parseDataType(listType->getElementType(), parseDataInfo);
                if (err)
                {
                    return err;
                }

                parseDataInfo =
                    format_string("{%d}:", i).append(countSpaces(listSize) - countSpaces(i + 1), ' ') + parseDataInfo;
                addSpaces(parseDataInfo, countSpaces(listSize) + 3);

                if (i > 0)
                {
                    parsedDataInfo += "\n";
                }
                parsedDataInfo += parseDataInfo;
            }
            break;
        }
        case DataType::_data_type::kStructType:
        {
            StructType *structType = dynamic_cast<StructType *>(dataType);
            assert(structType);
            parsedDataInfo = "struct " + structType->getName() + ":\n";
            StructType::member_vector_t members = structType->getMembers();
            for (int i = 0; i < members.size(); ++i)
            {
                string parseDataInfo;
                err = parseMemberType(structType, members[i], parseDataInfo);
                if (err)
                {
                    return err;
                }

                parseDataInfo = format_string("member '%s':\n", members[i]->getName().c_str()) + parseDataInfo;
                addSpaces(parseDataInfo, 2);
                if (i > 0)
                {
                    parsedDataInfo += "\n";
                }
                parsedDataInfo += parseDataInfo;
            }
            addSpaces(parsedDataInfo, 2);
            break;
        }
        case DataType::_data_type::kUnionType:
        {
            UnionType *unionType = dynamic_cast<UnionType *>(dataType);
            assert(unionType);
            int32_t discriminator;
            m_codec->startReadUnion(&discriminator);
            UnionCase *defaultCase = nullptr;
            UnionCase *matchedCase = nullptr;
            for (UnionCase *unionCase : unionType->getCases())
            {
                if (unionCase->getCaseValue() == discriminator)
                {
                    matchedCase = unionCase;
                    break;
                }
                else if (unionCase->getCaseValue() == -1)
                {
                    defaultCase = unionCase;
                }
            }
            if (!matchedCase && defaultCase)
            {
                matchedCase = defaultCase;
            }
            if (matchedCase)
            {
                parsedDataInfo = "case " + matchedCase->getName() + ":\n";
                for (string name : matchedCase->getMemberDeclarationNames())
                {
                    StructMember *structMember = matchedCase->getUnionMemberDeclaration(name);
                    string parseDataInfo;
                    err = parseMemberType(&unionType->getUnionMembers(), structMember, parseDataInfo);
                    if (err)
                    {
                        return err;
                    }

                    parseDataInfo = format_string("member '%s':\n", structMember->getName().c_str()) + parseDataInfo;
                    addSpaces(parseDataInfo, 2);
                    parsedDataInfo += parseDataInfo + "\n";
                }
                addSpaces(parsedDataInfo, 2);
            }
            else
            {
                parsedDataInfo += format_string("case %d:\n", discriminator);
            }
            parsedDataInfo = "union " + unionType->getName() + ":\n" + parsedDataInfo;
            addSpaces(parsedDataInfo, 2);
            break;
        }
        case DataType::_data_type::kVoidType:
        {
            parsedDataInfo = "void";
            break;
        }
        default:
        {
            throw runtime_error("Unrecognized data type.\n");
        }
    }
    return kErpcStatus_Success;
}

erpc_status_t Sniffer::parseMemberType(StructType *structType, StructMember *structMember, string &parsedMemberInfo)
{
    erpc_status_t err;
    DataType *currentDataType = structMember->getDataType();
    // DataType *currentTrueDataType = currentDataType->getTrueDataType();

    if (structType)
    {
        // check if data type is referenced
        for (StructMember *referenced : structType->getMembers())
        {
            Annotation *ann;
            string referencedName;
            if ((ann = referenced->findAnnotation(LENGTH_ANNOTATION, Annotation::kC)) ||
                (ann = referenced->findAnnotation(DISCRIMINATOR_ANNOTATION, Annotation::kC)))
            {
                if (Value *val = ann->getValueObject())
                {
                    referencedName = val->toString();
                }
            }
            DataType *referencedDataType = referenced->getDataType()->getTrueDataType();
            if (referencedName.empty() && referencedDataType->isUnion())
            {
                UnionType *unionType = dynamic_cast<UnionType *>(referencedDataType);
                assert(unionType);
                referencedName = unionType->getDiscriminatorName();
            }
            if (!referencedName.empty() && structMember->getName().compare(referencedName) == 0)
            {
                parsedMemberInfo = "serialized via other member";
                return kErpcStatus_Success;
            }
        }

        // check if reference is null-able
        Annotation *ann;
        if ((ann = structMember->findAnnotation(LENGTH_ANNOTATION, Annotation::kC)) ||
            (ann = structMember->findAnnotation(DISCRIMINATOR_ANNOTATION, Annotation::kC)))
        {
            if (Value *val = ann->getValueObject())
            {
                StructMember *reference =
                    dynamic_cast<StructMember *>(structType->getScope().getSymbol(val->toString(), false));
                err = readNullFlag(reference, parsedMemberInfo);
                if (err)
                {
                    return err;
                }
                else if (!parsedMemberInfo.empty())
                {
                    return kErpcStatus_Success;
                }
            }
        }
    }

    // check if current member is null-able
    err = readNullFlag(structMember, parsedMemberInfo);
    if (err)
    {
        return err;
    }
    else if (!parsedMemberInfo.empty())
    {
        return kErpcStatus_Success;
    }

    // check if current member is shared
    err = readSharedAddress(structMember, parsedMemberInfo);
    if (err)
    {
        return err;
    }
    else if (!parsedMemberInfo.empty())
    {
        return kErpcStatus_Success;
    }

    return parseDataType(currentDataType, parsedMemberInfo);
}

Interface *Sniffer::getInterface(uint32_t interfaceID)
{
    for (Symbol *interfaceSymbol : m_interfaces)
    {
        Interface *interface = dynamic_cast<Interface *>(interfaceSymbol);
        assert(interface);
        if (interface->getUniqueId() == interfaceID)
        {
            return interface;
        }
    }
    return nullptr;
}

Function *Sniffer::getFunction(uint32_t functionID, Interface *interface)
{
    for (Function *function : interface->getFunctions())
    {
        if (function->getUniqueId() == functionID)
        {
            return function;
        }
    }
    return nullptr;
}

string Sniffer::getDataTypeName(DataType *dataType)
{
    switch (dataType->getDataType())
    {
        case DataType::kListType:
        {
            ListType *listType = dynamic_cast<ListType *>(dataType);
            assert(listType);
            return "list<" + getDataTypeName(listType->getElementType()) + ">";
        }
        case DataType::kArrayType:
        {
            string returnVal;
            while (dataType->isArray())
            {
                ArrayType *arrayType = dynamic_cast<ArrayType *>(dataType);
                assert(arrayType);
                returnVal += format_string("[%d]", arrayType->getElementCount());
                dataType = arrayType->getElementType();
            }
            return getDataTypeName(dataType) + returnVal;
        }
        case DataType::kVoidType:
        {
            return "void";
        }
        default:
            return dataType->getName();
    }
}

string Sniffer::getPrototype(erpcgen::Function *function)
{
    string prototype = getDataTypeName(function->getReturnType());
    if (function->isOneway())
    {
        prototype = "oneway " + prototype;
    }

    prototype += " " + function->getName() + "(";

    StructType::member_vector_t members = function->getParameters().getMembers();
    for (uint32_t i = 0; i < members.size(); ++i)
    {
        prototype += getDataTypeName(members[i]->getDataType()) + " " + members[i]->getName();
        if (i < members.size() - 1)
        {
            prototype += ", ";
        }
    }
    prototype += ");";

    return prototype;
}

erpc_status_t Sniffer::analyzeMessage(string &message, const char *timeDiffernce, const char *currentTime)
{
    // Read header of received message.
    message_type_t messageType;
    uint32_t serviceId;
    uint32_t methodId;
    uint32_t sequence;
    m_codec->startReadMessage(&messageType, &serviceId, &methodId, &sequence);
    erpc_status_t err = m_codec->getStatus();
    if (err)
    {
        return err;
    }

    enum msg_t
    {
        msg_request, // client serialization/server deserialization
        msg_answer   // client deserialization/server serialization
    };

    // Set message type.
    msg_t msgType;
    if (messageType == message_type_t::kInvocationMessage || messageType == message_type_t::kOnewayMessage)
    {
        msgType = msg_request;
        message += "'Request'";
    }
    else
    {
        msgType = msg_answer;
        message += "'Answer'";
    }

    // Record analyzed message header
    message += format_string(" message (id: %d) at %s. Sequence number %d. Time from last message %s ns.\n",
                             (int32_t)messageType, currentTime, sequence, timeDiffernce);

    // Find and record interface information.
    if (Interface *interface = getInterface(serviceId))
    {
        string groupName = interface->getAnnStringValue(GROUP_ANNOTATION, Annotation::kC);
        message += format_string("Group name:%s\nInterface name:%s id:%d\n", groupName.c_str(),
                                 interface->getName().c_str(), serviceId);

        // Find and record function information.
        if (Function *function = getFunction(methodId, interface))
        {
            message += format_string("Function name:%s id:%d prototype: %s\n", function->getName().c_str(), methodId,
                                     getPrototype(function).c_str());

            // Record function's params information.
            StructType params = function->getParameters();
            for (StructMember *param : params.getMembers())
            {
                if ((msgType == msg_request && (param->getDirection() == _param_direction::kInDirection ||
                                                param->getDirection() == _param_direction::kInoutDirection)) ||
                    (msgType == msg_answer && (param->getDirection() == _param_direction::kOutDirection ||
                                               param->getDirection() == _param_direction::kInoutDirection)))
                {
                    string output;
                    err = parseMemberType(&params, param, output);
                    if (err)
                    {
                        return err;
                    }
                    output = format_string("  Param '%s': \n", param->getName().c_str()) + output;
                    addSpaces(output, 4);
                    message += output + "\n";
                }
            }

            // Record function's return data information.
            if (msgType == msg_answer)
            {
                StructMember *returnDataType = function->getReturnStructMemberType();
                if (returnDataType)
                {
                    string output;
                    err = parseMemberType(nullptr, returnDataType, output);
                    if (err)
                    {
                        return err;
                    }
                    output = "  Return type: \n" + output;
                    addSpaces(output, 4);
                    message += output + "\n";
                }
            }
        }
        else
        {
            message += format_string("Unrecognized function's id:%d\n", methodId);
        }
    }
    else
    {
        message += format_string("Unrecognized interface's id:%d\n", serviceId);
    }
    message += "\n";

    return kErpcStatus_Success;
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
