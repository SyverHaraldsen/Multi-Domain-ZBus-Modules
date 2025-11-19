/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/zbus/zbus.h>
#include <zephyr/zbus/proxy_agent/zbus_proxy_agent.h>

#include "ble_nus.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(mdm_ble_nus_module, CONFIG_APP_LOG_LEVEL);

#ifndef MDM_BLE_NUS_PROXY_NODE
#error "MDM_BLE_NUS_PROXY_NODE must be defined to use multi-domain zbus channels for BLE NUS module"
#endif


/* This file is for the non-runner/controller side: the controller has the shadow channel, and the
 * runner has the main channel
 */
ZBUS_SHADOW_CHAN_DEFINE(
	BLE_NUS_CHAN,
	struct ble_nus_module_message,
	MDM_BLE_NUS_PROXY_NODE,
	NULL,
	ZBUS_OBSERVERS_EMPTY,
	ZBUS_MSG_INIT(0)
);

#if IS_ENABLED(CONFIG_MDM_BLE_NUS_ZBUS_LOGGING)

static void log_ble_nus_message(const struct zbus_channel *chan)
{
	const struct ble_nus_module_message *msg = zbus_chan_const_msg(chan);
	LOG_INF("=== BLE NUS ZBUS Message Received ===");
	LOG_INF("Type: %s", ble_message_type_to_string(msg->type));
	LOG_INF("Timestamp: %u ms", msg->timestamp);
	LOG_INF("Length: %u bytes", msg->len);

	/* Try to print as string if printable */
	bool printable = true;

	for (uint16_t i = 0; i < msg->len; i++) {
		if (msg->data[i] < 0x20 && msg->data[i] != '\r' &&
		    msg->data[i] != '\n' && msg->data[i] != '\t') {
			printable = false;
			break;
		}
	}

	if (printable && msg->len < BLE_MAX_PRINT_LEN) {
		char str_buf[BLE_MAX_PRINT_LEN];

		memcpy(str_buf, msg->data, msg->len);
		str_buf[msg->len] = '\0';
		LOG_INF("As String: \"%s\"", str_buf);
	}
	LOG_INF("=============================");
}

ZBUS_LISTENER_DEFINE(ble_nus_logger, log_ble_nus_message);
ZBUS_CHAN_ADD_OBS(BLE_NUS_CHAN, ble_nus_logger, 0);

#endif /* CONFIG_MDM_BLE_NUS_ZBUS_LOGGING */
