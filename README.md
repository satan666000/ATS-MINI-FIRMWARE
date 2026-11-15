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
  <strong>ATS Mini Firmware Recovery Pack / Paquete de recuperación del firmware ATS Mini</strong><br>
  Flash-ready bootloader, partition, boot image, and main firmware files for compatible ATS Mini hardware.<br>
  Archivos listos para flashear: bootloader, tabla de particiones, imagen de arranque y firmware para ATS Mini compatible.
</p>

<p align="center"><a href="#overview">English</a> | <a href="#español">Español</a></p>

---

## Overview

This repository contains the KALIMAN firmware source and the binary assets required to restore, update, or repair a compatible ATS Mini device. It includes the bootloader, partition table, app boot image, and main firmware image, along with the flash offsets needed to write each file to the correct location in device memory.

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

## Build

GitHub Actions builds the firmware for pushes and pull requests. Versioned releases are built only from `X.Y.Z` tags that match `FIRMWARE_VERSION` in `main.cpp`.

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

---

## Español

<p align="center">
  <strong>Paquete de recuperación del firmware ATS Mini</strong><br>
  Archivos listos para flashear: bootloader, tabla de particiones, imagen de arranque y firmware principal para ATS Mini compatible.
</p>

### Descripción general

Este repositorio contiene el código fuente del firmware KALIMAN y los archivos binarios para restaurar, actualizar o reparar un ATS Mini compatible. Incluye el bootloader, la tabla de particiones, la imagen de arranque de la aplicación y el firmware principal, además de las direcciones necesarias para escribir cada archivo en la memoria del dispositivo.

> Advertencia: verifica el modelo y la revisión exactos del hardware antes de flashear. Un firmware incompatible puede dejar el dispositivo inutilizable. Se recomienda hacer una copia de seguridad antes de realizar cambios.

### Contenido del repositorio

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

### Archivos de firmware

| Archivo | Descripción | Dirección de flasheo |
| --- | --- | ---: |
| `ATSMINI/bootloader.bin` | Bootloader | `0x00000` |
| `ATSMINI/partitions.bin` | Tabla de particiones | `0x08000` |
| `ATSMINI/boot_app0.bin` | Imagen de arranque | `0x0E000` |
| `ATSMINI/firmware.bin` | Firmware principal | `0x10000` |

### Requisitos

- Un ATS Mini compatible
- Cable USB con datos y un puerto serie disponible
- Python 3.x
- [esptool](https://github.com/espressif/esptool) instalado

Instala `esptool`:

```bash
python -m pip install esptool
```

### Flasheo

1. Clona o descarga este repositorio.
2. Conecta el dispositivo e identifica el puerto serie (`COMx` en Windows o `/dev/tty...` en Linux/macOS).
3. Desde la raíz del proyecto, ejecuta:

```bash
python -m esptool --port <PORT> write-flash \
  0x00000 ATSMINI/bootloader.bin \
  0x08000 ATSMINI/partitions.bin \
  0x0E000 ATSMINI/boot_app0.bin \
  0x10000 ATSMINI/firmware.bin
```

Reemplaza `<PORT>` por el puerto real del dispositivo. La sintaxis puede variar según la versión de `esptool`; consulta `python -m esptool --help` si es necesario.

No borres toda la memoria flash salvo que la documentación específica del dispositivo lo indique. Consulta [`docs/FLASHING.md`](docs/FLASHING.md) para ver el procedimiento ampliado. Las direcciones también están en [`ATSMINI/FLASH-OFFSETS.txt`](ATSMINI/FLASH-OFFSETS.txt).

### Compilación

GitHub Actions compila el firmware en cada envío de cambios y en las pull requests. Las versiones publicadas requieren etiquetas `X.Y.Z` que coincidan con `FIRMWARE_VERSION` en `main.cpp`.

### Seguridad y compatibilidad

- Confirma el modelo y la revisión exactos antes de flashear.
- Usa solo imágenes verificadas y compatibles con tu dispositivo.
- Si la radio contiene configuraciones o datos importantes, respáldalos antes de actualizar.
- Este repositorio no sustituye la documentación oficial del fabricante ni la garantía del hardware.
- No publiques números de serie, credenciales ni datos personales al solicitar soporte.

### Solución de problemas

Si falla la escritura:

1. Verifica el puerto serie y el cable USB.
2. Prueba otro cable de datos o puerto.
3. Comprueba la versión de `esptool` y la sintaxis del comando.
4. Confirma que la imagen corresponde al modelo y la revisión del dispositivo.
5. Asegúrate de que el ATS Mini esté en modo de programación.

### Soporte

Al informar un problema, incluye:

- Modelo y revisión del ATS Mini
- Sistema operativo
- Versión de `esptool`
- Puerto serie utilizado
- Mensaje de error exacto

### Licencia

Este repositorio no declara una licencia explícita. No debe suponerse que el firmware, el código fuente o los binarios pueden reutilizarse, redistribuirse o modificarse libremente sin confirmar primero los derechos del titular.

### Notas

Este paquete está destinado a la recuperación técnica, el mantenimiento y la actualización del firmware. Para desarrollar o modificar el proyecto, revisa la estructura del código fuente y la documentación relacionada antes de cambiar su funcionamiento.
