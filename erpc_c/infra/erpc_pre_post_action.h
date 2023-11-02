/*
 * Copyright 2020 NXP
 * Copyright 2020 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__PREPOSTACTION_H_
#define _EMBEDDED_RPC__PREPOSTACTION_H_

#ifdef __cplusplus
#include <cstddef>
/*!
 * @addtogroup infra_transport
 * @{
 * @file
 */
extern "C" {
#endif

typedef void (*pre_post_action_cb)(void);

#ifdef __cplusplus
}

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpc {

/*!
 * @brief Client and server may used cb functions before and after rpc call.
 *
 * @ingroup infra_utility
 */
class PrePostAction
{
public:
    /*!
     * @brief PrePostAction constructor.
     */
    PrePostAction(void) : m_preCB(NULL), m_postCB(NULL){};

    /*!
     * @brief This function sets "before eRPC call start" callback function.
     *
     * @param[in] preCB Pointer for callback function. When NULL and ERPC_PRE_POST_ACTION_DEFAULT
     * is enabled then default function will be set.
     */
    void addPreCB(pre_post_action_cb preCB);

    /*!
     * @brief This function sets "after eRPC call finish" callback function.
     *
     * @param[in] postCB Pointer for callback function. When NULL and ERPC_PRE_POST_ACTION_DEFAULT
     * is enabled then default function will be set.
     */
    void addPostCB(pre_post_action_cb postCB);

    /*!
     * @brief This function returns "before eRPC call start" callback function.
     *
     * @return preCB Pointer for callback function.
     */
    pre_post_action_cb getPreCB(void) { return m_preCB; }

    /*!
     * @brief This function returns "after eRPC call finish" callback function.
     *
     * @return postCB Pointer for callback function.
     */
    pre_post_action_cb getPostCB(void) { return m_postCB; }

    /*!
     * @brief PrePostAction destructor
     */
    ~PrePostAction(void){};

protected:
    pre_post_action_cb m_preCB;  /*!< Pointer to "before eRPC call start" callback function. */
    pre_post_action_cb m_postCB; /*!< Pointer to after eRPC call finish" callback function. */
};

} // namespace erpc

/*! @} */

#endif

#endif // _EMBEDDED_RPC__PREPOSTACTION_H_
