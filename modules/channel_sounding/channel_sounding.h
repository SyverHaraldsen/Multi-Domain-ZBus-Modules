/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _MULTI_DOMAIN_MODULES_CHANNEL_SOUNDING_SHARED_ZBUS_H_
#define _MULTI_DOMAIN_MODULES_CHANNEL_SOUNDING_SHARED_ZBUS_H_

#include <zephyr/zbus/zbus.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Channels provided by this module */
ZBUS_CHAN_DECLARE(CS_DISTANCE_CHAN);

enum cs_msg_type {
	CS_DISTANCE_MEASUREMENT,
};

/**
 * @brief Channel Sounding distance measurement message
 *
 * Contains distance estimates from different measurement methods:
 * - IFFT: Inverse FFT based distance estimate
 * - Phase Slope: Phase slope based distance estimate
 * - RTT: Round Trip Time based distance estimate
 */
struct cs_distance_msg {
	enum cs_msg_type type;

	/** Antenna path number (0 to MAX_AP-1) */
	uint8_t antenna_path;

	/** Distance estimates in meters */
	float ifft;        /* IFFT-based distance estimate */
	float phase_slope; /* Phase slope-based distance estimate */
	float rtt;         /* RTT-based distance estimate */

	/** Timestamp when measurement was taken */
	uint32_t timestamp;
};

#ifdef __cplusplus
}
#endif

#endif /* _MULTI_DOMAIN_MODULES_CHANNEL_SOUNDING_SHARED_ZBUS_H_ */
