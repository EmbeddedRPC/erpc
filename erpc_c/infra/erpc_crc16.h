/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__CRC16_H_
#define _EMBEDDED_RPC__CRC16_H_

#include <stdint.h>

/*!
 * @addtogroup infra_transport
 * @{
 * @file
 */

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpc {

/*!
 * @brief Class for compute crc16.
 */
class Crc16
{
public:
    /*!
     * @brief Constructor.
     *
     * @param[in] crcStart Start value for crc function.
     */
    Crc16(uint32_t crcStart);

    Crc16(void);

    /*!
     * @brief Codec destructor
     */
    ~Crc16(void);

    /*!
     * @brief Compute a ITU-CCITT CRC-16 over the provided data.
     *
     * This implementation is slow but small in size.
     *
     * @param[in] data Pointer to data used for crc16.
     * @param[in] lengthInBytes Data length.
     */
    uint16_t computeCRC16(const uint8_t *data, uint32_t lengthInBytes);

    /*!
     * @brief Set crc start number.
     *
     * @param[in] crcStart Crc start number.
     */
    void setCrcStart(uint32_t crcStart);

protected:
    uint32_t m_crcStart; /*!< CRC start number. */
};

} // namespace erpc

/*! @} */

#endif // _EMBEDDED_RPC__CRC16_H_
