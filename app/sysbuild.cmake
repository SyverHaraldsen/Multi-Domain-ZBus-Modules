# Copyright (c) 2025 Nordic Semiconductor ASA
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause

if(CONFIG_MDM_CHANNEL_SOUNDING)
	include(${CMAKE_CURRENT_LIST_DIR}/../modules/channel_sounding/sysbuild_fragment.cmake)
endif()
