<div align="center">
  <img src="assets/ats-mini-banner.svg" alt="ATS Mini KALIMAN firmware banner" width="100%">

# ATS Mini — KALIMAN Firmware

**Firmware personalizado para radios ATS Mini SI4732 con ESP32-S3**

[![Latest release](https://img.shields.io/github/v/release/satan666000/ATS-MINI-FIRMWARE?style=for-the-badge)](https://github.com/satan666000/ATS-MINI-FIRMWARE/releases/latest)
[![Firmware CI](https://github.com/satan666000/ATS-MINI-FIRMWARE/actions/workflows/firmware.yml/badge.svg?branch=main)](https://github.com/satan666000/ATS-MINI-FIRMWARE/actions/workflows/firmware.yml)
![Platform](https://img.shields.io/badge/Platform-ESP32--S3-blue?style=for-the-badge)
![Radio](https://img.shields.io/badge/Radio-SI4732-green?style=for-the-badge)

[English](#english) · [Español](#español) · [Flashing Guide](docs/FLASHING.md)

</div>

> [!WARNING]
> **Compatibilidad y seguridad / Compatibility and safety**
>
> Confirm the exact ATS Mini model and hardware revision before flashing. Always keep a backup of the original firmware. Firmware images can permanently alter the device if the wrong hardware or offset is used.
>
> Confirma el modelo exacto y la revisión de hardware del ATS Mini antes de flashear. Conserva siempre una copia del firmware original. Una imagen, hardware o dirección incorrectos pueden modificar el dispositivo de forma permanente.

---

## English

**KALIMAN** is a custom firmware for the ATS Mini radio, based on the ESP32-S3 platform and SI4732 receiver, developed by **KO6EAI**.

### KALIMAN overview

**Current version:** `1.1.50`

KALIMAN adds expanded radio reception and control features while preserving a familiar, configurable interface for compatible ATS Mini devices.

### Features

- FM reception with RDS/RBDS, AM, shortwave, and SSB (LSB/USB).
- US CB support: 40 channels from 26.965 to 27.405 MHz.
- Automatic modulation selection by band or manual selection.
- Disabled, manual, or RSSI-based automatic squelch.
- Automatic FM stereo or forced mono reception.
- Signal level, SNR, battery, and mono/stereo indicators.
- Multilingual interface, selectable themes, and a reception-data-based HUD.
- Animated startup screen and configuration menus.

### What to know

- Wi-Fi and BLE remote control are not supported.
- Some functions are still being validated with real reception.
- Use only firmware images built for compatible hardware.
- Keep a backup of important settings and the original firmware.
- `firmware.bin` contains the application; it is not a complete flash backup.

### Downloads

The recommended images and release packages are published on the [GitHub Releases](https://github.com/satan666000/ATS-MINI-FIRMWARE/releases/latest) page.

| File | Purpose | Flash offset |
| :--- | :--- | ---: |
| `ATSMINI/bootloader.bin` | ESP32-S3 bootloader | `0x00000` |
| `ATSMINI/partitions.bin` | Flash partition table | `0x08000` |
| `ATSMINI/boot_app0.bin` | Application boot image | `0x0E000` |
| `ATSMINI/firmware.bin` | KALIMAN application firmware | `0x10000` |

### Quick flashing

**Requirements**

- Python 3.x
- `esptool`
- USB data cable
- Radio in programming mode

Install `esptool`:

```bash
python -m pip install esptool
```

Replace `<PORT>` with the serial port for the radio:

- Windows: `COM3`, `COM4`, or another available `COM` port
- Linux/macOS: `/dev/ttyUSB0`, `/dev/cu.usbserial-*`, or a similar device

```bash
python -m esptool --port <PORT> write-flash \
  0x00000 ATSMINI/bootloader.bin \
  0x08000 ATSMINI/partitions.bin \
  0x0E000 ATSMINI/boot_app0.bin \
  0x10000 ATSMINI/firmware.bin
```

Follow the [complete flashing guide](docs/FLASHING.md) and verify the [flash offsets](ATSMINI/FLASH-OFFSETS.txt) before proceeding. Do not erase the full flash unless the device-specific instructions require it.

### Build from source

```bash
python -m pip install platformio==6.1.18
pio run --project-conf ci/platformio-release.ini -e ats-mini
```

GitHub Actions builds the firmware for pushes and pull requests. A versioned release requires an `X.Y.Z` tag matching `FIRMWARE_VERSION` in `main.cpp`.

### Project structure

| Path | Description |
| :--- | :--- |
| `main.cpp` | Main firmware application and radio behavior |
| `Display.h` | ATS Mini display driver |
| `Languages.h` | Multilingual user-interface translations |
| `SSBPatch.h` | SSB patch implementation |
| `include/Remote.h` | Wi-Fi and BLE remote-control support |
| `ci/platformio-release.ini` | PlatformIO release configuration |
| `.github/workflows/firmware.yml` | Build and release workflow |

### Support

Include the following information when reporting a problem:

- Board model and hardware revision
- Operating system
- `esptool` version
- Serial port
- Exact error message

---

## Español

**KALIMAN** es un firmware personalizado para el ATS Mini, basado en la plataforma ESP32-S3 y el receptor SI4732, desarrollado por **KO6EAI**.

### Presentación de KALIMAN

**Versión actual:** `1.1.50`

KALIMAN amplía las funciones de recepción y control de radio mientras conserva una interfaz configurable y familiar para dispositivos ATS Mini compatibles.

### Características

- Recepción FM con RDS/RBDS, AM, onda corta y SSB (LSB/USB).
- CB estadounidense: 40 canales de 26,965 a 27,405 MHz.
- Selección de modulación automática por banda o selección manual.
- Squelch desactivado, manual o automático basado en RSSI.
- FM estéreo automático o recepción mono forzada.
- Indicadores de señal, SNR, batería y mono/estéreo.
- Interfaz multilingüe, temas configurables y HUD basado en datos de recepción.
- Pantalla de inicio animada y menús de configuración.

### Consideraciones importantes

- No incluye control remoto por Wi-Fi ni BLE.
- Algunas funciones siguen en validación con recepción real.
- Utiliza únicamente imágenes diseñadas para hardware compatible.
- Conserva una copia de seguridad de los ajustes y del firmware original.
- `firmware.bin` contiene la aplicación; no es una copia completa de la memoria flash.

### Descargas

Las imágenes recomendadas y los paquetes de cada versión se publican en [GitHub Releases](https://github.com/satan666000/ATS-MINI-FIRMWARE/releases/latest).

| Archivo | Función | Dirección de flasheo |
| :--- | :--- | ---: |
| `ATSMINI/bootloader.bin` | Bootloader del ESP32-S3 | `0x00000` |
| `ATSMINI/partitions.bin` | Tabla de particiones de la flash | `0x08000` |
| `ATSMINI/boot_app0.bin` | Imagen de arranque de la aplicación | `0x0E000` |
| `ATSMINI/firmware.bin` | Firmware de la aplicación KALIMAN | `0x10000` |

### Flasheo rápido

**Requisitos**

- Python 3.x
- `esptool`
- Cable USB con datos
- Radio en modo de programación

Instala `esptool`:

```bash
python -m pip install esptool
```

Reemplaza `<PORT>` por el puerto serie de la radio:

- Windows: `COM3`, `COM4` u otro puerto `COM` disponible
- Linux/macOS: `/dev/ttyUSB0`, `/dev/cu.usbserial-*` o un dispositivo similar

```bash
python -m esptool --port <PORT> write-flash \
  0x00000 ATSMINI/bootloader.bin \
  0x08000 ATSMINI/partitions.bin \
  0x0E000 ATSMINI/boot_app0.bin \
  0x10000 ATSMINI/firmware.bin
```

Consulta la [guía completa de flasheo](docs/FLASHING.md) y verifica la [lista de direcciones](ATSMINI/FLASH-OFFSETS.txt) antes de continuar. No borres toda la memoria flash salvo que las instrucciones específicas del dispositivo lo exijan.

### Compilar desde el código fuente

```bash
python -m pip install platformio==6.1.18
pio run --project-conf ci/platformio-release.ini -e ats-mini
```

GitHub Actions compila el firmware en cada envío de cambios y pull request. Para publicar una versión se requiere una etiqueta `X.Y.Z` que coincida con `FIRMWARE_VERSION` en `main.cpp`.

### Estructura del proyecto

| Ruta | Descripción |
| :--- | :--- |
| `main.cpp` | Aplicación principal y comportamiento de la radio |
| `Display.h` | Controlador de pantalla del ATS Mini |
| `Languages.h` | Traducciones de la interfaz multilingüe |
| `SSBPatch.h` | Implementación del parche SSB |
| `include/Remote.h` | Compatibilidad con control remoto Wi-Fi y BLE |
| `ci/platformio-release.ini` | Configuración de compilación con PlatformIO |
| `.github/workflows/firmware.yml` | Flujo de compilación y publicación |

### Soporte

Al informar un problema, incluye:

- Modelo y revisión de hardware de la placa
- Sistema operativo
- Versión de `esptool`
- Puerto serie
- Mensaje de error exacto

---

## License / Licencia

This repository does not declare an explicit license. Do not assume that source code, firmware, or binaries may be reused, redistributed, or modified without permission from the rights holder.

Este repositorio no declara una licencia explícita. No se debe asumir que el código fuente, el firmware o los binarios pueden reutilizarse, redistribuirse o modificarse sin autorización del titular de los derechos.
