# Copyright (c) 2025 Nordic Semiconductor ASA
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause

target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/channel_sounding.c)

target_include_directories(app PRIVATE .)

# Include files that are common for all modules
target_include_directories(app PRIVATE ../common)
