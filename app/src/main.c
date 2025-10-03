/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#ifdef CONFIG_DK_LIBRARY
#include <dk_buttons_and_leds.h>
#endif

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

/**
 * @brief Main application entry point
 */
int main(void)
{
	int err;

	LOG_INF("Application started");

#ifdef CONFIG_DK_LIBRARY
	/* Initialize LEDs for status indication */
	err = dk_leds_init();
	if (err) {
		LOG_ERR("LED init failed: %d", err);
	}
#endif

	/* BLE module auto-initializes via SYS_INIT if CONFIG_BLE_MODULE=y */

	/* Main loop - blink status LED */
	uint32_t blink_count = 0;

	while (1) {
#ifdef CONFIG_DK_LIBRARY
		/* LED1: Heartbeat (always blinking = app running) */
		dk_set_led(DK_LED1, (blink_count % 2));
#endif
		blink_count++;
		k_sleep(K_SECONDS(1));
	}

	return 0;
}
