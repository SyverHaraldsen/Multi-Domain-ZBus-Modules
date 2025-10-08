/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/**@file
 *
 * @brief   LED module.
 *
 * Module that handles LED behaviour.
 */

#ifndef LED_H__
#define LED_H__

#include <zephyr/kernel.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MSG_TO_LED_MSG(_msg) ((const struct led_msg *)_msg)

#ifdef __cplusplus
}
#endif

#endif /* LED_H__ */
