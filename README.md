<div align="center">
  <img src="assets/ats-mini-banner.svg" alt="KALIMAN firmware for ATS Mini" width="100%">

# KALIMAN Firmware for ATS Mini

**Custom firmware for the ATS Mini · ESP32-S3 · SI4732**

[![Download firmware](https://img.shields.io/badge/Download-latest%20firmware-238636?style=for-the-badge&logo=github)](https://github.com/satan666000/ATS-MINI-FIRMWARE/releases/latest)
[![Latest release](https://img.shields.io/github/v/release/satan666000/ATS-MINI-FIRMWARE?style=for-the-badge)](https://github.com/satan666000/ATS-MINI-FIRMWARE/releases/latest)
[![Firmware CI](https://github.com/satan666000/ATS-MINI-FIRMWARE/actions/workflows/firmware.yml/badge.svg?branch=main)](https://github.com/satan666000/ATS-MINI-FIRMWARE/actions/workflows/firmware.yml)
![Platform](https://img.shields.io/badge/Platform-ESP32--S3-blue?style=for-the-badge)
![Receiver](https://img.shields.io/badge/Receiver-SI4732-green?style=for-the-badge)

[English](#english) · [Español](#español) · [Flashing guide / Guía de flasheo](docs/FLASHING.md)

</div>

<div align="center">
  <strong>Download the firmware</strong> · <a href="https://github.com/satan666000/ATS-MINI-FIRMWARE/releases/latest">Get the latest release</a> · <a href="docs/FLASHING.md">Read the installation guide</a>
</div>

> [!WARNING]
> **Verify hardware compatibility before flashing.** Confirm the exact ATS Mini model and hardware revision, and back up the original firmware. Using an incompatible image or incorrect flash offsets may make the device unusable.
>
> **Verifica la compatibilidad del hardware antes de flashear.** Confirma el modelo y la revisión exactos del ATS Mini y guarda una copia del firmware original. Una imagen incompatible o direcciones de flasheo incorrectas pueden dejar el dispositivo inutilizable.

<details>
<summary><strong>Contents / Contenido</strong></summary>

**English:** [Overview](#english) · [Features](#features) · [Downloads](#downloads) · [Installation](#quick-flashing) · [Build](#build-from-source) · [Support](#support) · [License](#license)

**Español:** [Descripción](#español) · [Características](#características) · [Descargas](#descargas) · [Instalación](#flasheo-rápido) · [Compilación](#compilar-desde-el-código-fuente) · [Soporte](#soporte) · [Licencia](#licencia)
</details>

---

## English

KALIMAN is custom firmware for the ATS Mini radio, built around the ESP32-S3 and SI4732 receiver. Developed by **KO6EAI**, it adds expanded reception options and a configurable, multilingual interface.

**Firmware source version:** `1.1.100` · **Latest published release:** see [GitHub Releases](https://github.com/satan666000/ATS-MINI-FIRMWARE/releases/latest).

### Features

<table>
  <tr>
    <td width="50%" valign="top"><strong>FM broadcast</strong><br>FM reception with RDS/RBDS support.</td>
    <td width="50%" valign="top"><strong>AM, shortwave &amp; SSB</strong><br>AM and shortwave reception, plus SSB in LSB and USB modes.</td>
  </tr>
  <tr>
    <td width="50%" valign="top"><strong>US CB band</strong><br>40 channels from 26.965 to 27.405 MHz.</td>
    <td width="50%" valign="top"><strong>Flexible tuning</strong><br>Automatic modulation selection by band or manual selection.</td>
  </tr>
  <tr>
    <td width="50%" valign="top"><strong>Squelch control</strong><br>Disabled, manual, or automatic RSSI-based operation.</td>
    <td width="50%" valign="top"><strong>Audio modes</strong><br>Automatic FM stereo or forced mono.</td>
  </tr>
  <tr>
    <td width="50%" valign="top"><strong>Reception status</strong><br>Signal strength, SNR, battery, and mono/stereo indicators.</td>
    <td width="50%" valign="top"><strong>Custom interface</strong><br>Multiple languages, selectable themes, a reception-data HUD, and an animated startup screen.</td>
  </tr>
</table>

### Compatibility and limitations

- Intended for compatible ATS Mini devices using an ESP32-S3 and SI4732.
- Wi-Fi/BLE remote control is not enabled in the current firmware build.
- Some functions remain under validation with real-world reception.
- `firmware.bin` contains the application only; it is not a full-flash backup.

### Downloads

Download published firmware packages from [GitHub Releases](https://github.com/satan666000/ATS-MINI-FIRMWARE/releases/latest). The repository's recovery image files and their flash offsets are:

| File | Description | Flash offset |
| :--- | :--- | ---: |
| `ATSMINI/bootloader.bin` | ESP32-S3 bootloader | `0x00000` |
| `ATSMINI/partitions.bin` | Partition table | `0x08000` |
| `ATSMINI/boot_app0.bin` | Application boot image | `0x0E000` |
| `ATSMINI/firmware.bin` | KALIMAN application | `0x10000` |

### Quick flashing

**Requirements:** Python 3.x, `esptool`, a USB data cable, and the radio in programming mode.

Install `esptool`:

```bash
python -m pip install esptool
```

From the repository root, replace `<PORT>` with the radio's serial port (`COMx` on Windows or `/dev/tty...` on Linux/macOS):

```bash
python -m esptool --port <PORT> write-flash \
  0x00000 ATSMINI/bootloader.bin \
  0x08000 ATSMINI/partitions.bin \
  0x0E000 ATSMINI/boot_app0.bin \
  0x10000 ATSMINI/firmware.bin
```

Read the [complete flashing guide](docs/FLASHING.md) and verify the [flash-offset list](ATSMINI/FLASH-OFFSETS.txt) before proceeding. Do not erase the full flash unless the device-specific instructions explicitly require it.

### Build from source

Install the PlatformIO version used by CI and build the ATS Mini environment:

```bash
python -m pip install platformio==6.1.18
pio run --project-conf ci/platformio-release.ini -e ats-mini
```

GitHub Actions builds the firmware for pushes and pull requests. Release tags must use the `X.Y.Z` format and match `FIRMWARE_VERSION` in `main.cpp`.

### Project structure

| Path | Description |
| :--- | :--- |
| `main.cpp` | Firmware application and radio behavior |
| `Display.h` | ATS Mini display driver |
| `Languages.h` | User-interface translations |
| `SSBPatch.h` | SSB patch implementation; subject to separate terms |
| `include/Remote.h` | Remote-control implementation, not enabled in the current build |
| `ci/platformio-release.ini` | PlatformIO release-build configuration |
| `.github/workflows/firmware.yml` | Firmware build and release workflow |

### Support

When reporting an issue, include the board model and hardware revision, operating system, `esptool` version, serial port, and exact error message. Do not include device credentials or other sensitive information.

---

## Español

KALIMAN es un firmware personalizado para el ATS Mini, basado en el ESP32-S3 y el receptor SI4732. Desarrollado por **KO6EAI**, amplía las opciones de recepción e incorpora una interfaz configurable y multilingüe.

**Versión del código fuente:** `1.1.50` · **Última versión publicada:** consulta [GitHub Releases](https://github.com/satan666000/ATS-MINI-FIRMWARE/releases/latest).

### Características

<table>
  <tr>
    <td width="50%" valign="top"><strong>FM comercial</strong><br>Recepción FM con soporte RDS/RBDS.</td>
    <td width="50%" valign="top"><strong>AM, onda corta y SSB</strong><br>Recepción AM y de onda corta, además de SSB en modos LSB y USB.</td>
  </tr>
  <tr>
    <td width="50%" valign="top"><strong>Banda CB estadounidense</strong><br>40 canales de 26,965 a 27,405 MHz.</td>
    <td width="50%" valign="top"><strong>Sintonización flexible</strong><br>Selección automática de modulación por banda o selección manual.</td>
  </tr>
  <tr>
    <td width="50%" valign="top"><strong>Control de squelch</strong><br>Desactivado, manual o automático basado en RSSI.</td>
    <td width="50%" valign="top"><strong>Modos de audio</strong><br>FM estéreo automática o mono forzada.</td>
  </tr>
  <tr>
    <td width="50%" valign="top"><strong>Estado de recepción</strong><br>Indicadores de intensidad de señal, SNR, batería y modo mono/estéreo.</td>
    <td width="50%" valign="top"><strong>Interfaz personalizable</strong><br>Varios idiomas, temas seleccionables, HUD con datos de recepción y pantalla de inicio animada.</td>
  </tr>
</table>

### Compatibilidad y limitaciones

- Diseñado para dispositivos ATS Mini compatibles con ESP32-S3 y SI4732.
- El control remoto por Wi-Fi/BLE no está habilitado en la compilación actual.
- Algunas funciones siguen en validación con recepción real.
- `firmware.bin` contiene únicamente la aplicación; no es una copia completa de la memoria flash.

### Descargas

Descarga los paquetes publicados desde [GitHub Releases](https://github.com/satan666000/ATS-MINI-FIRMWARE/releases/latest). Los archivos de recuperación del repositorio y sus direcciones de flasheo son:

| Archivo | Descripción | Dirección |
| :--- | :--- | ---: |
| `ATSMINI/bootloader.bin` | Bootloader del ESP32-S3 | `0x00000` |
| `ATSMINI/partitions.bin` | Tabla de particiones | `0x08000` |
| `ATSMINI/boot_app0.bin` | Imagen de arranque de la aplicación | `0x0E000` |
| `ATSMINI/firmware.bin` | Aplicación KALIMAN | `0x10000` |

### Flasheo rápido

**Requisitos:** Python 3.x, `esptool`, un cable USB con datos y la radio en modo de programación.

Instala `esptool`:

```bash
python -m pip install esptool
```

Desde la raíz del repositorio, reemplaza `<PORT>` por el puerto serie de la radio (`COMx` en Windows o `/dev/tty...` en Linux/macOS):

```bash
python -m esptool --port <PORT> write-flash \
  0x00000 ATSMINI/bootloader.bin \
  0x08000 ATSMINI/partitions.bin \
  0x0E000 ATSMINI/boot_app0.bin \
  0x10000 ATSMINI/firmware.bin
```

Consulta la [guía completa de flasheo](docs/FLASHING.md) y verifica la [lista de direcciones](ATSMINI/FLASH-OFFSETS.txt) antes de continuar. No borres toda la memoria flash salvo que las instrucciones específicas del dispositivo lo exijan.

### Compilar desde el código fuente

Instala la misma versión de PlatformIO que utiliza CI y compila el entorno ATS Mini:

```bash
python -m pip install platformio==6.1.18
pio run --project-conf ci/platformio-release.ini -e ats-mini
```

GitHub Actions compila el firmware en cada envío de cambios y pull request. Las etiquetas de versión deben tener el formato `X.Y.Z` y coincidir con `FIRMWARE_VERSION` en `main.cpp`.

### Estructura del proyecto

| Ruta | Descripción |
| :--- | :--- |
| `main.cpp` | Aplicación del firmware y comportamiento de la radio |
| `Display.h` | Controlador de pantalla del ATS Mini |
| `Languages.h` | Traducciones de la interfaz |
| `SSBPatch.h` | Implementación del parche SSB; sujeto a términos independientes |
| `include/Remote.h` | Implementación del control remoto, no habilitada en la compilación actual |
| `ci/platformio-release.ini` | Configuración de compilación de lanzamiento con PlatformIO |
| `.github/workflows/firmware.yml` | Flujo de compilación y publicación del firmware |

### Soporte

Al informar un problema, incluye el modelo y la revisión de hardware, el sistema operativo, la versión de `esptool`, el puerto serie y el mensaje de error exacto. No incluyas credenciales del dispositivo ni otros datos confidenciales.

---

## License

The [MIT License](LICENSE) applies only to original project material owned by the copyright holder. It excludes `SSBPatch.h`, third-party dependencies, vendor-supplied files, and firmware/recovery binaries that contain or incorporate excluded material. These components remain subject to their own notices and terms. The SSB patch notice restricts commercial redistribution; the MIT license does not grant permission to redistribute that patch or affected binaries.

### Licencia

La [licencia MIT](LICENSE) se aplica únicamente al material original del proyecto cuyo titular es el propietario de los derechos. Excluye `SSBPatch.h`, las dependencias de terceros, los archivos suministrados por proveedores y los binarios de firmware/recuperación que incluyan material excluido. Estos componentes se rigen por sus propios avisos y condiciones. El aviso del parche SSB restringe su redistribución comercial; la licencia MIT no autoriza redistribuir ese parche ni los binarios afectados.
