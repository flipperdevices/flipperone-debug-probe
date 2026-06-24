# Debug Probe for Flipper One 
Hardware debug probe for Flipper One MCU and CPU debug based on RP2350  
<img width="500" alt="flipperone_debug_probe" src="https://github.com/user-attachments/assets/5e3bc268-aef0-4ffb-ba69-801cc303c311" />

## Automated firmware builds

Every push to `dev`, every version tag (`v*`), and every pull request triggers a CI build ([`.github/workflows/build.yml`](.github/workflows/build.yml)). The compiled `.uf2` is attached to each run as a downloadable artifact — open the run from the [**Actions**](../../actions) tab (or the **Checks** tab of a pull request) and download it from the **Artifacts** section.

Pushing a version tag additionally publishes a [**GitHub Release**](../../releases) with the firmware attached:

```shell
git tag v1.0.0
git push origin v1.0.0
```

## How to build

Install [VS Code](https://code.visualstudio.com/) with the [Raspberry Pi Pico extension](https://marketplace.visualstudio.com/items?itemName=raspberry-pi.raspberry-pi-pico). The extension automatically downloads the ARM toolchain, CMake, Ninja, and Pico SDK. Open the project folder and use the extension's compile button.

<details>
<summary>Manual build (Linux / macOS)</summary>

Prerequisites: [ARM GCC toolchain](https://github.com/xpack-dev-tools/arm-none-eabi-gcc-xpack/releases) (tested with 14.2.1), CMake 3.13+, [Pico SDK 2.2.0](https://github.com/raspberrypi/pico-sdk).

On macOS, the ARM toolchain can be installed via Homebrew:

```shell
brew install --cask gcc-arm-embedded
```

```shell
git clone --recursive https://github.com/flipperdevices/flipperone_debug_probe.git
cd flipperone_debug_probe

git clone -b 2.2.0 https://github.com/raspberrypi/pico-sdk.git ../pico-sdk
cd ../pico-sdk && git submodule update --init && cd ../flipperone_debug_probe

mkdir -p build && cd build
PICO_SDK_PATH=../../pico-sdk cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release --parallel
```

The output firmware file will be at `build/flipperone_debug_probe.uf2`.
</details>

## How to update firmware

1. Disconnect debug probe from USB
2. Hold `MCU Reset & Probe Boot`
3. While holding button connect to USB
4. RP2350 USB storage should appears
5. Copy firmware image to USB storage

## Schematic and gerber
Debug probe is fully open hardware on (WHAT LICENSE?) 
Download schematic, 3D models and gerber files here in Altium project: https://flipper.365.altium.com/designs/14B8CA82-B532-4581-BF6F-641FED8AF7F5

<img width="2800" height="1536" alt="image" src="https://github.com/user-attachments/assets/06c4a491-e4f4-407d-a966-894a8cb9f463" />

# Usage

## Serial Ports

The Flipper One Debug Probe is detected by the operating system as four serial ports.  
Port names and paths may vary depending on your operating system.

Example device paths on macOS:

| Port | Device path | Description | Baud rate |
| ---- | ----------- | ----------- | --------- |
| Port 1 | `/dev/tty.usbmodemflip_one_debug2` | RK3576 CPU console | `1500000` |
| Port 2 | `/dev/tty.usbmodemflip_one_debug4` | Flipper One MCU CLI | `230400` |
| Port 3 | `/dev/tty.usbmodemflip_one_debug6` | MCU debug log | `230400` |
| Port 4 | `/dev/tty.usbmodemflip_one_debug8` | Debug Probe MCU CLI | `230400` |

## Connect to the RK3576 CPU Console on macOS

This example shows how to connect to the RK3576 CPU console on macOS.

We recommend using [`tio`](https://github.com/tio/tio), because it is lightweight and stable. You can install it with `brew install tio`  

### Basic connection

`tio -b 1500000 /dev/tty.usbmodemflip_one_debug2`

### Connection with timestamps
Use timestamps to see delays between boot log lines and identify where the boot process slows down:  

`tio --timestamp --timestamp-format 24hour-delta -b 1500000 /dev/tty.usbmodemflip_one_debug2`
