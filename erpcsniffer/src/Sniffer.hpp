/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__SNIFFER_H_
#define _EMBEDDED_RPC__SNIFFER_H_

#include "erpc_c/infra/erpc_basic_codec.hpp"
#include "erpc_c/infra/erpc_transport.hpp"

#include "CGenerator.hpp"

#include <chrono>
////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpcsniffer {
/*!
 * @brief Code eRPC messages sniffer tool.
 */
class Sniffer
{
public:
    /*!
     * @brief This function is constructor of Sniffer class.
     *
     * Set application specific attributes.
     *
     * @param[in] transport Transport used for catching eRPC messages.
     * @param[in] def Contains information parsed from IDL.
     * @param[in] outputFilePath Path to output file used for record output.
     * @param[in] quantity How much messages record.
     */
    Sniffer(erpc::Transport *transport, erpcgen::InterfaceDefinition *def, const char *outputFilePath,
            uint64_t quantity)
    : m_transport(transport)
    , m_interfaces(def->getGlobals().getSymbolsOfType(erpcgen::Symbol::symbol_type_t::kInterfaceSymbol))
    , m_outputFilePath(outputFilePath)
    , m_quantity(quantity)
    , m_codec(new erpc::BasicCodec()){};

    /*!
     * @brief This function is destructor of Sniffer class.
     */
    virtual ~Sniffer(){};

    /*!
     * @brief This function run sniffer tool.
     *
     * Application will call receive function through set transport in loop.
     * Each message is analyzed and stored in file of output file destination is set.
     */
    virtual erpc_status_t run();

protected:
    erpc::Transport *m_transport;                       /*!< Initiated transport. */
    erpcgen::SymbolScope::symbol_vector_t m_interfaces; /*!< Vector of interfaces parsed from IDL. */
    const char *m_outputFilePath;                       /*!< Path to the file for recording output. */
    uint64_t m_quantity;                                /*!< Number of messages to catch. */
    erpc::Codec *m_codec;                               /*!< Codec type used in eRPC applications */

    /*!
     * @brief This function will create and open file for recording outputs.
     *
     * @param[in,out] outputFileStream Output stream used for recording outputs.
     */
    void openFile(std::ofstream &outputFileStream);

    /*!
     * @brief This function reads null flag when structMember need.
     *
     * It reads null flag when @nullable annotation is used within structMember.
     *
     * @param[in] structMember Output stream used for recording outputs.
     * @param[in,out] nullFlag String containing "NULL" when received null flag was true.
     *
     * @retval kErpcStatus_Success when reading passed.
     */
    erpc_status_t readNullFlag(erpcgen::StructMember *structMember, std::string &nullFlag);

    /*!
     * @brief This function reads shared memory address when structMember need.
     *
     * It reads shared memory address when @shared annotation is used within structMember.
     *
     * @param[in] structMember Output stream used for recording outputs.
     * @param[in,out] address String containing received address.
     *
     * @retval kErpcStatus_Success when reading passed.
     */
    erpc_status_t readSharedAddress(erpcgen::StructMember *structMember, std::string &address);

    /*!
     * @brief This function counts spaces for special indent.
     *
     * This function counts spaces for indent used for arrays and lists outputs.
     *
     * @param[in] value Number used for indent.
     *
     * @return Indent.
     */
    uint8_t countSpaces(uint64_t value);

    /*!
     * @brief This function adds spaces for indent each line.
     *
     * @param[in,out] text Original string used for adding indent.
     * @param[in] spacesCount Number of spaces used for indent text.
     */
    void addSpaces(std::string &text, uint spacesCount);

    /*!
     * @brief This function parse part of received message based on expected data type.
     *
     * @param[in] dataType Expected data type.
     * @param[in,out] parsedDataInfo Output of parsed data.
     *
     * @retval kErpcStatus_Success when parsing passed.
     */
    erpc_status_t parseDataType(erpcgen::DataType *dataType, std::string &parsedDataInfo);

    /*!
     * @brief This function parse part of received message based on expected struct member.
     *
     * @param[in] structType Contains all members. Needed for handling annotations.
     * @param[in] structMember Expected struct member.
     * @param[in,out] parsedMemberInfo Output of parsed data.
     *
     * @retval kErpcStatus_Success when parsing passed.
     */
    erpc_status_t parseMemberType(erpcgen::StructType *structType, erpcgen::StructMember *structMember,
                                  std::string &parsedMemberInfo);

    /*!
     * @brief This function returns interface object based on given id.
     *
     * @param[in] interfaceID Interface id number.
     *
     * @retval Found interface object.
     */
    erpcgen::Interface *getInterface(uint32_t interfaceID);

    /*!
     * @brief This function returns function object based on given id.
     *
     * @param[in] functionID Interface id number.
     * @param[in] interface Interface with list of functions.
     *
     * @retval Found function object.
     */
    erpcgen::Function *getFunction(uint32_t functionID, erpcgen::Interface *interface);

    /*!
     * @brief This function returns data type name.
     *
     * @param[in] dataType Data type which is transformed to human readable data type name.
     *
     * @retval Data type name.
     */
    std::string getDataTypeName(erpcgen::DataType *dataType);

    /*!
     * @brief This function returns function prototype.
     *
     * @param[in] function Function which is transformed to human readable function prototype.
     *
     * @retval Function prototype.
     */
    std::string getPrototype(erpcgen::Function *function);

    /*!
     * @brief This function parse received message based.
     *
     * @param[in,out] message Output based on parsed received message.
     * @param[in] timeDiffernce Time difference between current received message time and previous received message
     * time.
     * @param[in] currentTime Formatted current time.
     *
     * @retval kErpcStatus_Success when parsing passed.
     */
    erpc_status_t analyzeMessage(std::string &message, const char *timeDiffernce, const char *currentTime);
};
} // namespace erpcsniffer

#endif // _EMBEDDED_RPC__SNIFFER_H_
