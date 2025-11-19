# Multi-Domain ZBus Modules

A modular architecture for applications using Zephyr RTOS and ZBus proxy agents for inter-domain communication.

## Modules

This project includes three example modules that demonstrate inter-module communication using ZBus channels and proxy agents:

### LED Module (`modules/led`)
Controls RGB LEDs based on commands received via ZBus. The module listens to the `LED_CHAN` channel and executes LED operations (color, blinking patterns, duration).

- **ZBus Role**: Listener
- **Channel**: `LED_CHAN`
- **Runner**: Typically runs on the domain with direct LED hardware access
- **Use Case**: Visual feedback, status indication, user notifications

### BLE NUS Module (`modules/ble_nus`)
Provides Bluetooth Low Energy Nordic UART Service functionality. Publishes received BLE data to the `BLE_NUS_CHAN` channel.

- **ZBus Role**: Publisher
- **Channel**: `BLE_NUS_CHAN`
- **Runner**: Runs on the domain with Bluetooth stack
- **Use Case**: Wireless communication, data exchange, mobile app integration
- **BLE Role**: Peripheral (accepts connections from phones/centrals)

### Channel Sounding Module (`modules/channel_sounding`)
Implements Bluetooth Channel Sounding as an initiator that ranges with reflector devices. Publishes distance measurements to the `CS_DISTANCE_CHAN` channel.

- **ZBus Role**: Publisher
- **Channel**: `CS_DISTANCE_CHAN`
- **Runner**: Runs on the domain with Channel Sounding capability
- **Use Case**: Distance measurement, proximity detection, asset tracking
- **BLE Role**: Central (initiates connections to reflectors)

Each module is self-contained with its own Kconfig, CMakeLists.txt, and ZBus channel definitions, making it easy to enable/disable modules based on your application needs.

## Getting Started

### Initialize West Module

This repository is designed to be integrated as a west module into your nRF Connect SDK project.

<details>
<summary><strong>Add to your west.yml manifest:</strong></summary>

Add the following to your project's `west.yml` file under `projects`:

```yaml
- name: multi-domain-modules
  url: https://github.com/SyverHaraldsen/Multi-Domain-ZBus-Modules.git
  revision: main
  path: project/multi-domain-modules
```

Then update your workspace:

```shell
west update
```

</details>

<details>
<summary><strong>Optional: Standalone workspace setup:</strong></summary>

If you want to work with this module as a standalone project:

```shell
# Install nRF Util
pip install nrfutil

# or follow install [documentation](https://docs.nordicsemi.com/bundle/nrfutil/page/guides/installing.html)

# Install toolchain
nrfutil toolchain-manager install --ncs-version v3.1.0

# Launch toolchain
nrfutil toolchain-manager launch --ncs-version v3.1.0 --shell

# Initialize workspace
west init -m https://github.com/SyverHaraldsen/Multi-Domain-ZBus-Modules.git --mr main multi-domain-zbus-modules
cd multi-domain-zbus-modules/project
west update
```

</details>

### Enable/Disable Modules

Modules are enabled through Kconfig options in your `prj.conf` file.

#### Available Modules

- **LED Module** - `CONFIG_MDM_LED`
- **BLE NUS Module** - `CONFIG_MDM_BLE_NUS`
- **Channel Sounding Module** - `CONFIG_MDM_CHANNEL_SOUNDING`

#### To Enable a Module

Add the configuration to your `app/prj.conf`:

```conf
# Enable LED module
CONFIG_MDM_LED=y
CONFIG_MDM_LED_RUNNER=y

# Enable BLE NUS module
CONFIG_MDM_BLE_NUS=y
CONFIG_MDM_BLE_NUS_RUNNER=y
CONFIG_MDM_BLE_NUS_ZBUS_LOGGING=y

# Enable Channel Sounding module
CONFIG_MDM_CHANNEL_SOUNDING=y
CONFIG_MDM_CHANNEL_SOUNDING_RUNNER=y

# ZBus multi-domain support
CONFIG_ZBUS=y
CONFIG_ZBUS_CHANNEL_NAME=y
CONFIG_ZBUS_MSG_SUBSCRIBER=y
CONFIG_POLL=y
CONFIG_ZBUS_PROXY_AGENT=y

# ZBus UART backend
CONFIG_CRC=y
CONFIG_SERIAL=y
CONFIG_UART_ASYNC_API=y
CONFIG_ZBUS_PROXY_AGENT_UART=y
```

The `_RUNNER` suffix indicates that the module runs on the current image/domain. Each module uses ZBus proxy agents for inter-domain communication via UART.

**Device Tree Configuration:** You also need to define a UART proxy agent in your board overlay file:

```dts
uart_proxy_agent: uart-proxy {
    compatible = "zephyr,zbus-proxy-agent-uart";
    status = "okay";
    uart-device = <&uart30>;
};
```

#### To Disable a Module

Simply remove the configuration from `prj.conf` or set it to `n`:

```conf
CONFIG_MDM_LED=n
```

<details>
<summary><strong>Create Your Own Module</strong></summary>

Create a folder for your module in the `modules/` directory (e.g., `modules/my_module`).

#### 1. Create Core Files

Create the following files in your module directory:

**`CMakeLists.txt`** - Build configuration:
```cmake
target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/my_module.c)

target_include_directories(app PRIVATE .)

# Include files that are common for all modules
target_include_directories(app PRIVATE ../common)
```

**`Kconfig.multidomain`** - Module enablement (available to ALL domains):
```kconfig
menuconfig MDM_MY_MODULE
	bool "My Module"
	help
	  Enable My Module
```

**Important:** This file is available to all domains/images, even if they're not running the module. This allows every domain to know about the module's existence.

**`Kconfig.my_module`** - Module-specific configuration (runner domain only):
```kconfig
rsource "Kconfig.multidomain"

if MDM_MY_MODULE

config MDM_MY_MODULE_RUNNER
	bool "Module is running on this image"
	default n
	select REQUIRED_DEPENDENCY_1
	select REQUIRED_DEPENDENCY_2
	help
	  Select if this module is running on this image.

if MDM_MY_MODULE_RUNNER

module = MDM_MY_MODULE
module-str = mdm_my_module
source "subsys/logging/Kconfig.template.log_config"

endif # MDM_MY_MODULE_RUNNER

endif # MDM_MY_MODULE
```

**Important:** This file is where you `select` dependencies and configure what the module needs to run (BLE stack, sensors, etc). Only the domain running the module includes this file.

**`my_module.h`** - Module header:
```c
#ifndef MY_MODULE_H_
#define MY_MODULE_H_

#include <zephyr/zbus/zbus.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Channels provided by this module */
ZBUS_CHAN_DECLARE(MY_MODULE_CHAN);

enum my_module_msg_type {
	MY_MODULE_DATA,
};

struct my_module_msg {
	enum my_module_msg_type type;
	uint8_t data;
	uint32_t timestamp;
};

#ifdef __cplusplus
}
#endif

#endif /* MY_MODULE_H_ */
```

**`my_module.c`** - Module implementation:
```c
#include "my_module.h"
#include <zephyr/zbus/zbus.h>
#include <zephyr/zbus/proxy_agent/zbus_proxy_agent.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(my_module, CONFIG_MDM_MY_MODULE_LOG_LEVEL);

#ifndef MDM_MY_MODULE_PROXY_NODE
#error "MDM_MY_MODULE_PROXY_NODE must be defined to use multi-domain zbus channels"
#endif

/* The module runner has the main channel */
ZBUS_CHAN_DEFINE(
	MY_MODULE_CHAN,
	struct my_module_msg,
	NULL,
	NULL,
	ZBUS_OBSERVERS_EMPTY,
	ZBUS_MSG_INIT(0)
);

ZBUS_PROXY_ADD_CHAN(MDM_MY_MODULE_PROXY_NODE, MY_MODULE_CHAN);

int my_module_init(void)
{
	LOG_INF("My Module initialized");
	return 0;
}

SYS_INIT(my_module_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
```

**`remote_zbus.c`** - Remote ZBus shadow channel (for non-runner domains):
```c
#include <zephyr/zbus/zbus.h>
#include <zephyr/zbus/proxy_agent/zbus_proxy_agent.h>
#include "my_module.h"

#ifndef MDM_MY_MODULE_PROXY_NODE
#error "MDM_MY_MODULE_PROXY_NODE must be defined to use multi-domain zbus channels"
#endif

/* The module runner has the main channel, controller the shadow channel */
ZBUS_SHADOW_CHAN_DEFINE(
	MY_MODULE_CHAN,
	struct my_module_msg,
	MDM_MY_MODULE_PROXY_NODE,
	NULL,
	ZBUS_OBSERVERS_EMPTY,
	ZBUS_MSG_INIT(0)
);
```

#### 2. Register Module in App Configuration

**In `app/Kconfig`**, add:
```kconfig
rsource "../modules/my_module/Kconfig.my_module"
```

**In `app/CMakeLists.txt`**, add:
```cmake
# Add proxy node definitions for your module
target_compile_definitions(app PRIVATE "MDM_MY_MODULE_PROXY_NODE=DT_NODELABEL(uart_proxy_agent)")

# Add the module runner
add_subdirectory_ifdef(CONFIG_MDM_MY_MODULE_RUNNER ../modules/my_module ${CMAKE_BINARY_DIR}/modules/my_module)
```

**In `modules/CMakeLists.txt`**, add:
```cmake
target_sources_ifdef(CONFIG_MDM_MY_MODULE app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/my_module/remote_zbus.c)
target_include_directories_ifdef(CONFIG_MDM_MY_MODULE app PRIVATE my_module)
```

#### 3. Enable Your Module

Add to `app/prj.conf`:
```conf
CONFIG_MDM_MY_MODULE=y
CONFIG_MDM_MY_MODULE_RUNNER=y
```

#### Optional: Add Sysbuild Partial Configs

If your module needs to configure other images (e.g., network core, radio core):

**Determine your image name** - Common names are `ipc_radio`, `hci_ipc`, `remote`, etc. For nRF54L with Bluetooth, use `ipc_radio`.

**Create `sysbuild/<image_name>/Kconfig.<image_name>`** - Configuration for the remote image:

Example for `ipc_radio` image:
```kconfig
# Copyright (c) 2025 Nordic Semiconductor
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause

config ipc_radio_CONFIG_IPC_SERVICE
	bool
	default y

config ipc_radio_CONFIG_MBOX
	bool
	default y

config ipc_radio_CONFIG_HEAP_MEM_POOL_SIZE
	int
	default 4096
```

**Note:** The config symbol prefix must match the image name (e.g., `ipc_radio_CONFIG_*` for the `ipc_radio` image).

**Create `Kconfig.sysbuild`** - Sysbuild-level Kconfig options:
```kconfig
if MDM_MY_MODULE

config NRF_DEFAULT_BLUETOOTH
	default y

config NETCORE_IPC_RADIO_BT_HCI_IPC
	default y

rsource "sysbuild/ipc_radio/Kconfig.ipc_radio"

endif # MDM_MY_MODULE
```

**Register in `app/Kconfig.sysbuild`:**
```kconfig
rsource "../modules/my_module/Kconfig.sysbuild"
```

</details>
