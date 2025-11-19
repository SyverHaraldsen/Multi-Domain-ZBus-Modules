/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/zbus/zbus.h>
#include <zephyr/zbus/proxy_agent/zbus_proxy_agent.h>

#include "channel_sounding.h"

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
