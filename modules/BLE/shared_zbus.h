/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * This header defines the zbus channels used for inter chip communication
 *
 * To use this module, users of this module should set CONFIG_MDM_BLE=y
 * in the prj.conf file of the application.
 * The runner application should set CONFIG_MDM_BLE=y and CONFIG_MDM_BLE_RUNNER=y
 * in the prj.conf file of the application.
 */

#ifndef _MULTI_DOMAIN_MODULES_BLE_SHARED_ZBUS_H_
#define _MULTI_DOMAIN_MODULES_BLE_SHARED_ZBUS_H_

#include <zephyr/zbus/zbus.h>

#define BLE_MODULE_MESSAGE_SIZE 128

enum ble_msg_type {
	BLE_RECV,
};
struct ble_module_message {
	enum ble_msg_type type;
	uint8_t data[BLE_MODULE_MESSAGE_SIZE];
	uint16_t len;
	uint32_t timestamp;
};

ZBUS_CHAN_DECLARE(BLE_CHAN);

#endif /* _MULTI_DOMAIN_MODULES_BLE_SHARED_ZBUS_H_ */
