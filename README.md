# Multi-Domain ZBus Modules

A modular architecture for applications using Zephyr RTOS and ZBus for inter-domain communication.

## Modules

This project includes three example modules that demonstrate inter-module communication using ZBus channels:

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
- **Note**: Cannot be enabled simultaneously with Channel Sounding module

### Channel Sounding Module (`modules/channel_sounding`)
Implements Bluetooth Channel Sounding as an initiator that ranges with reflector devices. Publishes distance measurements to the `CS_DISTANCE_CHAN` channel.

- **ZBus Role**: Publisher
- **Channel**: `CS_DISTANCE_CHAN`
- **Runner**: Runs on the domain with Channel Sounding capability
- **Use Case**: Distance measurement, proximity detection, asset tracking
- **Note**: Cannot be enabled simultaneously with BLE NUS module

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

# Enable Channel Sounding module
CONFIG_MDM_CHANNEL_SOUNDING=y
CONFIG_MDM_CHANNEL_SOUNDING_RUNNER=y
```

The `_RUNNER` suffix indicates that the module runs on the current image/domain. If the module is enabled, this config will determine how to set up the multidomain ZBus channels.

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

**`Kconfig.multidomain_channels`** - Module enablement:
```kconfig
menuconfig MDM_MY_MODULE
	bool "My Module"
	help
	  Enable My Module

if MDM_MY_MODULE

config MDM_MY_MODULE_RUNNER
	bool "Module is running on this image"
	default n
	help
	  Select if this module is running on this image.

endif
```

**`Kconfig.my_module`** - Module-specific configuration:
```kconfig
rsource "Kconfig.multidomain_channels"

if MDM_MY_MODULE_RUNNER

module = MDM_MY_MODULE
module-str = MY_MODULE
source "subsys/logging/Kconfig.template.log_config"

endif

endif # MDM_MY_MODULE_RUNNER
```

**`my_module.conf`** - Module overlay configuration:
```conf
# Module-specific configuration options
```

**`my_module.h`** - Module header:
```c
#ifndef MY_MODULE_H_
#define MY_MODULE_H_

#endif /* MY_MODULE_H_ */
```

**`my_module.c`** - Module implementation:
```c
#include "my_module.h"
#include "shared_zbus_definition.h"
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(my_module, CONFIG_MDM_MY_MODULE_LOG_LEVEL);

int my_module_init(void)
{
	LOG_INF("My Module initialized");
	return 0;
}

SYS_INIT(my_module_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
```

**`shared_zbus.h`** - ZBus message types and channel declaration:
```c
#ifndef _MULTI_DOMAIN_MODULES_MY_MODULE_SHARED_ZBUS_H_
#define _MULTI_DOMAIN_MODULES_MY_MODULE_SHARED_ZBUS_H_

#include <zephyr/zbus/zbus.h>

enum my_module_msg_type {
	MY_MODULE_DATA,
};

struct my_module_msg {
	enum my_module_msg_type type;
	uint8_t data;
	uint32_t timestamp;
};

ZBUS_CHAN_DECLARE(MY_MODULE_CHAN);

#endif
```

**`shared_zbus_definition.h`** - ZBus channel definition:
```c
#ifndef _MULTI_DOMAIN_MODULES_MY_MODULE_SHARED_ZBUS_DEFINITION_H_
#define _MULTI_DOMAIN_MODULES_MY_MODULE_SHARED_ZBUS_DEFINITION_H_

#include "shared_zbus.h"

#if IS_ENABLED(CONFIG_ZBUS_MULTIDOMAIN)
ZBUS_MULTIDOMAIN_CHAN_DEFINE(MY_MODULE_CHAN,
			     struct my_module_msg,
			     NULL,
			     NULL,
			     ZBUS_OBSERVERS_EMPTY,
			     ZBUS_MSG_INIT(0),
			     IS_ENABLED(CONFIG_MDM_MY_MODULE_RUNNER),
			     IS_ENABLED(CONFIG_MDM_MY_MODULE));
#else
ZBUS_CHAN_DEFINE(MY_MODULE_CHAN,
		 struct my_module_msg,
		 NULL,
		 NULL,
		 ZBUS_OBSERVERS_EMPTY,
		 ZBUS_MSG_INIT(0));
#endif

#endif
```

#### 2. Register Module in App Configuration

**In `app/Kconfig`**, add:
```kconfig
source "$(APPLICATION_SOURCE_DIR)/../modules/my_module/Kconfig.my_module"
```

**In `app/CMakeLists.txt`**, add:

In the config fragment section:
```cmake
add_config_fragment_ifdef(CONFIG_MDM_MY_MODULE
	"${CMAKE_CURRENT_SOURCE_DIR}/../modules/my_module/my_module.conf")
```

In the subdirectory section:
```cmake
add_subdirectory_ifdef(CONFIG_MDM_MY_MODULE_RUNNER ../modules/my_module ${CMAKE_BINARY_DIR}/modules/my_module)
```

#### 3. Enable Your Module

Add to `app/prj.conf`:
```conf
CONFIG_MDM_MY_MODULE=y
CONFIG_MDM_MY_MODULE_RUNNER=y
```

#### Optional: Add Sysbuild Partial Configs

If your module needs to configure other images (e.g., network core, radio core):

**Create `sysbuild/hci_ipc/prj.conf`** - Configuration for the radio/network core:
```conf
# Module-specific radio core configuration
CONFIG_IPC_SERVICE=y
CONFIG_MBOX=y
CONFIG_HEAP_MEM_POOL_SIZE=4096
```

**Create `sysbuild_fragment.cmake`** - Include the partial config:
```cmake
list(APPEND ipc_radio_CONF_FILE ${CMAKE_CURRENT_LIST_DIR}/sysbuild/hci_ipc/prj.conf)
```

**Create `Kconfig.sysbuild`** - Sysbuild-level Kconfig options:
```kconfig
if MDM_MY_MODULE

config NRF_DEFAULT_BLUETOOTH
	default y

config NETCORE_IPC_RADIO_BT_HCI_IPC
	default y

endif
```

**Register in `app/Kconfig.sysbuild`:**
```kconfig
rsource "../modules/my_module/Kconfig.sysbuild"
```

**Register in `app/sysbuild.cmake`:**
```cmake
if(CONFIG_MDM_MY_MODULE)
	include(${CMAKE_CURRENT_LIST_DIR}/../modules/my_module/sysbuild_fragment.cmake)
endif()
```

</details>
