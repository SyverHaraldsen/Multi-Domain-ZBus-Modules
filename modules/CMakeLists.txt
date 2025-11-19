#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

target_sources_ifdef(CONFIG_MDM_LED app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/led/remote_zbus.c)
target_include_directories_ifdef(CONFIG_MDM_LED app PRIVATE led)

target_sources_ifdef(CONFIG_MDM_BLE_NUS app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/ble_nus/remote_zbus.c)
target_include_directories_ifdef(CONFIG_MDM_BLE_NUS app PRIVATE ble_nus)

target_sources_ifdef(CONFIG_MDM_CHANNEL_SOUNDING app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/channel_sounding/remote_zbus.c)
target_include_directories_ifdef(CONFIG_MDM_CHANNEL_SOUNDING app PRIVATE channel_sounding)
