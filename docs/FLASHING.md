# Professional flashing guide for ATS Mini

This guide documents the recommended procedure for writing the ATS Mini firmware image to the device flash safely and reproducibly.

## Flash offsets summary

```text
0x00000  bootloader.bin
0x08000  partitions.bin
0x0E000  boot_app0.bin
0x10000  firmware.bin
```

## Prerequisites

- Python 3.x
- [esptool](https://github.com/espressif/esptool) installed
- An available serial port
- ATS Mini connected correctly and in programming mode
- Backup of the original firmware if the device contains important configuration or user data

## Install esptool

```bash
python -m pip install esptool
```

## Flashing procedure

Run the following from the repository root:

```bash
python -m esptool --port <PORT> write-flash \
  0x00000 ATSMINI/bootloader.bin \
  0x08000 ATSMINI/partitions.bin \
  0x0E000 ATSMINI/boot_app0.bin \
  0x10000 ATSMINI/firmware.bin
```

Replace `<PORT>` with the actual serial port on your system (`COMx` on Windows or `/dev/tty...` on Linux/macOS).

## Important considerations

- Verify hardware compatibility before beginning.
- An incorrect or incompatible firmware image may render the device unusable.
- Do not erase the full flash unless the hardware-specific documentation explicitly allows it.
- Keep a backup of the prior state whenever possible.

## Quick troubleshooting

If the flashing process fails:

1. Check the serial port and USB cable.
2. Try another port or a different data cable.
3. Verify the installed `esptool` version.
4. Confirm that the files match your device and revision.
5. Ensure the device is entering programming mode correctly.

## References

- Main project README: `../README.md`
- Detailed offsets: `ATSMINI/FLASH-OFFSETS.txt`
- Firmware source: `main.cpp`

## Safety note

Firmware flashing is a sensitive operation. If you are not certain that the device and image are compatible, do not proceed until that compatibility has been verified.
