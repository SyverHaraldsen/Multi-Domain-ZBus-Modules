# Initial push of Multi-Domain Zbus Modules repository

### Setup

<details>
<summary>1. <strong>Initialize workspace:</strong></summary>

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
