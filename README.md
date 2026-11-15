<p align="center">
  <img src="assets/ats-mini-banner.svg" alt="ATS Mini Firmware banner" width="100%">
</p>

<p align="center">
  <a href="#overview"><img src="https://img.shields.io/badge/Platform-ESP32--S3-1d4ed8?style=for-the-badge" alt="Platform ESP32-S3" /></a>
  <a href="#overview"><img src="https://img.shields.io/badge/Device-ATS%20Mini-16a34a?style=for-the-badge" alt="Device ATS Mini" /></a>
  <a href="#flashing"><img src="https://img.shields.io/badge/Firmware-Recovery%20Image-0ea5e9?style=for-the-badge" alt="Firmware Recovery Image" /></a>
  <a href="#license"><img src="https://img.shields.io/badge/License-Unspecified-red?style=for-the-badge" alt="License Unspecified" /></a>
</p>

<p align="center">
  <strong>ATS Mini Firmware Recovery Pack</strong><br>
  Flash-ready bootloader, partition, boot image, and main firmware files for compatible ATS Mini hardware.
</p>

---

## Overview

This repository contains the binary assets required to restore, update, or repair a compatible ATS Mini device. It includes the bootloader, partition table, app boot image, and the main firmware image, along with the flash offsets needed to write each file to the correct location in the device memory.

> Warning: verify the exact hardware model and revision before flashing any image. An incorrect firmware can render the device unusable. It is strongly recommended to back up the current firmware before making changes.

## Repository contents

```text
.
├── ATSMINI/
│   ├── FLASH-OFFSETS.txt
│   ├── boot_app0.bin
│   ├── bootloader.bin
│   ├── firmware.bin
│   └── partitions.bin
├── assets/
│   └── ats-mini-banner.svg
├── ci/
│   ├── platformio-release.ini
│   └── ...
├── docs/
│   └── FLASHING.md
├── README.md
├── Display.h
├── Languages.h
├── SSBPatch.h
├── main.cpp
├── include/
│   └── Remote.h
└── .github/
    └── workflows/
        └── firmware.yml
```

## Firmware files

| File | Description | Flash offset |
| --- | --- | ---: |
| `ATSMINI/bootloader.bin` | Bootloader | `0x00000` |
| `ATSMINI/partitions.bin` | Partition table | `0x08000` |
| `ATSMINI/boot_app0.bin` | Boot image | `0x0E000` |
| `ATSMINI/firmware.bin` | Main firmware | `0x10000` |

## Requirements

- A compatible ATS Mini unit
- USB data cable and an available serial port
- Python 3.x
- [esptool](https://github.com/espressif/esptool) installed

Install `esptool`:

```bash
python -m pip install esptool
```

## Flashing

1. Clone or download this repository.
2. Connect the device and identify the serial port (`COMx` on Windows or `/dev/tty...` on Linux/macOS).
3. From the project root, run:

```bash
python -m esptool --port <PORT> write-flash \
  0x00000 ATSMINI/bootloader.bin \
  0x08000 ATSMINI/partitions.bin \
  0x0E000 ATSMINI/boot_app0.bin \
  0x10000 ATSMINI/firmware.bin
```

Replace `<PORT>` with the actual device port. Command syntax may vary slightly depending on the installed `esptool` version; consult `python -m esptool --help` if needed.

Do not erase the entire flash unless the device-specific documentation explicitly instructs you to do so. For an expanded procedure, see [`docs/FLASHING.md`](docs/FLASHING.md). The exact offsets are also listed in [`ATSMINI/FLASH-OFFSETS.txt`](ATSMINI/FLASH-OFFSETS.txt).

## Build and Schematik sync

GitHub Actions builds the firmware for pushes and pull requests. Versioned releases are built only from `X.Y.Z` tags that match `FIRMWARE_VERSION` in `main.cpp`.

Schematik is an external service and is not configured by this repository. To have it receive source updates, configure its project to use this GitHub repository and the `main` branch. Changes on feature branches are not available to that deployment until merged into `main`. The GitHub workflow builds and publishes files; it does not deploy to Schematik or flash a radio.

## Safety and compatibility

- Confirm the exact hardware model and revision before flashing.
- Use only verified images compatible with your device.
- Back up the existing firmware if the radio contains important configuration or user data.
- This repository does not replace the manufacturer's official documentation or hardware warranty.
- Do not publish sensitive details such as serial numbers, credentials, or personal data during support discussions.

## Troubleshooting

If the write fails:

1. Verify the serial port and USB cable.
2. Try another data cable or serial port.
3. Check the installed `esptool` version and command syntax.
4. Confirm that the image matches the target device and revision.
5. Make sure the ATS Mini is in the correct programming mode.

## Support

When reporting a problem, include:

- ATS Mini model and revision
- Operating system
- `esptool` version
- Serial port used
- Exact error message

## License

This repository does not declare an explicit license. It should not be assumed that the firmware, source files, or binaries may be reused, redistributed, or modified freely without first confirming the rights of the copyright holder.

## Notes

This package is intended for technical recovery, maintenance, and firmware update workflows. If your goal is active development or modifications to the project itself, review the source code structure and related documentation before making functional changes.
