/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/zbus/zbus.h>
#include <zephyr/zbus/proxy_agent/zbus_proxy_agent.h>

#include "channel_sounding.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(mdm_channel_sounding_module, CONFIG_APP_LOG_LEVEL);

#ifndef MDM_CHANNEL_SOUNDING_PROXY_NODE
#error "MDM_CHANNEL_SOUNDING_PROXY_NODE must be defined to use multi-domain zbus channels"
#endif

/* This file is for the non-runner/controller side: the controller has the shadow channel, and the
 * runner has the main channel
 */
ZBUS_SHADOW_CHAN_DEFINE(
	CS_DISTANCE_CHAN,
	struct cs_distance_msg,
	MDM_CHANNEL_SOUNDING_PROXY_NODE,
	NULL,
	ZBUS_OBSERVERS_EMPTY,
	ZBUS_MSG_INIT(0)
);

#if IS_ENABLED(CONFIG_MDM_CHANNEL_SOUNDING_ZBUS_LOGGING)

static void log_cs_message(const struct zbus_channel *chan)
{
	const struct cs_distance_msg *msg = zbus_chan_const_msg(chan);
	LOG_INF("=== Channel Sounding ZBUS Message Received ===");
	LOG_INF("Type: %s", cs_message_type_to_string(msg->type));
	LOG_INF("Timestamp: %u ms", msg->timestamp);
	LOG_INF("Antenna Path: %u", msg->antenna_path);
	LOG_INF("Distance Estimates (meters): IFFT: %.2f, Phase Slope: %.2f, RTT: %.2f",
		(double)msg->ifft, (double)msg->phase_slope, (double)msg->rtt);
	LOG_INF("=============================================");
}

ZBUS_LISTENER_DEFINE(cs_logger, log_cs_message);
ZBUS_CHAN_ADD_OBS(CS_DISTANCE_CHAN, cs_logger, 0);

#endif /* CONFIG_MDM_CHANNEL_SOUNDING_ZBUS_LOGGING */
