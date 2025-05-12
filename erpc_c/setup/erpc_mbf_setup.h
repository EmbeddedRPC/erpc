/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _ERPC_MBF_SETUP_H_
#define _ERPC_MBF_SETUP_H_

#include "erpc_transport_setup.h"

/*!
 * @addtogroup message_buffer_factory_setup
 * @{
 * @file
 */

////////////////////////////////////////////////////////////////////////////////
// Types
////////////////////////////////////////////////////////////////////////////////

//! @brief Opaque MessageBufferFactory object type.
typedef struct ErpcMessageBufferFactory *erpc_mbf_t;

////////////////////////////////////////////////////////////////////////////////
// API
////////////////////////////////////////////////////////////////////////////////

#if __cplusplus > 201703L
#include "erpc_setup_mbf_static_fixed.h"

/*!
 * @brief Create MessageBuffer factory which is using static fixed allocated buffers.
 */
template <size_t BUFFER_COUNT, size_t WORD_COUNT, class WORD_SIZE=uint8_t>
erpc_mbf_t erpc_mbf_static_fixed_init(void){
    static ManuallyConstructed<StaticFixedMessageBufferFactory<BUFFER_COUNT, WORD_COUNT, WORD_SIZE>> s_msgFactory;
    s_msgFactory.construct();
    return reinterpret_cast<erpc_mbf_t>(s_msgFactory.get());
    return nullptr;
}
#endif

#ifdef __cplusplus

extern "C" {
#endif

//! @name MessageBufferFactory setup
//@{

/*!
 * @brief Create MessageBuffer factory which is using dynamic allocated buffers.
 */
erpc_mbf_t erpc_mbf_dynamic_init(void);

/*!
 * @brief Deinit MessageBuffer factory.
 *
 * @param[in] mbf MessageBuffer factory which was initialized in init function.
 */
void erpc_mbf_dynamic_deinit(erpc_mbf_t mbf);

/*!
 * @brief Create MessageBuffer factory which is using RPMSG LITE zero copy buffers.
 *
 * Has to be used with RPMSG lite zero copy transport.
 */
erpc_mbf_t erpc_mbf_rpmsg_init(erpc_transport_t transport);

/*!
 * @brief Deinit MessageBuffer factory.
 *
 * @param[in] mbf MessageBuffer factory which was initialized in init function.
 */
void erpc_mbf_rpmsg_deinit(erpc_mbf_t mbf);

/*!
 * @brief Create MessageBuffer factory which is using static allocated buffers.
 */
erpc_mbf_t erpc_mbf_static_init(void);

/*!
 * @brief Deinit MessageBuffer factory.
 *
 * @param[in] mbf MessageBuffer factory which was initialized in init function.
 */
void erpc_mbf_static_deinit(erpc_mbf_t mbf);

//@}

#ifdef __cplusplus
}
#endif

/*! @} */

#endif // _ERPC_MBF_SETUP_H_
