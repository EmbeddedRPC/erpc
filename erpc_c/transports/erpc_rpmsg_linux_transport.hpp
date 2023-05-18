/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Download "rpmsg_linux_endpoint.h/.cpp" from
 * github.com/EmbeddedRPC/erpc-imx-demos/tree/master/middleware/rpmsg-cpp */
#include "erpc_rpmsg_linux_endpoint.hpp"
#include "erpc_transport.hpp"

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpc {

/*!
 * @brief RPMSG Linux transport to send/receive messages through RPMSG endpoints
 * based on github.com/nxp-mcuxpresso/rpmsg-sysfs/tree/0aa1817545a765c200b1b2f9b6680a420dcf9171
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

    virtual ~RPMsgLinuxTransport(void);

    /*!
     * @brief This function returns rpmsg endpoint object.
     *
     * @return RPMsgEndpoint * Rpmsg endpoint.
     */
    RPMsgEndpoint *getRpmsgEndpoint(void){ return m_endPoint; }

    /*!
     * @brief This function initializes Linux environment for sending and receiving messages.
     *
     * @retval kErpcStatus_Success When environment was set successfully.
     * @retval kErpcStatus_Fail When environment wasn't set successfully.
     */
    erpc_status_t init(void);

    /*!
     * @brief This function receives the eRPC messages.
     *
     * @param[in] message Message to receive.
     *
     * @retval kErpcStatus_Success When message was received successfully.
     * @retval kErpcStatus_Fail When message wasn't received successfully.
     */
    virtual erpc_status_t receive(MessageBuffer *message);

    /*!
     * @brief This function sends the eRPC messages.
     *
     * @param[in] message Message to send.
     *
     * @retval kErpcStatus_Success When message was sent successfully.
     * @retval kErpcStatus_Fail When message wasn't sent successfully.
     */
    virtual erpc_status_t send(MessageBuffer *message);

private:
    RPMsgEndpoint *m_endPoint; /*!< Object operating with endpoints. */
    int16_t m_remote;          /*!< Remote address. */
};
} // namespace erpc
