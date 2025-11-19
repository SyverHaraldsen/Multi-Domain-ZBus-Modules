/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef BLE_NUS_MODULE_H_
#define BLE_NUS_MODULE_H_

#include <zephyr/zbus/zbus.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BLE_NUS_MODULE_MESSAGE_SIZE 100
#define BLE_MAX_PRINT_LEN           256

/* Channels provided by this module */
ZBUS_CHAN_DECLARE(BLE_NUS_CHAN);

enum ble_msg_type {
	BLE_RECV,
};
struct ble_nus_module_message {
	enum ble_msg_type type;
	uint8_t data[BLE_NUS_MODULE_MESSAGE_SIZE];
	uint16_t len;
	uint32_t timestamp;
};

static inline const char *ble_message_type_to_string(enum ble_msg_type type)
{
	switch (type) {
	case BLE_RECV:
		return "BLE_RECV";
	default:
		return "UNKNOWN";
	}
}

#ifdef __cplusplus
}
#endif

#endif /* BLE_NUS_MODULE_H_ */
