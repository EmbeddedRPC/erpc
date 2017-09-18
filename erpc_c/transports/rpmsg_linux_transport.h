/*
 * Copyright 2017 NXP
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

/* Download "rpmsg_linux_endpoint.h/.cpp" from https://github.com/EmbeddedRPC/erpc-imx-demos/tree/master/middleware/rpmsg-cpp */
#include "rpmsg_linux_endpoint.h"
#include "transport.h"

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpc {

/*!
 * @brief RPMSG linux transport to send/receive messages through RPMSG endpoints
 * based on https://github.com/NXPmicro/rpmsg-sysfs/tree/0aa1817545a765c200b1b2f9b6680a420dcf9171
 * implementation.
 * @ingroup rpmsg_linux_transport
 */
class RPMsgLinuxTransport : public Transport
{
public:
    /*!
     * @brief Constructor to set RPMsg endpoint and remote address.
     */
    RPMsgLinuxTransport(RPMsgEndpoint *endPoint, int16_t remote);

    virtual ~RPMsgLinuxTransport();

    /*!
     * @brief This function initializes Linux enviroment for sending and receving messages.
     *
     * @retval kErpcStatus_Success When enviroment was set successfuly.
     * @retval kErpcStatus_Fail When enviroment wasn't set successfuly.
     */
    erpc_status_t init();

    /*!
     * @brief This function receives the eRPC messages.
     *
     * @param[in] message Message to receive.
     *
     * @retval kErpcStatus_Success When message was received successfuly.
     * @retval kErpcStatus_Fail When message wasn't received successfuly.
     */
    virtual erpc_status_t receive(MessageBuffer *message);

    /*!
     * @brief This function sends the eRPC messages.
     *
     * @param[in] message Message to send.
     *
     * @retval kErpcStatus_Success When message was sent successfuly.
     * @retval kErpcStatus_Fail When message wasn't sent successfuly.
     */
    virtual erpc_status_t send(MessageBuffer *message);

private:
    RPMsgEndpoint *m_endPoint; /*!< Object operating with endpoints. */
    int16_t m_remote;          /*!< Remote address. */
};
} // namespace erpc
