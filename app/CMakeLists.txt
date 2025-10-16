#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

# Function to conditionally add config fragments based on Kconfig settings in prj.conf.
# This allows modules to have their own partial .conf files that are automatically included
#
# Note: OVERLAY_CONFIG must be set before find_package(Zephyr), but at this point Kconfig
# variables are not yet available. Therefore we parse prj.conf manually to determine which
# config fragments to include.
function(add_config_fragment_ifdef kconfig_option config_fragment_file)
	if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/prj.conf)
		file(READ ${CMAKE_CURRENT_SOURCE_DIR}/prj.conf prj_conf_content)
		string(REGEX MATCH "${kconfig_option}=y" config_enabled "${prj_conf_content}")
		if(config_enabled AND EXISTS ${config_fragment_file})
			list(APPEND OVERLAY_CONFIG ${config_fragment_file})
			set(OVERLAY_CONFIG ${OVERLAY_CONFIG} PARENT_SCOPE)
			message(STATUS "Adding config fragment: ${config_fragment_file}")
		endif()
	endif()
endfunction()

add_config_fragment_ifdef(CONFIG_MDM_CHANNEL_SOUNDING
	"${CMAKE_CURRENT_SOURCE_DIR}/../modules/channel_sounding/channel_sounding.conf")

add_config_fragment_ifdef(CONFIG_MDM_BLE_NUS
	"${CMAKE_CURRENT_SOURCE_DIR}/../modules/ble_nus/ble_nus.conf")

add_config_fragment_ifdef(CONFIG_MDM_LED
	"${CMAKE_CURRENT_SOURCE_DIR}/../modules/led/led.conf")

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project("Multi-Domain-Modules")

target_sources(app PRIVATE src/main.c)

add_subdirectory_ifdef(CONFIG_MDM_BLE_NUS ../modules/ble_nus ${CMAKE_BINARY_DIR}/modules/ble_nus)
add_subdirectory_ifdef(CONFIG_MDM_LED ../modules/led ${CMAKE_BINARY_DIR}/modules/led)
add_subdirectory_ifdef(CONFIG_MDM_CHANNEL_SOUNDING ../modules/channel_sounding ${CMAKE_BINARY_DIR}/modules/channel_sounding)
