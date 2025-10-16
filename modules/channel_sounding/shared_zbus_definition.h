#ifndef _MULTI_DOMAIN_MODULES_CHANNEL_SOUNDING_SHARED_ZBUS_DEFINITION_H_
#define _MULTI_DOMAIN_MODULES_CHANNEL_SOUNDING_SHARED_ZBUS_DEFINITION_H_

#include "shared_zbus.h"

#if IS_ENABLED(CONFIG_ZBUS_MULTIDOMAIN)
/* When multidomain is enabled, define as a multidomain channel */
ZBUS_MULTIDOMAIN_CHAN_DEFINE(CS_DISTANCE_CHAN,
			     struct cs_distance_msg,
			     NULL,
			     NULL,
			     ZBUS_OBSERVERS_EMPTY,
			     ZBUS_MSG_INIT(0),
			     IS_ENABLED(CONFIG_MDM_CHANNEL_SOUNDING_RUNNER), /* Runner is master */
			     IS_ENABLED(CONFIG_MDM_CHANNEL_SOUNDING));
#else
/* When multidomain is disabled, define as a regular local-only channel */
ZBUS_CHAN_DEFINE(CS_DISTANCE_CHAN,
		 struct cs_distance_msg,
		 NULL,
		 NULL,
		 ZBUS_OBSERVERS_EMPTY,
		 ZBUS_MSG_INIT(0));
#endif /* CONFIG_ZBUS_MULTIDOMAIN */

#endif /* _MULTI_DOMAIN_MODULES_CHANNEL_SOUNDING_SHARED_ZBUS_DEFINITION_H_ */

