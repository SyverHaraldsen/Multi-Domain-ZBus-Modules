/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

#if IS_ENABLED(CONFIG_ZBUS_MULTIDOMAIN)
#include <zephyr/zbus/multidomain/zbus_multidomain.h>

/* Set up UART proxy agent, with no channels attached this will just receive
 * messages from the other domain and forward them to the local zbus.
 */
#define ZBUS_UART_NODE DT_ALIAS(zbus_uart)
ZBUS_PROXY_AGENT_DEFINE(uart_proxy, ZBUS_MULTIDOMAIN_TYPE_UART, ZBUS_UART_NODE);

#if IS_ENABLED(CONFIG_MDM_BLE_NUS_RUNNER)
ZBUS_CHAN_DECLARE(BLE_NUS_CHAN);
ZBUS_PROXY_ADD_CHANNEL(uart_proxy, BLE_NUS_CHAN);
#endif

#if !IS_ENABLED(CONFIG_MDM_LED_RUNNER)
ZBUS_CHAN_DECLARE(LED_CHAN);
ZBUS_PROXY_ADD_CHANNEL(uart_proxy, LED_CHAN);
#endif
#endif

int main(void)
{
	LOG_INF("Module runner started");
	while (1) {
		k_sleep(K_SECONDS(10));
		LOG_INF("Module runner alive");
	}
	return 0;
}
