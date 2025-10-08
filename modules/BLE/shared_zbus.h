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

struct ble_module_message {
	uint8_t data[BLE_MODULE_MESSAGE_SIZE];
	uint16_t len;
	uint32_t timestamp;
};

ZBUS_MULTIDOMAIN_CHAN_DEFINE(BLE_CHAN,
			     struct ble_module_message,
			     NULL,
			     NULL,
			     ZBUS_OBSERVERS_EMPTY,
			     ZBUS_MSG_INIT(0),
			     IS_ENABLED(CONFIG_MDM_BLE_RUNNER), /* Runner is master */
			     IS_ENABLED(CONFIG_MDM_BLE));

#endif /* _MULTI_DOMAIN_MODULES_BLE_SHARED_ZBUS_H_ */
