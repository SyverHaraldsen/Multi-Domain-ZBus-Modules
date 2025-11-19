/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/zbus/zbus.h>
#include <zephyr/zbus/proxy_agent/zbus_proxy_agent.h>

#include "ble_nus.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ble_nus_module, CONFIG_APP_LOG_LEVEL);

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

static const char *ble_message_type_to_string(enum ble_msg_type type)
{
	switch (type) {
	case BLE_RECV:
		return "BLE_RECV";
	default:
		return "UNKNOWN";
	}
}

/* Add a message subscriber to the BLE_NUS_CHAN channel to log received messages */
ZBUS_MSG_SUBSCRIBER_DEFINE(test_msg_subscriber);
ZBUS_CHAN_ADD_OBS(BLE_NUS_CHAN, test_msg_subscriber, 0);

static void test_subscriber_thread(void *unused1, void *unused2, void *unused3)
{
	ARG_UNUSED(unused1);
	ARG_UNUSED(unused2);
	ARG_UNUSED(unused3);

	const struct zbus_channel *chan;
	struct ble_nus_module_message msg;

	while (true) {
		if (zbus_sub_wait_msg(&test_msg_subscriber, &chan, &msg, K_FOREVER) == 0) {
			if (chan == &BLE_NUS_CHAN) {
				LOG_INF("=== ZBUS Message Received ===");
				LOG_INF("Type: %s", ble_message_type_to_string(msg.type));
				LOG_INF("Timestamp: %u ms", msg.timestamp);
				LOG_INF("Length: %u bytes", msg.len);

				/* Try to print as string if printable */
				bool printable = true;

				for (uint16_t i = 0; i < msg.len; i++) {
					if (msg.data[i] < 0x20 && msg.data[i] != '\r' &&
					    msg.data[i] != '\n' && msg.data[i] != '\t') {
						printable = false;
						break;
					}
				}

				if (printable && msg.len < BLE_MAX_PRINT_LEN) {
					char str_buf[BLE_MAX_PRINT_LEN];

					memcpy(str_buf, msg.data, msg.len);
					str_buf[msg.len] = '\0';
					LOG_INF("As String: \"%s\"", str_buf);
				}
				LOG_INF("=============================");
			}
		}
	}
}

K_THREAD_DEFINE(test_subscriber_tid, CONFIG_MDM_BLE_NUS_ZBUS_LOGGING_STACK_SIZE,
		test_subscriber_thread, NULL, NULL, NULL, CONFIG_MDM_BLE_NUS_ZBUS_LOGGING_PRIORITY,
		0, 0);

#endif /* CONFIG_MDM_BLE_NUS_ZBUS_LOGGING */
