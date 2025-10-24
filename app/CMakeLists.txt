#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project("Multi-Domain-Modules")

target_sources(app PRIVATE src/main.c)

add_subdirectory_ifdef(CONFIG_MDM_BLE_NUS_RUNNER ../modules/ble_nus ${CMAKE_BINARY_DIR}/modules/ble_nus)
add_subdirectory_ifdef(CONFIG_MDM_LED_RUNNER ../modules/led ${CMAKE_BINARY_DIR}/modules/led)
add_subdirectory_ifdef(CONFIG_MDM_CHANNEL_SOUNDING_RUNNER ../modules/channel_sounding ${CMAKE_BINARY_DIR}/modules/channel_sounding)
