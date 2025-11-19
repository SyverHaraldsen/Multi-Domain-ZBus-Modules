/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);


int main(void)
{
	LOG_INF("Module runner started");
	while (1) {
		k_sleep(K_SECONDS(10));
		LOG_INF("Module runner alive");
	}
	return 0;
}
