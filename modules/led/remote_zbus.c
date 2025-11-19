/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/zbus/zbus.h>
#include <zephyr/zbus/proxy_agent/zbus_proxy_agent.h>

#include "mdm_led.h"

#ifndef MDM_LED_PROXY_NODE
#error "MDM_LED_PROXY_NODE must be defined to use multi-domain zbus channels for LED module"
#endif

/* If CONFIG_APP_LED is defined in ATT application, the LED_CHAN is defined there.
 * Otherwise, define it here for the module to use.
 */
#if !defined(CONFIG_APP_LED)

/* This file is for the non-runner/controller side: the controller has the main channel, and the
 * runner has the shadow channel
 */
ZBUS_CHAN_DEFINE(
	LED_CHAN,
	struct led_msg,
	NULL,
	NULL,
	ZBUS_OBSERVERS_EMPTY,
	ZBUS_MSG_INIT(0)
);
#endif

ZBUS_PROXY_ADD_CHAN(MDM_LED_PROXY_NODE, LED_CHAN);
