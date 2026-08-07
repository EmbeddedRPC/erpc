# eRPC RT-Thread integration

This directory provides RT-Thread integration files for the eRPC C runtime.

## Files

- `Kconfig`: RT-Thread `menuconfig` options for selecting eRPC runtime components.
- `SConscript`: RT-Thread SCons build script for compiling the selected eRPC sources.

The eRPC generic CMake source list is not changed by this integration. RT-Thread builds use the files in this directory through RT-Thread `menuconfig` and SCons.

## Required RT-Thread features

The package configuration selects the following RT-Thread features when `PKG_USING_ERPC` is enabled:

- `RT_USING_CPLUSPLUS`
- `RT_USING_HEAP`
- `RT_USING_MUTEX`
- `RT_USING_SEMAPHORE`

For the RT-Thread UART transport, the BSP must also provide device and serial support:

- `RT_USING_DEVICE`
- `RT_USING_SERIAL`

## Kconfig entry

Add the eRPC RT-Thread Kconfig file to the RT-Thread package or project Kconfig entry used by your BSP.

For example, when eRPC is placed under a package directory that is reachable from the BSP Kconfig, add a `source` line similar to:

```text
source "$PKGS_DIR/erpc/rtthread/Kconfig"
```

Adjust the path to match the package layout used by your RT-Thread project.

## Configure with menuconfig

Open RT-Thread configuration from the BSP root directory:

```sh
scons --menuconfig
```

Then enable:

```text
eRPC: Embedded RPC framework
```

Select at least one runtime role:

- `Enable client runtime`, if generated client stubs run on the RT-Thread target.
- `Enable server runtime`, if generated server stubs run on the RT-Thread target.

For UART transport, enable:

```text
RT-Thread UART transport
```

The UART transport option depends on RT-Thread device and serial support.

## SCons entry

This integration keeps the RT-Thread build logic in `rtthread/SConscript` and does not require the eRPC repository to carry a root-level `SConscript` file.

If your RT-Thread package flow expects a `SConscript` file at the eRPC repository root, create a small root-level shim in your local package copy:

```python
#
# RT-Thread package entry for eRPC.
#
# SPDX-License-Identifier: BSD-3-Clause
#

from building import *
import os

cwd = GetCurrentDir()
group = SConscript(os.path.join(cwd, 'rtthread', 'SConscript'))

Return('group')
```

The shim only forwards the build to `rtthread/SConscript`. The actual source selection remains controlled by RT-Thread `menuconfig` options.

## Configuration header

The RT-Thread SCons integration adds this include path before the generic eRPC configuration directory:

```text
erpc_c/port/rtthread
```

This makes `erpc_c/port/rtthread/erpc_config.h` the eRPC configuration header for RT-Thread package builds.

The RT-Thread configuration header includes `rtconfig.h`, disables host POSIX/Win32 auto-detection for the RT-Thread package build, and selects the native RT-Thread backend:

```c
#define ERPC_THREADS (ERPC_THREADS_RTTHREAD)
#define ERPC_ALLOCATION_POLICY (ERPC_ALLOCATION_POLICY_DYNAMIC)
```

Thread defaults and UART transport registry size are controlled by the `PKG_ERPC_*` options selected in `menuconfig`.

## UART transport usage

The RT-Thread UART transport uses the RT-Thread device API. The device name must match the BSP UART device name, for example:

```c
erpc_transport_t transport = erpc_transport_rtthread_uart_init("uart2", 115200);
```

The UART transport owns the RT-Thread `rx_indicate` callback while active. Applications should not install another receive callback on the same UART device during the transport lifetime.

The transport registry is updated during init/deinit. Do not deinitialize a transport while it is being used by an eRPC client/server or while another thread may access the same transport.
