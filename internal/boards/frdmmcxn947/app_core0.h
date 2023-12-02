/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _APP_CORE0_H_
#define _APP_CORE0_H_

/*${header:start}*/
#include "fsl_gpio.h"
/*${header:end}*/

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*${macro:start}*/
#define ERPC_TRANSPORT_RPMSG_LITE_LINK_ID (RL_PLATFORM_MCXNX4X_M33_M33_LINK_ID)

#define BUTTON_INIT()                   \
    gpio_pin_config_t button_config = { \
        kGPIO_DigitalInput,             \
        0,                              \
    };                                  \
    GPIO_PinInit(BOARD_SW3_GPIO, BOARD_SW3_GPIO_PIN, &button_config)
#define IS_BUTTON_PRESSED() (0U == GPIO_PinRead(BOARD_SW3_GPIO, BOARD_SW3_GPIO_PIN))
#define BUTTON_NAME         BOARD_SW3_NAME

/* Address of RAM, where the image for core1 should be copied */
#define CORE1_BOOT_ADDRESS 0x2004E000

#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
extern uint32_t Image$$CORE1_REGION$$Base;
extern uint32_t Image$$CORE1_REGION$$Length;
#define CORE1_IMAGE_START &Image$$CORE1_REGION$$Base
#elif defined(__ICCARM__)
extern unsigned char core1_image_start[];
#define CORE1_IMAGE_START core1_image_start
#elif (defined(__GNUC__)) && (!defined(__MCUXPRESSO))
extern const char core1_image_start[];
extern const char *core1_image_end;
extern uint32_t core1_image_size;
#define CORE1_IMAGE_START ((void *)core1_image_start)
#define CORE1_IMAGE_SIZE  ((void *)core1_image_size)
#endif

extern char rpmsg_lite_base[];

/*${macro:end}*/
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*${prototype:start}*/
void BOARD_InitHardware(void);

#ifdef CORE1_IMAGE_COPY_TO_RAM
uint32_t get_core1_image_size(void);
#endif
/*${prototype:end}*/

#endif /* _APP_CORE0_H_ */
