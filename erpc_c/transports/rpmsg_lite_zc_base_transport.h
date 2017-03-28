/*
 * Copyright (c) 2015-2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
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

#ifndef _EMBEDDED_RPC__RPMSG_LITE_TRANSPORT_H_
#define _EMBEDDED_RPC__RPMSG_LITE_TRANSPORT_H_

#include "rpmsg_lite.h"
#include "transport.h"

/*!
 * @addtogroup rpmsg_lite_zc_transport
 * @addtogroup rpmsg_lite_zc_rtos_transport
 * @{
 * @file
 */

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpc {
/*!
 * @brief Transport that other RPMsg transports inherts.
 *
 * @ingroup rpmsg_lite_transport
 * @ingroup rpmsg_lite_rtos_transport
 */
class RPMsgZCBaseTransport : public Transport
{
public:
    RPMsgZCBaseTransport()
    : Transport(){};

    virtual ~RPMsgZCBaseTransport() {}

    /*!
     * @brief This function returns pointer to instance of RPMSG lite
     *
     * @retval pointer to instance of RPMSG lite
     */
    struct rpmsg_lite_instance *get_rpmsg_lite_instance()
    {
        return s_rpmsg;
    }

protected:
    static struct rpmsg_lite_instance *s_rpmsg; /*!< Pointer to instance of RPMSG lite. */
    static uint8_t s_initialized;               /*!< Represent information if the rpmsg-lite was initialized. */
};

} // namespace erpc

/*! @} */

#endif // _EMBEDDED_RPC__RPMSG_LITE_TRANSPORT_H_
