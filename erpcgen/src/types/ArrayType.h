/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
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

#ifndef _EMBEDDED_RPC__ARRAYTYPE_H_
#define _EMBEDDED_RPC__ARRAYTYPE_H_

#include <string>
#include "DataType.h"

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpcgen
{
/*!
 * @brief Fixed length array of any other data type.
 */
class ArrayType : public DataType
{
public:
    /*!
     * @brief Constructor.
     *
     * This function set element type to given data type and element count to given count.
     *
     * @param[in] elementType Given data type.
     * @param[in] elementCount Given count.
     */
    ArrayType(DataType *elementType, uint32_t elementCount)
    : DataType("(array)", kArrayType)
    , m_elementType(elementType)
    , m_elementCount(elementCount)
    {
    }

    /*!
     * @brief This function returns element data type.
     *
     * @return Element data type.
     */
    DataType *getElementType() const { return m_elementType; }
    /*!
     * @brief This function set element data type.
     *
     * @param[in] elementType Element data type.
     */
    void setElementType(DataType *elementType) { m_elementType = elementType; }
    /*!
     * @brief This function return "true" value for identify array type.
     *
     * @retval true Always return true.
     */
    virtual bool isArray() const { return true; }
    /*!
     * @brief This function returns description about the array.
     *
     * @return String description about array.
     *
     * @see std::string AliasType::getDescription() const
     * @see std::string EnumType::getDescription() const
     * @see std::string EnumMember::getDescription() const
     * @see std::string StructMember::getDescription() const
     * @see std::string StructType::getDescription() const
     * @see std::string VoidType::getDescription() const
     * @see std::string ListType::getDescription() const
     * @see std::string UnionType::getDescription() const
     */
    virtual std::string getDescription() const;

    /*!
     * @brief This function returns size of array.
     *
     * @return Size of array.
     */
    uint32_t &getElementCount() { return m_elementCount; }
protected:
    DataType *m_elementType; /*!< Array element data type. */
    uint32_t m_elementCount; /*!< Array size. */
};

} // namespace erpcgen

#endif // _EMBEDDED_RPC__ARRAYTYPE_H_
