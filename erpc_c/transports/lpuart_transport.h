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

#ifndef _EMBEDDED_RPC__LPUART_TRANSPORT_H_
#define _EMBEDDED_RPC__LPUART_TRANSPORT_H_

#include <stdlib.h>
#include "framed_transport.h"
#include "fsl_lpuart.h"

/*!
 * @addtogroup lpuart_transport
 * @{
 * @file
 */

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpc
{
/*!
 * @brief Very basic transport to send/receive messages via LPUART
 *
 * @ingroup lpuart_transport
 */
class LpuartTransport : public FramedTransport
{
public:
    // LpuartTransport(const char * name);
    LpuartTransport(LPUART_Type *lpuartInstance, uint32_t baudRate, uint32_t srcClock_Hz);
    virtual ~LpuartTransport();

    virtual status_t init();

protected:
    LPUART_Type *m_lpuartInstance; /*!< */
    uint32_t m_baudRate;           /*!< */
    uint32_t m_srcClock_Hz;        /*!< */
                                   /***************************************************************************
                                   * LPUART configuration and state structures
                                   **************************************************************************/
    /*! @param uartConfig LPUART configuration structure */
    lpuart_config_t m_lpuartConfig; /*!< */

private:
    virtual status_t underlyingReceive(uint8_t *data, uint32_t size);
    virtual status_t underlyingSend(const uint8_t *data, uint32_t size);
};

} // namespace erpc

/*! @} */

#endif // _EMBEDDED_RPC__LPUART_TRANSPORT_H_
