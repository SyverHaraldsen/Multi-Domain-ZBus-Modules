/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef BLE_MODULE_H_
#define BLE_MODULE_H_

#include <zephyr/types.h>
#include <zephyr/bluetooth/conn.h>
#include <stdint.h>
#include <stdbool.h>

#include <zephyr/zbus/zbus.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Send data over BLE NUS service
 *
 * @param data Pointer to data to send
 * @param len Length of data
 * @return 0 on success, negative errno on failure
 */
int ble_module_send(const uint8_t *data, uint16_t len);

/**
 * @brief Check if BLE is connected
 *
 * @return true if connected, false otherwise
 */
bool ble_module_is_connected(void);

/**
 * @brief Get connection object
 *
 * @return Pointer to connection object or NULL if not connected
 */
struct bt_conn *ble_module_get_connection(void);

/**
 * @brief Check if ready to send (connected and notifications enabled)
 *
 * @return true if ready to send data, false otherwise
 */
bool ble_module_is_ready(void);

#ifdef __cplusplus
}
#endif

#endif /* BLE_MODULE_H_ */
