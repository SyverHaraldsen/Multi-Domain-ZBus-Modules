/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * This header defines the zbus channels used for inter chip communication
 *
 * To use this module, users of this module should set CONFIG_MDM_LED=y
 * in the prj.conf file of the application.
 * The runner application should set CONFIG_MDM_LED=y and CONFIG_MDM_LED_RUNNER=y
 * in the prj.conf file of the application.
 */

#ifndef _MULTI_DOMAIN_MODULES_LED_SHARED_ZBUS_H_
#define _MULTI_DOMAIN_MODULES_LED_SHARED_ZBUS_H_

#include <zephyr/zbus/zbus.h>

enum led_msg_type {
	LED_RGB_SET,
};

struct led_msg {
	enum led_msg_type type;

	/** RGB values (0 to 255) */
	uint8_t red;
	uint8_t green;
	uint8_t blue;

	/** Duration of the RGB on/off cycle */
	uint32_t duration_on_msec;
	uint32_t duration_off_msec;

	/** Number of on/off cycles (-1 indicates forever) */
	int repetitions;
};

/* Helper macro to invert CONFIG_MDM_LED_MASTER */
#if IS_ENABLED(CONFIG_MDM_LED_RUNNER)
#define MDM_LED_CHANNEL_IS_LISTENER 0
#else
#define MDM_LED_CHANNEL_IS_LISTENER 1
#endif

ZBUS_MULTIDOMAIN_CHAN_DEFINE(LED_CHAN,
			     struct led_msg,
			     NULL,
			     NULL,
			     ZBUS_OBSERVERS_EMPTY,
			     ZBUS_MSG_INIT(0),
			     MDM_LED_CHANNEL_IS_LISTENER, /* Runner is listener */
			     IS_ENABLED(CONFIG_MDM_LED));

#endif /* _MULTI_DOMAIN_MODULES_LED_SHARED_ZBUS_H_ */
