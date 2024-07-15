/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 * Copyright 2021 ACRIOS Systems s.r.o.
 * Copyright 2021 DroidDrive GmbH
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _ERPC_SETUP_MBF_STATIC_FIXED_H_
#define _ERPC_SETUP_MBF_STATIC_FIXED_H_

#include "erpc_config_internal.h"
#include "erpc_manually_constructed.h"
#include "erpc_mbf_setup.h"
#include "erpc_message_buffer.h"

#include <assert.h>
#include <string.h>
#include <array>

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

/*!
 * @brief Static Message buffer factory
 */
template <size_t BUFFER_COUNT, size_t WORD_COUNT, class WORD_SIZE=uint8_t>
class StaticFixedMessageBufferFactory : public MessageBufferFactory
{
private:
    static constexpr std::array<bool, BUFFER_COUNT> init_free_buffer_bitmap() {
        std::array<bool, BUFFER_COUNT> a{};
        for(size_t i = 0; i < BUFFER_COUNT; i++) {
            a[i] = true;
        }
        return a;
    };
    template<typename T>
    static constexpr void init_buffer(T* b) {
        for(size_t j = 0; j < WORD_COUNT; j++){
                b[j] = T{0};
        }
        return;
    };
    static constexpr std::array<std::array<WORD_SIZE, WORD_COUNT>, BUFFER_COUNT> init_buffers() {
        std::array<std::array<WORD_SIZE, WORD_COUNT>, BUFFER_COUNT> bufs = {{}};
        for(size_t i = 0; i < BUFFER_COUNT; i++) {
            init_buffer(bufs[i].data());
        }
        return bufs;
    };
public:
    /*!
     * @brief Constructor.
     */
    StaticFixedMessageBufferFactory(void) {}
    /*!
     * @brief This function creates new message buffer.
     *
     * @return MessageBuffer New created MessageBuffer.
     */
    virtual MessageBuffer create(void)
    {
        WORD_SIZE* bufPtr = nullptr;
        /// check free buffers and return first free one
        for(size_t i = 0; i < BUFFER_COUNT; i++){
            bool free = free_buffer_bitmap_[i];
            if(free)
            {
                free_buffer_bitmap_[i] = false;
                bufPtr = static_cast<WORD_SIZE*>(buffers_[i].data());
                break;
            }
        }
        // if free buffer found, return that
        assert(nullptr != bufPtr);
        uint8_t* buf = static_cast<uint8_t*>(bufPtr);
        return MessageBuffer(buf, WORD_COUNT);
    }

    /*!
     * @brief This function disposes message buffer.
     *
     * @param[in] buf MessageBuffer to dispose.
     */
    virtual void dispose(MessageBuffer *buf)
    {
        assert(buf);
        uint8_t* bufPtr = buf->get();
        if (bufPtr != nullptr)
        {
            // check buf ptr against all current buffers to see which one it was
            for(size_t i = 0; i < BUFFER_COUNT; i++)
            {
                uint8_t* internalBufPtr = static_cast<uint8_t*>(buffers_[i].data());
                if(bufPtr == internalBufPtr){
                    free_buffer_bitmap_[i] = true;
                    init_buffer(bufPtr);
                }
            }
        }
    }

protected:
    static inline auto free_buffer_bitmap_ = init_free_buffer_bitmap();
    static inline auto buffers_ = init_buffers();
};

#endif // _ERPC_SETUP_MBF_STATIC_FIXED_H_