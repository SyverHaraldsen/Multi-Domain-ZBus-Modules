/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/zbus/multidomain/zbus_multidomain.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

/* Set up UART1 proxy agent, with no channels attached this will just receive
 * messages from the other domain and forward them to the local zbus.
 */
#define ZBUS_UART_NODE DT_ALIAS(zbus_uart)
ZBUS_PROXY_AGENT_DEFINE(uart1_proxy, ZBUS_MULTIDOMAIN_TYPE_UART, ZBUS_UART_NODE);

int main(void)
{
        LOG_INF("Module runner started");
        while (1) {
                k_sleep(K_SECONDS(1));
                LOG_INF("Module runner alive");
        }
        return 0;
}
