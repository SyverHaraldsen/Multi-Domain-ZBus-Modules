#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project("Multi-Domain-Modules")

target_sources(app PRIVATE src/main.c)

add_subdirectory_ifdef(CONFIG_MDM_BLE ../modules/BLE ${CMAKE_BINARY_DIR}/modules/BLE)
add_subdirectory_ifdef(CONFIG_MDM_LED ../modules/led ${CMAKE_BINARY_DIR}/modules/led)
