/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/zbus/zbus.h>
#include <zephyr/zbus/proxy_agent/zbus_proxy_agent.h>

#include "mdm_led.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(mdm_led_module, CONFIG_APP_LOG_LEVEL);

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

#if IS_ENABLED(CONFIG_MDM_LED_ZBUS_LOGGING)

static void log_led_message(const struct zbus_channel *chan)
{
	const struct led_msg *msg = zbus_chan_const_msg(chan);
	LOG_INF("=== LED ZBUS Message Received ===");
	LOG_INF("Type: %s", led_message_type_to_string(msg->type));
	LOG_INF("R: %d, G: %d, B: %d", msg->red, msg->green, msg->blue);
	LOG_INF("On Duration: %u ms", msg->duration_on_msec);
	LOG_INF("Off Duration: %u ms", msg->duration_off_msec);
	LOG_INF("Repetitions: %d", msg->repetitions);
	LOG_INF("=============================");
}

ZBUS_LISTENER_DEFINE(led_logger, log_led_message);
ZBUS_CHAN_ADD_OBS(LED_CHAN, led_logger, 0);

#endif /* CONFIG_MDM_LED_ZBUS_LOGGING */
